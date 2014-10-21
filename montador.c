#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct Label {
	char nome[101];
	int endereco[2];
	struct Label * next;
};

struct Const {
	char nome[101];
	int valor;
	struct Const * next;
};

struct Pend {
	char nome[101];
	struct Pend * next;
};

struct Label *comecoLabels = NULL;
struct Label *finalLabels = NULL;
struct Const *comecoConst = NULL;
struct Const *finalConst = NULL;
struct Pend *comecoPend = NULL;
struct Pend *finalPend = NULL;
int tracker[2] = {0, 0};

void analisaInstrucao(char *op, FILE *arquivoSaida) { return; }; /* Se arquivoSaida = NULL, primeira passada, senao segunda */

void erroSintaxe() { return; };
void erroEnderecoInvalido() { return; };

void confereUsoLabels() { return; };
void armazenaConstante(char nome[101], int valor) { return; };
void confereConflitoNome(char *nome) { return; }; /*Confere se um nome ja nao foi usado*/
void conferePendentes(char nome[101], int tipo) { return; }; /*Procura na lista de pendencias e remove, tipo 0 para label e tipo 1 para constante*/
int confereNumeroNome(char *valor) { return 0; }; /*Testa se o numero ou nome eh valido*/

void analisaDiretiva(char *dir, FILE *arquivoSaida) { /* Se arquivoSaida = NULL, primeira passada, senao segunda */
	char *token;
	char nomeConst[101];
	int tipo;
	int n;
	int i;
	
	if (arquivoSaida == NULL) {
		if (!strcmp(dir, ".word")) {
			if (tracker[1] == 1)
				erroSintaxe();
			else {
				token = strtok(NULL, " \n");
				tipo = confereNumeroNome(token);
				/*Caso seja um nome, tratar na lista de pendencias*/
				if (tracker[0] > 1023)
					erroEnderecoInvalido();
				tracker[0] = tracker[0]+1;
			}
		}
		else if(!strcmp(dir, ".wfill")) {
			if (tracker[1] == 1)
				erroSintaxe();
			else {
				token = strtok(NULL, " \n");
				tipo = confereNumeroNome(token);
				/*Trata o primeiro argumento de wfill (n)*/
				token = strtok(NULL, " \n");
				tipo = confereNumeroNome(token);
				/*Caso seja um nome, tratar na lista de pendencias*/
				for (i = 0; i < n; i++)
					tracker[0] = tracker[0]+1;
				if (tracker[0] > 1024)
					erroEnderecoInvalido();
			}
		}
		else if(!strcmp(dir, ".org")) {
			token = strtok(NULL, " \n");
			tipo = confereNumeroNome(token);
			/*Trata o numero e coloca em n*/
			tracker[0] = n;
			tracker[1] = 0;
		}
		else if(!strcmp(dir, ".align")) {
			token = strtok(NULL, " \n");
			tipo = confereNumeroNome(token);
			/*Trata o numero e coloca em n*/
			if (tracker[1] == 1) {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
			while (tracker[0]%n)
				tracker[0] = tracker[0]+1;
			if (tracker[0] > 1024)
				erroEnderecoInvalido();
		}
		else if(!strcmp(dir, ".set")) {
			token = strtok(NULL, " \n");
			tipo = confereNumeroNome(token);
			if (tipo != 0)
				erroSintaxe();
			else {
			strncpy(nomeConst, token, strlen(token))
			nomeConst[strlen(token)] = '\0';
			}
			confereConflitoNome(nomeConst);
			conferePendentes(nomeConst, 1);
			token = strtok(NULL, " \n");
			tipo = confereNumeroNome(token);
			/*Trata o numero e coloca em n*/
			armazenaConstante(nomeConst, n);
		}
		else
			erroSintaxe();
	}
}


void armazenaLabel(char *label) { 
	struct Label newLabel;
	char nomeTemp[101] = "";
	int i;
	
	/* Tamanho invalido */
	if (strlen(label) > 101)
		erroSintaxe();
	
	/* Retira : */
	strncpy (nomeTemp, label, strlen(label)-1);
	nomeTemp[strlen(label)-1] = '\0';
	
	/* Analise de sintaxe */
	for (i = 0; i < strlen(nomeTemp); i++) {
		if ( !isalpha(nomeTemp[i]) && !(nomeTemp[i] == '_') )
			erroSintaxe();
	}
	
	/*confereNumeroNome(nomeTemp); <- poderia ser usada no lugar do pedaço de cima. */
	confereConflitoNome(nomeTemp);
	
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
	
	/* Confere pendencias */
	conferePendentes(newLabel.nome, 0);
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
