#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TAM_RAM 4096  // bytes de RAM - 2^12 = 4 KB
#define TAM_CACHE 128 // bytes de caché - 2^7 = 128 B

#define TAM_PALABRA 4 // bits de palabra
#define TAM_MARCO 3   // bits de marco de bloque
#define TAM_ETQ 5     // bits de etiqueta

#define TAM_LINEA 16                                // bytes de linea/bloque
#define TAM_BUS (TAM_PALABRA + TAM_MARCO + TAM_ETQ) // bits del bus

#define NUM_LINEAS_CACHE (TAM_CACHE / TAM_LINEA) // número de líneas que tiene la caché

// Estructura Memoria Cache
typedef struct Cache {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

void LimpiarCACHE(T_CACHE_LINE tbl[NUM_LINEAS_CACHE]);                                     // Asigna valores iniciales a la caché
void VolcarCACHE(T_CACHE_LINE *tbl);                                                       // Muestra los contenidos de la caché en un archivo
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque); // Lee dirreciones de memoria y las traduce
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);           // Si hay un fallo de caché, busca en la RAM

void imprimeCache(T_CACHE_LINE *tbl);
void imprimeRAM(char *MRAM);

int globaltime = 0;
int numfallos = 0;

int main(int argc, char **argv) {
    char ficheroRam[] = "CONTENTS_RAM.bin";
    char ficheroDirecciones[] = "accesos_memoria.txt";

    FILE *file;

    T_CACHE_LINE tbl[NUM_LINEAS_CACHE]; // Memoria Caché
    unsigned char Simul_RAM[TAM_RAM];   // Memoria RAM

    /*  *** EMPIEZA EL PROCESO *** */
    LimpiarCACHE(tbl); // Da un valor incial a cada campo de la caché

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
    if (!file) {
        printf("ERROR - FALLO AL LEER FICHERO \"%s\"\n", ficheroDirecciones);
        return -1;
    }

    imprimeCache(tbl);
    printf("\n");
    imprimeRAM(Simul_RAM);
    printf("\n");

    int numAccesos = 1, index = 0;
    char texto[4096]; // Acumula el texto leído por la CPU

    /* *** MAIN LOOP - LEE EL FICHERO LINEA POR LINEA *** */
    while (getline(&line, &len, file) != EOF) {
        unsigned int addr = 0;
        int etq = 0, linea = 0, palabra = 0, bloque = 0;

        sscanf(line, "%x", &addr); // Convierte char* a int
        printf("\n--------- Dirección a leer : %02X ---------\n\n", addr);

        ParsearDireccion(addr, &etq, &palabra, &linea, &bloque);

        int indexLinea = bloque % NUM_LINEAS_CACHE; // Obtiene la linea a buscar de la caché

        printf("Buscar en la línea %d de la caché\n", indexLinea);
        printf("Linea %d caché : ETQ %02X\n\n", indexLinea, tbl[indexLinea].ETQ);

        // Comprueba si la etiqueta de dirección es igual a la de la cache
        if (tbl[indexLinea].ETQ == etq) {
            printf("T: %d, Acierto de CACHE, ADDR %02X ETQ %02X linea %02X palabra %02X DATO %02X\n\n", numAccesos, addr, etq, linea, palabra, tbl[linea].Data[palabra]);
            globaltime++;

        } else {
            printf("T: %d, Fallo de CACHE %d, ADDR %02X ETQ %02X linea %02X palabra %02X bloque %02X\n", numAccesos, ++numfallos, addr, etq, linea, palabra, bloque);
            TratarFallo(tbl, Simul_RAM, etq, linea, bloque); // Copia el bloque necesario de RAM a Caché
            globaltime += 10;
        }

        texto[index++] = tbl[linea].Data[palabra]; // Añadimos el carácter leído al array texto
        imprimeCache(tbl);

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
    *palabra = addr & ((1 << TAM_PALABRA) - 1);
    addr >>= TAM_PALABRA;
    *linea = addr & ((1 << TAM_MARCO) - 1);
    addr >>= TAM_MARCO;
    *ETQ = addr & ((1 << TAM_ETQ) - 1);
    *bloque = (*ETQ << TAM_MARCO) | *linea; // block = label size + frame size -> increases size to fit frame and adds it
}

void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque) {
    printf("Bloque %02X a copiar : ", bloque);
    for (int i = 0; i < TAM_LINEA; i++) {
        printf("%X ", MRAM[i + bloque * TAM_LINEA]);
    }
    printf("\n");
    printf("Cargando el bloque %02X en la linea %d de la caché\n\n", bloque, linea);

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
        fprintf(file, "ETQ : %02X  -  Datos : ", tbl[i].ETQ);
        for (int j = TAM_LINEA - 1; j >= 0; j--) // Las palabras en la linea se imprimen de izquierda a derecha
            fprintf(file, "%X ", tbl[i].Data[j]);
        fprintf(file, "\n");
    }
    fclose(file);
}

void imprimeCache(T_CACHE_LINE *tbl) {
    printf("********************************** CACHE **********************************\n");
    for (int i = 0; i < NUM_LINEAS_CACHE; i++) {
        printf("%3d -> ETQ : %02X  -  Datos : ", i, tbl[i].ETQ);
        for (int j = TAM_LINEA - 1; j >= 0; j--) // Las palabras en la linea se imprimen de izquierda a derecha
            printf("%02X ", tbl[i].Data[j]);
        printf("\n");
    }
}

void imprimeRAM(char *MRAM) {
    printf("*********************************** RAM ***********************************");
    for (int i = 0; i < TAM_RAM / TAM_LINEA; i++) {
        if (i % 8 == 0)
            printf("\n");
        printf("%3d -> BLQ : %02X  -  Datos : ", i, i);
        for (int j = 0; j < TAM_LINEA; j++)
            printf("%02X ", MRAM[j + i * TAM_LINEA]);
        printf("\n");
    }
}
