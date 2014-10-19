#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*struct Label * comecoLabels = NULL;
struct Label * finalLabels = NULL;
struct Const * comecoConst = NULL;
struct Const * finalConst = NULL;
int tracker[2] = {0, 0};*/

void primeiraPassada(FILE * arquivoEntrada) { return; }
void segundaPassada(FILE * arquivoEntrada, FILE * arquivoSaida) { return; }

int main(int argc, char *argv[]) {
	FILE * arquivoEntrada;
	FILE * arquivoSaida;
	char * nomeTemp;
	
	/* Inicializacao de variaveis*/
	if (argc < 2 || argc > 3) {
		printf("ERRO: Insira apenas um ou dois argumentos, correspondentes a, respectivamente, o nome do arquivo de entrada e (opcionalmente) o nome do arquivo de saida.");
		return 0;
	}
	else if (argc == 2) {
		arquivoEntrada = fopen(argv[1], "r");
		nomeTemp = (char*) malloc(sizeof(argv[1])+(sizeof(char)*4)); /* Tamanho suficiente para nome do arquivo principal + .hex */
		strcpy(nomeTemp, argv[1]);
		strcat(nomeTemp, ".hex");
		arquivoSaida = fopen(nomeTemp, "w");
		printf("%c", nomeTemp[strlen(nomeTemp)-1]);
		free(nomeTemp);
	}
	else {
		arquivoEntrada = fopen(argv[1], "r");
		arquivoSaida = fopen(argv[2], "w");
	}
	
	/*Execucao principal */
	primeiraPassada(arquivoEntrada);
	segundaPassada(arquivoEntrada, arquivoSaida);
	
	fclose(arquivoEntrada);
	fclose(arquivoSaida);
	return 0;
}

