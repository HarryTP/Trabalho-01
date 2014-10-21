#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct Label {
	char nome[101];
	int endereco[2];
	struct Label * next;
};

struct Label *comecoLabels = NULL;
struct Label *finalLabels = NULL;
/*struct Const * comecoConst = NULL;
struct Const * finalConst = NULL;*/
int tracker[2] = {0, 0};

void analisaDiretiva(char *dir, FILE *arquivoSaida) { return; }; /* Se arquivoSaida = NULL, primeira passada, senao segunda */
void analisaInstrucao(char *op, FILE *arquivoSaida) { return; }; /* Se arquivoSaida = NULL, primeira passada, senao segunda */

void erroSintaxe() { return; };
void confereUsoLabels() { return; };
void conferePendentes() { return; };

void armazenaLabel(char *label) { 
	struct Label newLabel;
	char nomeTemp[101] = "";
	int i;
	
	/* Tamanho invalido */
	if (strlen(label) > 101)
		erroSintaxe();
	
	/* Retira : */
	strncpy (nomeTemp, label, strlen(label)-1);
	nomeTemp[strlen(label)] = '\0';
	
	/* Analise de sintaxe */
	for (i = 0; i < strlen(nomeTemp); i++) {
		if ( !isalpha(nomeTemp[i]) && !(nomeTemp[i] == '_') )
			erroSintaxe();
	}
	
	/* Seta a nova variavel */
	strcpy(newLabel.nome, nomeTemp);
	if (tracker[1] == 1) {
		newLabel.endereco[0] = tracker[0]+1;
		newLabel.endereco[1] = 0;
	}
	else {
		newLabel.endereco[0] = tracker[0];
		newLabel.endereco[1] = 1;
	}
	newLabel.next = NULL;
	
	/* Concatena na lista */
	if (comecoLabels == NULL) {
		comecoLabels = &newLabel;
		finalLabels = &newLabel;
	}
	else {
		finalLabels->next = &newLabel;
		finalLabels = &newLabel;
	}	
}

void primeiraPassada(FILE * arquivoEntrada) {
	char inputLine[1000];
	char *token;
	bool wholeLine = false;
	bool readLabel = false;
	bool readDirective = false;
	bool readOperation = false;
	
	while (!feof(arquivoEntrada)) {
		if (fgets(inputLine, 1000, arquivoEntrada) != NULL) {
			if (inputLine[strlen(inputLine)-1] == '\n') /*Confere se toda a linha foi lida*/
				wholeLine = true;
			else
				wholeLine = false;
				
			token = strtok(inputLine, " \n");
			while (token != NULL) {
				if (token[strlen(token)-1] == ':') { /*Label*/
					if (!readLabel && !readDirective && !readOperation) {
						armazenaLabel(token);
						readLabel = true;
						token = strtok(NULL, " \n");
					}
					else
						erroSintaxe();
				}
				
				else if (token[0] == '.') { /*Diretiva*/
					if (!readDirective && !readOperation) {
						analisaDiretiva(token, NULL);
						readDirective = true;
						token = strtok(NULL, " \n");
					}
					else
						erroSintaxe();
				}
				
				else if (token[0] == '#') { /*Comentario*/
					while (!wholeLine) {
						fgets(inputLine, 1000, arquivoEntrada);
						if (inputLine[strlen(inputLine)-1] == '\n')
							wholeLine = true;
						else
							wholeLine = false;
					}
					readLabel = false;
					readDirective = false;
					readOperation = false;
					token = NULL;
				}
				
				else { /*Instrucao*/
					if (!readOperation && !readDirective) {
						analisaInstrucao(token, NULL);
						readOperation = true;
						token = strtok(NULL, " \n");
					}
					else
						erroSintaxe();
				}
			}
		}
	}
	
	conferePendentes();/*Conferir se nenhuma label foi usada sem ter sido declarada, ou se alguma constante
	foi usada antes de ser declarada*/
}

void segundaPassada(FILE * arquivoEntrada, FILE * arquivoSaida) {
	char inputLine[1000];
	char *token;
	bool wholeLine = false;
	
	while (!feof(arquivoEntrada)) {
		if (fgets(inputLine, 1000, arquivoEntrada) != NULL) {
			if (inputLine[strlen(inputLine)-1] == '\n') /*Confere se toda a linha foi lida*/
				wholeLine = true;
			else
				wholeLine = false;
				
			token = strtok(inputLine, " \n");
			while (token != NULL) {
				if (token[strlen(token)-1] == ':') /*Label*/
					token = strtok(NULL, " \n");
				
				else if (token[0] == '.') { /*Diretiva*/
					analisaDiretiva(token, arquivoSaida);
					while (!wholeLine) {
						fgets(inputLine, 1000, arquivoEntrada);
						if (inputLine[strlen(inputLine)-1] == '\n')
							wholeLine = true;
						else
							wholeLine = false;
					}
					token = NULL;
				}
				
				else if (token[0] == '#') { /*Comentario*/
					while (!wholeLine) {
						fgets(inputLine, 1000, arquivoEntrada);
						if (inputLine[strlen(inputLine)-1] == '\n')
							wholeLine = true;
						else
							wholeLine = false;
					}
					token = NULL;
				}
				
				else { /*Instrucao*/
					analisaInstrucao(token, arquivoSaida);
					while (!wholeLine) {
						fgets(inputLine, 1000, arquivoEntrada);
						if (inputLine[strlen(inputLine)-1] == '\n')
							wholeLine = true;
						else
							wholeLine = false;
					}
					token = NULL;
				}
			}
		}
	}
}

int main(int argc, char *argv[]) {
	FILE * arquivoEntrada;
	FILE * arquivoSaida;
	char * nomeTemp;
	
	/* Inicializacao de variaveis*/
	if (argc < 2 || argc > 3) {
		printf("ERRO: Insira apenas um ou dois argumentos, correspondentes a, respectivamente, o nome do arquivo de entrada e (opcionalmente) o nome do arquivo de saida.\n");
		exit(1);
	}
	else if (argc == 2) {
		arquivoEntrada = fopen(argv[1], "r");
		nomeTemp = (char*) malloc(sizeof(argv[1])+(sizeof(char)*4)); /* Tamanho suficiente para nome do arquivo principal + .hex */
		strcpy(nomeTemp, argv[1]);
		strcat(nomeTemp, ".hex");
		arquivoSaida = fopen(nomeTemp, "w");
		free(nomeTemp);
	}
	else {
		arquivoEntrada = fopen(argv[1], "r");
		arquivoSaida = fopen(argv[2], "w");
	}
	
	/*Execucao principal */
	primeiraPassada(arquivoEntrada);
	rewind(arquivoEntrada);
	tracker[0] = 0;
	tracker[1] = 0;
	segundaPassada(arquivoEntrada, arquivoSaida);
	
	fclose(arquivoEntrada);
	fclose(arquivoSaida);
	return 0;
}
