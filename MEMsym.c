#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TAM_RAM 4096   // bytes de RAM - 2^12 = 4 KB
#define TAM_CACHE 128  // bytes de caché - 2^7 = 128 B

#define TAM_PALABRA 4  // bits de palabra
#define TAM_MARCO 3    // bits de marco de bloque
#define TAM_ETQ 5      // bits de etiqueta

#define TAM_LINEA 16                                 // bytes de linea/bloque
#define TAM_BUS (TAM_PALABRA + TAM_MARCO + TAM_ETQ)  // bits del bus

#define NUM_LINEAS_CACHE (TAM_CACHE / TAM_LINEA)  // número de líneas que tiene la caché

// Estructura Memoria Cache
typedef struct Cache {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

void LimpiarCACHE(T_CACHE_LINE tbl[NUM_LINEAS_CACHE]);                                      // Asigna valores iniciales a la Caché
void VolcarCACHE(T_CACHE_LINE *tbl);                                                        // Muestra los conenidos de la Caché en un archivo
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);  // Lee dirreciones y las traduce
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);            // Si hay un miss de Caché, busca en la RAM

int *convertToBinary(char *line);
int binaryToDecimal(int *binario, int size);
void imprimeCache(T_CACHE_LINE *tbl);
void imprimeRAM(char *MRAM);

int globaltime = 0;
int numfallos = 0;

int main(int argc, char **argv) {
    char ficheroRam[] = "CONTENTS_RAM.bin";
    char ficheroDirecciones[] = "accesos_memoria.txt";

    FILE *file;

    T_CACHE_LINE tbl[NUM_LINEAS_CACHE];  // Memoria Caché
    unsigned char Simul_RAM[TAM_RAM];    // Memoria RAM

    /*  *** EMPIEZA EL PROCESO *** */
    LimpiarCACHE(tbl);  // Da un valor incial a cada campo de la caché

    /* *** PRIMERO LEE FICHERO "CONTENTS_RAM.bin" *** */
    file = fopen(ficheroRam, "rb");
    if (!file) {
        printf("ERROR - FALLO AL LEER FICHERO \"%s\"\n", ficheroRam);
        return -1;
    }
    fread(Simul_RAM, 1, sizeof(Simul_RAM), file);
    fclose(file);

    /* *** LEE FICHERO "accesos_memoria.txt" *** */
    char *line = NULL;
    size_t len = 0;

    file = fopen(ficheroDirecciones, "r");
    if (file == NULL) {
        printf("ERROR - FALLO AL LEER FICHERO \"%s\"\n", ficheroDirecciones);
        return -1;
    }

    // imprimeCache(tbl);
    // imprimeRAM(Simul_RAM);

    int numAccesos = 1, index = 0;
    char texto[4096];  // Acumula el texto leído por la CPU

    /* *** MAIN LOOP - LEE EL FICHERO LINEA POR LINEA *** */
    while (getline(&line, &len, file) != EOF) {
        unsigned int addr = 0;
        int etq = 0, linea = 0, palabra = 0, bloque = 0;

        sscanf(line, "%x", &addr);  // Convierte char* a int
        ParsearDireccion(addr, &etq, &palabra, &linea, &bloque);

        // printf("Dirección : %X\n\n", addr);

        // printf("ETQ : 0x%X - %d\n", etq, etq);
        // printf("Linea : 0x%X - %d\n", linea, linea);
        // printf("Palabra : 0x%X - %d\n", palabra, palabra);
        // printf("Bloque : 0x%X - %d\n", bloque, bloque);

        int indexLinea = bloque % NUM_LINEAS_CACHE;  // Obtiene la linea a buscar de la caché

        // printf("\nBuscar en línea : %d\n", indexLinea);
        // printf("Linea %d caché : 0x%X ETQ\n\n", indexLinea, tbl[indexLinea].ETQ);

        // Comprueba si la estiqueta de dirección es igual a la de la cache
        if (tbl[indexLinea].ETQ == etq) {
            printf("T: %d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X\n", numAccesos, addr, etq, linea, palabra, tbl[linea].Data[palabra]);
            texto[index++] = tbl[linea].Data[palabra];  // Añadimos el carácter leído al array texto
            globaltime++;
        } else {
            printf("T: %d, Fallo de CACHE %d, ADDR %04X Label %X linea %02X palabra %02X bloque %02X\n", numAccesos, numfallos, addr, etq, linea, palabra, bloque);
            TratarFallo(tbl, Simul_RAM, etq, linea, bloque);  // Copia el bloque necesario de RAM a Caché
            texto[index++] = tbl[linea].Data[palabra];        // Añadimos el carácter leído al array texto
        }

        // imprimeCache(tbl);

        printf("\n");
        numAccesos++;
        sleep(1);
    }

    /* *** FIN BUCLE - NO HAY MÁS DIRECCIONES QUE LEER *** */
    VolcarCACHE(tbl);

    printf("Accesos totales : %d; fallos : %d; Tiempo medio : %.2f\n", (numAccesos - 1), numfallos, (double)globaltime / (numAccesos - 1));
    printf("Texto leído : %s\n", texto);

    fclose(file);
    free(line);

    return 0;
}

