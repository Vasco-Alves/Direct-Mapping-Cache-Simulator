#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TAM_PALABRA 4
#define TAM_LINEA 8
#define TAM_ETQ 5

// numfilas 4
// maxline 100
// tamlinea 8
// lram 1024

typedef struct Cache {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

int globaltime = 0;
int numfallos = 0;

void init(T_CACHE_LINE *tbl, int size);
// void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);

int *convertToBinary(char *line, int index);

int main(int argc, char **argv) {
    printf("\n");
    int numLineas = 0;
    FILE *file;

    T_CACHE_LINE tbl[8];
    unsigned char Simul_RAM[4096];

    init(tbl, sizeof(tbl) / sizeof(tbl[0]));  // Asigna valores iniciales a la caché
    printf("\n");

    /* Lee fichero binario */
    file = fopen("CONTENTS_RAM.bin", "rb");
    if (!file) {
        printf("Fallo al leer fichero\n");
        return -1;
    }
    fread(Simul_RAM, 1, sizeof(Simul_RAM), file);
    fclose(file);

    // for (int i = 0; i < sizeof(Simul_RAM); i++)
    //     printf("%u ", Simul_RAM[i]);  // prints a series of bytes

    char *line = NULL;
    size_t len = 0;

    int i = 0;

    /* Lee fichero accesos_memoria.txt */
    file = fopen("accesos_memoria.txt", "r");
    if (file == NULL)
        return -1;

    char **datos = NULL;

    int *binario = (int *)malloc(sizeof(int) * 12);
    int index = 0;

    // Main loop
    while (getline(&line, &len, file) != -1) {
        printf(" ------------------- \n");
        printf("Hex : %s", line);
        // printf("Hex : ");
        // for (int i = 0; i < 3; i++)
        //     printf("%c", line[i]);
        binario = convertToBinary(line, index++);  // Array con valor binario de linea
        printf("Bin : ");
        for (int i = 0; i < 12; i++) {
            if (i == 4 || i == 8)
                printf(" ");
            printf("%d", binario[i]);
        }

        printf("\n\n");

        // ETQ
        printf("ETQ : ");
        for (int i = 0; i < 5; i++)
            printf("%d", binario[i]);
        printf("\n");
        // Linea
        printf("Linea : ");
        for (int i = 5; i < 8; i++)
            printf("%d", binario[i]);
        printf("\n");
        // Palabra
        printf("Palabra : ");
        for (int i = 8; i < 12; i++)
            printf("%d", binario[i]);
        printf("\n");

        printf("\n");
        sleep(1);
    }

    fclose(file);
    free(line);
    free(datos);
    printf("\n");

    return 0;
}

// Arranque proceso
void init(T_CACHE_LINE *tbl, int size) {
    for (int i = 0; i < size; i++) {
        tbl[i].ETQ = 0xFF;
        for (int j = 0; j < TAM_LINEA; j++)
            tbl[i].Data[j] = 0x23;
    }

    for (int i = 0; i < size; i++) {
        printf("ETQ : %X Data : ", tbl[i].ETQ);
        for (int j = 0; j < TAM_LINEA; j++)
            printf("%X ", tbl[i].Data[j]);
        printf("\n");
    }
}

// Convierte hexadecimal en un array dee binarios y la devuelve
int *convertToBinary(char *line, int index) {
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

// Compartir con usuario github CarlosVU
