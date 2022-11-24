#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TAM_PALABRA 4  // w
#define TAM_MARCO 3    // r
#define TAM_ETQ 5      // ETQ

#define TAM_LINEA 16
#define TAM_BUS 12
#define NUM_LINEAS 8

#define TAM_RAM 4096   // 2^12
#define TAM_CACHE 128  // 2^7

typedef struct Cache {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

int globaltime = 0;
int numfallos = 0;

void LimpiarCACHE(T_CACHE_LINE tbl[NUM_LINEAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);

int *convertToBinary(char *line);
int binaryToDecimal(int *binario, int size);

int main(int argc, char **argv) {
    int timer = 1;
    FILE *file;

    T_CACHE_LINE tbl[NUM_LINEAS];
    unsigned char Simul_RAM[TAM_RAM];

    //  *** EMPIEZA EL PROCESO ***
    LimpiarCACHE(tbl);
    printf("\n");

    // *** PRIMERO LEE FICHERO "CONTENTS_RAM.bin" ***
    file = fopen("CONTENTS_RAM.bin", "rb");
    if (!file) {
        printf("Fallo al leer fichero\n");
        return -1;
    }
    fread(Simul_RAM, 1, sizeof(Simul_RAM), file);
    fclose(file);

    // *** LEE FICHERO "accesos_memoria.txt" ***
    char *line = NULL;
    size_t len = 0;

    file = fopen("accesos_memoria.txt", "r");
    if (file == NULL)
        return -1;

    int *binario = (int *)malloc(sizeof(int) * 12);

    // *** MAIN LOOP - Mientras hay direcciones que leer ***
    while (getline(&line, &len, file) != -1) {
        unsigned int addr = 0;
        int etq = 0, linea = 0, palabra = 0, bloque = 0;

        // binario = convertToBinary(line);  // Convierte la dirección en un array de numeros binarios
        sscanf(line, "%x", &addr);  // Convierte char* a int
        ParsearDireccion(addr, &etq, &palabra, &linea, &bloque);

        printf("Dirección : %X", addr);

        printf("ETQ : %X\n", etq);
        printf("Linea : %X\n", linea);
        printf("Palbra : %X\n", palabra);

        // int etq = binaryToDecimal(binario, TAM_ETQ);               // Convierte ETQ en decimal
        // int bloque = binaryToDecimal(bloqueArray, TAM_MARCO);      // Convierte bloque en decimal
        // int palabra = binaryToDecimal(palabraArray, TAM_PALABRA);  // Convierte palabra en decimal

        // int direccionDecimal = binaryToDecimal(binario, TAM_ETQ + TAM_MARCO + TAM_PALABRA);  // Convierte la dirección de hexadecimal a decimal
        // int indexLinea = bloque % NUM_LINEAS;                                                // Obtiene la línea en que buscar en la caché

        // // ETQ
        // printf("\nETQ : ");
        // for (int i = 0; i < TAM_ETQ; i++)
        //     printf("%d", binario[i]);
        // printf(" - 0x%X\n", etq);

        // // Linea
        // printf("Linea : ");
        // for (int i = TAM_ETQ; i < TAM_ETQ + TAM_MARCO; i++)
        //     printf("%d", binario[i]);
        // printf(" - 0x%X\n", bloque);

        // // Palabra
        // printf("Palabra : ");
        // for (int i = TAM_ETQ + TAM_MARCO; i < TAM_BUS; i++)
        //     printf("%d", binario[i]);
        // printf(" - 0x%X\n", palabra);

        // printf("\nBuscar en línea : %d\n", indexLinea);
        // printf("Linea %d caché : 0x%X ETQ\n\n", indexLinea, tbl[indexLinea].ETQ);

        // if (tbl[indexLinea].ETQ == etq)
        //     printf("“T: %d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X”\n", timer, etq, tbl[indexLinea].ETQ, bloque, palabra, 0);
        // else {
        //     printf("“T: %d, Fallo de CACHE %d, ADDR %04X Label %X linea %02X palabra %02X bloque %02X”,\n", timer, numfallos, direccionDecimal, etq, bloque, palabra, Simul_RAM[bloque]);
        //     TratarFallo(tbl, Simul_RAM, etq, bloque, bloque);
        // }

        printf("\n");
        timer++;
        sleep(1);
    }

    // *** TERMINA BUCLE ***
    printf("Accesos totales : %d; fallos : %d; Tiempo medio : %d\n", 0, numfallos, 0);
    printf("Texto leído : \n");

    fclose(file);
    free(line);
    free(binario);
    printf("\n");

    return 0;
}

void LimpiarCACHE(T_CACHE_LINE tbl[NUM_LINEAS]) {
    for (int i = 0; i < NUM_LINEAS; i++) {
        tbl[i].ETQ = 0xFF;
        for (int j = 0; j < TAM_LINEA; j++)
            tbl[i].Data[j] = 0x23;
    }
}

// Al fina del programa vuelca los 128 bytes en CONTENTS_CACHE.bin
void VolcarCACHE(T_CACHE_LINE *tbl) {
}

void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque) {
    char hex[TAM_BUS / 4];
    sprintf(hex, "%X", addr);

    int *binario = convertToBinary(hex);
    int etqArray[TAM_ETQ] = {binario[0], binario[1], binario[2], binario[3], binario[4]};
    int bloqueArray[TAM_MARCO] = {binario[5], binario[6], binario[7]};
    int palabraArray[TAM_PALABRA] = {binario[8], binario[9], binario[10], binario[11]};

    *ETQ = binaryToDecimal(binario, TAM_ETQ);
    *linea = binaryToDecimal(bloqueArray, TAM_MARCO);
    *palabra = binaryToDecimal(palabraArray, TAM_PALABRA);
}

void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque) {
    numfallos++;
    globaltime += 10;

    printf("RAM : %X\n", MRAM[256 * linea % NUM_LINEAS]);
}

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