void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque) {
    char hex[TAM_BUS / 4];     // Cada numero hexadeciaml es un cuarteto binario
    sprintf(hex, "%X", addr);  // Convierte int a char*

    int *binario = convertToBinary(hex);

    int etqArray[TAM_ETQ];          // La etiqueta son los primeros TAM_ETQ bits
    int lineaArray[TAM_MARCO];      // La linea son los siguientes TAM_MACRO bits
    int palabraArray[TAM_PALABRA];  // La palabra son los últimos TAM_PALABRA bits

    int bloqueArray[TAM_ETQ + TAM_MARCO];  // Dirección de la RAM donde se encunetra el bloque

    // Separa los bits del array binario en los arrays etiqueta, linea y palabra
    for (int i = 0; i < TAM_BUS; i++) {
        if (i < TAM_ETQ) {
            bloqueArray[i] = binario[i];
            etqArray[i] = binario[i];
        }
        if (i >= TAM_ETQ && i < TAM_ETQ + TAM_MARCO) {
            bloqueArray[i] = binario[i];
            lineaArray[i - TAM_ETQ] = binario[i];
        }
        if (i >= (TAM_ETQ + TAM_MARCO) && i < TAM_BUS)
            palabraArray[i - (TAM_ETQ + TAM_MARCO)] = binario[i];
    }

    // Calcula el valor decimal de los arrays de bits
    *ETQ = binaryToDecimal(etqArray, TAM_ETQ);
    *linea = binaryToDecimal(lineaArray, TAM_MARCO);
    *palabra = binaryToDecimal(palabraArray, TAM_PALABRA);

    *bloque = binaryToDecimal(bloqueArray, TAM_ETQ + TAM_MARCO);

    free(binario);
}

void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque) {
    numfallos++;
    globaltime += 10;

    // printf("Bloque %X de RAM a copiar : ", bloque);
    // for (int i = 0; i < TAM_LINEA; i++) {
    //     printf("%X ", MRAM[i + bloque * TAM_LINEA]);
    // }
    // printf("\n");
    printf("Cargando el bloque %X en la linea %X\n", bloque, linea);

    for (int i = 0; i < TAM_LINEA; i++)
        tbl[linea].Data[i] = MRAM[i + bloque * TAM_LINEA];
    tbl[linea].ETQ = ETQ;
}

void LimpiarCACHE(T_CACHE_LINE tbl[NUM_LINEAS_CACHE]) {
    for (int i = 0; i < NUM_LINEAS_CACHE; i++) {
        tbl[i].ETQ = 0xFF;
        for (int j = 0; j < TAM_LINEA; j++)
            tbl[i].Data[j] = 0x23;
    }
}

// Al final del programa vuelca los 128 bytes en "CONTENTS_CACHE.bin"
void VolcarCACHE(T_CACHE_LINE *tbl) {
    FILE *file = fopen("CONTENTS_CACHE.bin", "wb");
    if (!file) {
        printf("ERROR - FALLO AL CREAR FICHERO \"CONTENTS_CACHE.bin\"\n");
        exit(0);
    }
    for (int i = 0; i < NUM_LINEAS_CACHE; i++) {
        fprintf(file, "Etiqueta : %02X  Datos : ", tbl[i].ETQ);
        for (int j = TAM_LINEA - 1; j >= 0; j--)  // Las palabras en la linea se imprimen de izquierda a derecha
            fprintf(file, "%X ", tbl[i].Data[j]);
        fprintf(file, "\n");
    }
    fclose(file);
}

