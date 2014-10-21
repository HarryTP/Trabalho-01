#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

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

void erroSintaxe() { 
		printf("Entrou em erroSintaxe\n");
		exit();
};
void confereUsoLabels() { return; };
void conferePendentes(char nome[101], int tipo) { return; }; /*Procura na lista de pendencias e remove, tipo 0 para label e tipo 1 para constante*/
int confereNumeroNome(char *valor) { /*Testa se o numero ou nome eh valido*/
	/* Codigos de retorno:
	 * 0 - Nome
	 * 1 - Binario
	 * 2 - Octal
	 * 3 - Decimal
	 * 4 - Hexa
	 */
	int i;
	
	switch(valor[0]) {
		/* Numero em base =/= 0 */
		case '0':
			/* Hexadecimal */
			if (valor[1] == 'x' || valor[1] == 'X') {
				for (i = 2; i < strlen(valor); i++)
					if ( ! (valor[i] == '0' || valor[i] == '1' || valor[i] == '2' || valor[i] == '3' || valor[i] == '4' || valor[i] == '5' || valor[i] == '6' || valor[i] == '7' || valor[i] == '8' || valor[i] == '9' || valor[i] == 'A' || valor[i] == 'B' || valor[i] == 'C' || valor[i] == 'D' || valor[i] == 'E' || valor[i] == 'F') )
						erroSintaxe();
				return 4;
			}
			/* Octal */
			if (valor[1] == 'o' || valor[1] == 'O') {
				for (i = 2; i < strlen(valor); i++)
					if (! (valor[i] == '0' || valor[i] == '1' || valor[i] == '2' || valor[i] == '3' || valor[i] == '4' || valor[i] == '5' || valor[i] == '6' || valor[i] == '7') )
						erroSintaxe();
				return 2;
			}
			/* Binario */
			if (valor[1] == 'b' || valor[1] == 'B') {
				for (i = 2; i < strlen(valor); i++)
					if ( !(valor[i] == '0' || valor[i] == '1') )
						erroSintaxe();
				return 1;
			}
			else
				erroSintaxe();
		/* Numero em base decimal */
		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			for (i = 0; i < strlen(valor); i++)
				if (! (valor[i] == '0' || valor[i] == '1' || valor[i] == '2' || valor[i] == '3' || valor[i] == '4' || valor[i] == '5' || valor[i] == '6' || valor[i] == '7' || valor[i] == '8' || valor[i] == '9') )
					erroSintaxe();
			return 3;
		/* Nome */
		default:
			if (strlen(valor) > 100)
				erroSintaxe();
			for (i = 0; i < strlen(valor); i++) {
				if ( !isalpha(valor[i]) && !(valor[i] == '_') )
					erroSintaxe();
			}
			return 0;
	}
} 

void analisaDiretiva(char *dir, FILE *arquivoSaida) { /* Se arquivoSaida = NULL, primeira passada, senao segunda */
	char *token;
	
	if (arquivoSaida == NULL) {
		switch (dir) {
			case .word:
				if (tracker[1] == 1)
					erroSintaxe();
				else {
					token = strtok(NULL, " \n");
					confereNumeroConstante(token);
					tracker[0] = tracker[0]+1;
				}
		}
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
	nomeTemp[strlen(label)] = '\0';
	
	confereNumeroConstante(nomeTemp);
	
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
	conferePendentes(newLabel.nome);
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