/* *** MIS FUNCIONES *** */
// Imprime los contenidos de la cache
void imprimeCache(T_CACHE_LINE *tbl) {
    printf("\n");
    for (int i = 0; i < NUM_LINEAS_CACHE; i++) {
        printf("l : %d  %02X  ", i, tbl[i].ETQ);
        for (int j = TAM_LINEA - 1; j >= 0; j--)  // Las palabras en la linea se imprimen de izquierda a derecha
            printf("%X ", tbl[i].Data[j]);
        printf("\n");
    }
}

// Imprime los contenidos de la RAM
void imprimeRAM(char *MRAM) {
    for (int i = 0; i < TAM_RAM / TAM_LINEA; i++) {
        if (i % 8 == 0) printf("\n");
        printf("l : %02X    ", i);
        for (int j = 0; j < TAM_LINEA; j++)
            printf("%02X ", MRAM[j + i * TAM_LINEA]);
        printf("\n");
    }
}

// Convierte de binario a decimal
int binaryToDecimal(int *binario, int size) {
    int exp = 0, decimal = 0;

    for (int i = 0; i < size; i++) {
        exp = (size - 1) - i;
        int valor = 1;
        if (binario[i] == 1) {
            while (exp != 0) {
                valor *= 2;
                exp--;
            }
            decimal += valor;
        }
    }
    return decimal;
}

// Convierte hexadecimal en un array de binarios y la devuelve
int *convertToBinary(char *line) {
    int *binario = (int *)malloc(sizeof(int) * 12);

    for (int i = 0; i < 3; i++) {
        switch (line[i]) {
            case '0':
                binario[0 + 4 * i] = 0;
                binario[1 + 4 * i] = 0;
                binario[2 + 4 * i] = 0;
                binario[3 + 4 * i] = 0;
                break;

            case '1':
                binario[0 + 4 * i] = 0;
                binario[1 + 4 * i] = 0;
                binario[2 + 4 * i] = 0;
                binario[3 + 4 * i] = 1;
                break;

            case '2':
                binario[0 + 4 * i] = 0;
                binario[1 + 4 * i] = 0;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 0;
                break;

            case '3':
                binario[0 + 4 * i] = 0;
                binario[1 + 4 * i] = 0;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 1;
                break;

            case '4':
                binario[0 + 4 * i] = 0;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 0;
                binario[3 + 4 * i] = 0;
                break;

            case '5':
                binario[0 + 4 * i] = 0;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 0;
                binario[3 + 4 * i] = 1;
                break;

            case '6':
                binario[0 + 4 * i] = 0;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 0;
                break;

            case '7':
                binario[0 + 4 * i] = 0;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 1;
                break;

            case '8':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 0;
                binario[2 + 4 * i] = 0;
                binario[3 + 4 * i] = 0;
                break;

            case '9':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 0;
                binario[2 + 4 * i] = 0;
                binario[3 + 4 * i] = 1;
                break;

            case 'A':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 0;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 0;
                break;

            case 'B':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 0;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 1;
                break;

            case 'C':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 0;
                binario[3 + 4 * i] = 0;
                break;

            case 'D':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 0;
                binario[3 + 4 * i] = 1;
                break;

            case 'E':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 0;
                break;

            case 'F':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 1;
                break;

            case 'a':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 0;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 0;
                break;

            case 'b':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 0;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 1;
                break;

            case 'c':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 0;
                binario[3 + 4 * i] = 0;
                break;

            case 'd':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 0;
                binario[3 + 4 * i] = 1;
                break;

            case 'e':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 0;
                break;

            case 'f':
                binario[0 + 4 * i] = 1;
                binario[1 + 4 * i] = 1;
                binario[2 + 4 * i] = 1;
                binario[3 + 4 * i] = 1;
                break;

            default:
                break;
        }
    }
    return binario;
}

// Vasco de Melo - 2 INSO A
