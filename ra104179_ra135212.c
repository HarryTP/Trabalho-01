/* MC404 A - Trabalho 01 - Montador do IAS
   Thales Carvalho Cremaschi Baumel   RA: 104179
   Caio Teixeira					  RA: 135212*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

struct Label { /* Struct usada para a lista ligada de rotulos */
	char nome[101];
	int endereco[2];
	struct Label * next;
};

struct Const { /* Struct usada para a lista ligada de constantes definidas pelo .set */
	char nome[101];
	long int valor;
	struct Const * next;
};

struct Pend { /* Struct usada para a lista de pendencias, usada para conferir o uso correto de labels e constantes */
	char nome[101];
	struct Pend * next;
};

struct Label *comecoLabels = NULL; /* Apontadores da lista de rotulos */
struct Label *finalLabels = NULL;

struct Const *comecoConst = NULL; /* Apontadores da lista de constantes */
struct Const *finalConst = NULL;

struct Pend *comecoPend = NULL; /* Apontadores da lista de pendencias */
struct Pend *finalPend = NULL;

int tracker[2] = {0, 0}; /* Posicao do proximo endereco a ser escrito no mapa de memoria */

int jumpType = 0; /* Variavel auxiliar para instrucoes com diferentes op codes para direita e esquerda
					0 = nao eh usada, 1 = esquerda, 2 = direita */

long int * nameVal; /* Variavel auxiliar utilizada na busca de rotulos e constantes nas listas */

void freeAll() { /* Funcao para liberar memoria alocada para as listas */
	struct Label * atualLabels;
	struct Const * atualConst;
	struct Pend * atualPend;
	
	if (comecoLabels) {
		atualLabels = comecoLabels;
		while (comecoLabels != finalLabels) {
			comecoLabels = comecoLabels->next;
			free(atualLabels);
			atualLabels = comecoLabels;
		}
		free(atualLabels);
		comecoLabels = NULL;
		finalLabels = NULL;
	}
	
	if (comecoConst) {
		atualConst = comecoConst;
		while (comecoConst != finalConst) {
			comecoConst = comecoConst->next;
			free(atualConst);
			atualConst = comecoConst;
		}
		free(atualConst);
		comecoConst = NULL;
		finalConst = NULL;
	}
	
	if (comecoPend) {
		atualPend = comecoPend;
		while (comecoPend != finalPend) {
			comecoPend = comecoPend->next;
			free(atualPend);
			atualPend = comecoPend;
		}
		free(atualPend);
		comecoPend = NULL;
		finalPend = NULL;
	}
}

/* Funcoes de erro */
void erroEnderecoInvalido() {
	printf("Erro: Endereco Invalido\n");
	freeAll();
	exit(1);
}
void erroSintaxe() { 
	printf("Erro de Sintaxe\n");
	freeAll();
	exit(1);
}
void erroUsoNome() { 
	printf("Erro no uso de rotulos ou constantes.\n");
	freeAll();
	exit(1);
}


void imprime(FILE * arquivoSaida, int tipo, long int param, int codigo) { /* Funcao para impressao no mapa de memoria */
	/* Tipos:
	 * 0 - Linha inteira (usada nas diretivas .word, .wfill e .align)
	 * 1 - Meia linha
	 */
	 
	unsigned int mask = 0b1111;
	int parts[10];
	int i;
	
	if (!tipo) {
		for (i = 0; i < 10; i++) {
			parts[i] = param & mask;
			param = param >> 4;
		}
		fprintf(arquivoSaida, "%03X %X%X %X%X%X %X%X %X%X%X\n", tracker[0], parts[9], parts[8], parts[7], parts[6], parts[5], parts[4], parts[3], parts[2], parts[1], parts[0]);
	}
	else {
		for (i = 0; i < 2; i++) {
			parts[i] = codigo & mask;
			codigo = codigo >> 4;
		}
		for (i = 2; i < 5; i++) {
			parts[i] = param & mask;
			param = param >> 4;
		}
		if (!tracker[1]) {
			fprintf(arquivoSaida, "%03X %X%X %X%X%X ", tracker[0], parts[1], parts[0], parts[4], parts[3], parts[2]);
		}
		else {
			fprintf(arquivoSaida, "%X%X %X%X%X\n", parts[1], parts[0], parts[4], parts[3], parts[2]);
		}
	}
}

long int converteStringNumero (char * valor, int tipo) { /* Converte um numero, dado na string valor, para um inteiro */
	/* Tipos:
	 * 1 - Binario +
	 * 2 - Binario -
	 * 3 - Octal +
	 * 4 - Octal -
	 * 5 - Decimal +
	 * 6 - Decimal -
	 * 7 - Hexa +
	 * 8 - Hexa -
	 */
	long int result;
	char temp[101];
	
	switch (tipo) {
		case 1:
			strncpy(temp, valor+(sizeof(char)*2), (strlen(valor)-2) );
			temp[strlen(valor)-2] = '\0';
			result = strtol(temp, NULL, 2);
			break;
		case 2:
			strncpy(temp, valor+(sizeof(char)*3), (strlen(valor)-3) );
			temp[strlen(valor)-3] = '\0';
			result = strtol(temp, NULL, 2);
			result = -result;
			break;
		case 3:
			strncpy(temp, valor+(sizeof(char)*2), (strlen(valor)-2) );
			temp[strlen(valor)-2] = '\0';
			result = strtol(temp, NULL, 8);
			break;
		case 4:
			strncpy(temp, valor+(sizeof(char)*3), (strlen(valor)-3) );
			temp[strlen(valor)-3] = '\0';
			result = strtol(valor+(sizeof(char)*3), NULL, 8);
			result = -result;
			break;
		case 5: case 6: 
			strncpy(temp, valor, strlen(valor) );
			temp[strlen(valor)] = '\0';
			result = strtol(temp, NULL, 10);
			break;
		case 7: case 8:
			strncpy(temp, valor, strlen(valor) );
			temp[strlen(valor)] = '\0';
			result = strtol(temp, NULL, 16);
			break;
		default:
			erroSintaxe();
	}
	return result;
}

void armazenaPendencia(char *nome) { /* Funcao utilizada para inserir um novo elemento na lista de pendencias */
	struct Label * label = NULL;
	struct Const * set = NULL;
	struct Pend * pend = NULL;
	struct Pend * newPend;
	
	/* Primeiramente, a funcao percorre as listas de rotulos e constantes */
	label = comecoLabels;
	while (label) {
		if (!strcmp(label->nome, nome))
			return;
		else
			label = label->next;
	}
	
	set = comecoConst;
	while (set) {
		if (!strcmp(set->nome, nome))
			return;
		else
			set = set->next;
	}
	
	/* Caso nao exista um rotulo ou constante com o nome, adiciona na lista de pendencias o nome */
	if (comecoPend == NULL) {
		newPend = malloc(sizeof(struct Pend));
		
		strcpy(newPend->nome, nome);
		newPend->next = NULL;
		
		comecoPend = newPend;
		finalPend = newPend;
	}
	else {
		pend = comecoPend;
		while (pend) {
			if (!strcmp(pend->nome, nome))
				return;
			else
				pend = pend->next;
		}
		
		newPend = malloc(sizeof(struct Pend));
		
		strcpy(newPend->nome, nome);
		newPend->next = NULL;
		
		finalPend->next = newPend;
		finalPend = newPend;
	}
}

void confereConflitoNome(char *nome) { /* Confere se um nome ja nao foi usado */
	struct Label * label = NULL;
	struct Const * set = NULL;
	
	label = comecoLabels;
	while (label) {
		if (!strcmp(label->nome, nome))
			erroUsoNome();
		else
			label = label->next;
	}
	
	set = comecoConst;
	while (set) {
		if (!strcmp(set->nome, nome))
			erroUsoNome();
		else
			set = set->next;
	}
}

void conferePendentes(char nome[101], int tipo) { /* Procura na lista de pendencias */ 
	/*  Tipo 0 = Label
		Tipo 1 = Constante */
	 
	 struct Pend * last = NULL;
	 struct Pend * current = NULL;
	 
	if (comecoPend != NULL) {
		current = comecoPend;
		if (tipo == 0) { /* No caso de ser um rotulo, remove a pendencia da lista */
			while (current) {
				if ( !strcmp(current->nome, nome) ) {
					if (comecoPend == finalPend && current == comecoPend) { /* Lista com 1 elemento */
						comecoPend = NULL;
						finalPend = NULL;
						free(current);
						return;
					}
					else if (current == comecoPend) { /* Primeiro em lista > 1 */
						comecoPend = comecoPend->next;
						free(current);
						return;
					}
					else if (current == finalPend) { /* Ultimo elemento em lista > 1 */
						finalPend = last;
						finalPend->next = NULL;
						free(current);
						return;
					}
					else { /* Caso geral: meio da lista */
						last->next = current->next;
						free(current);
						return;
					}
				}
				else {
					last = current;
					current = current->next;
				}
			}
		}
		
		else if (tipo == 1) { /* Para as constantes, se o nome estiver nas pendencias eh emitido um erro,
								 porque a constante foi utilizada antes de ser declarada */
			while (current) {
				if (!strcmp(current->nome, nome))
					erroUsoNome();
				else
					current = current->next;
			}
		}
	}
}

char * convertToLower(char *str) { /* Funcao que converte todos os tokens lidos para minusculas, para que o programa
									  nao diferencie entre maiusculas e minusculas*/
	int i;
	
	if (str != NULL) {
		for (i = 0; i < strlen(str); i++)
			str[i] = tolower(str[i]);
	}
	
	return str;
}

int confereNumeroNome(char * valor) { /*Testa se o numero ou nome eh valido*/
	/* Codigos de retorno:
	 * 0 - Nome
	 * 1 - Binario +
	 * 2 - Binario -
	 * 3 - Octal +
	 * 4 - Octal -
	 * 5 - Decimal +
	 * 6 - Decimal -
	 * 7 - Hexa +
	 * 8 - Hexa -
	 */
	int i;
	int pos = 0;
	bool negativo = false;
	
	if (valor[pos] == '-') {
		negativo = true;
		pos++;
	}
	
	switch(valor[pos]) {
		/* Numero em base =/= 10 */
		case '0':
			pos++;
			/* Hexadecimal */
			if (valor[pos] == 'x') {
				for (i = pos+1; i < strlen(valor); i++)
					if ( ! (valor[i] == '0' || valor[i] == '1' || valor[i] == '2' || valor[i] == '3' || valor[i] == '4' || valor[i] == '5' || valor[i] == '6' || valor[i] == '7' || valor[i] == '8' || valor[i] == '9' || valor[i] == 'a' || valor[i] == 'b' || valor[i] == 'c' || valor[i] == 'd' || valor[i] == 'e' || valor[i] == 'f') )
						erroSintaxe();
				if (negativo) return 8;
				else return 7;
			}
			/* Octal */
			else if (valor[pos] == 'o') {
				for (i = pos+1; i < strlen(valor); i++)
					if (! (valor[i] == '0' || valor[i] == '1' || valor[i] == '2' || valor[i] == '3' || valor[i] == '4' || valor[i] == '5' || valor[i] == '6' || valor[i] == '7') )
						erroSintaxe();
				if (negativo) return 4;
				else return 3;
			}
			/* Binario */
			else if (valor[pos] == 'b') {
				for (i = pos+1; i < strlen(valor); i++)
					if ( !(valor[i] == '0' || valor[i] == '1') )
						erroSintaxe();
				if (negativo) return 2;
				else return 1;
			}
			/* Caso apenas um 0 (decimal) */
			else if (valor[pos] == '\0')
				return 5;
			/* Caso seja um decimal comecando em 0 */
			else if ( valor[pos] == '0' || valor[pos] == '1' || valor[pos] == '2' || valor[pos] == '3' || valor[pos] == '4' || valor[pos] == '5' || valor[pos] == '6' || valor[pos] == '7' || valor[pos] == '8' || valor[pos] == '9' ) { 
				for (i = pos; i < strlen(valor); i++)
					/* Se nao for um numero */
					if (!(valor[i] == '0' || valor[i] == '1' || valor[i] == '2' || valor[i] == '3' || valor[i] == '4' || valor[i] == '5' || valor[i] == '6' || valor[i] == '7' || valor[i] == '8' || valor[i] == '9'))
						erroSintaxe();
				if (negativo) return 6;
				else return 5;
			}
			else
				erroSintaxe();
		/* Numero em base decimal */
		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			for (i = pos; i < strlen(valor); i++)
				/* Se nao for um numero */
				if (!(valor[i] == '0' || valor[i] == '1' || valor[i] == '2' || valor[i] == '3' || valor[i] == '4' || valor[i] == '5' || valor[i] == '6' || valor[i] == '7' || valor[i] == '8' || valor[i] == '9'))
					erroSintaxe();
			if (negativo) return 6;
			else return 5;
		/* Nome */
		default:
			if (negativo == true || strlen(valor) > 100)
				erroSintaxe();
			for (i = 0; i < strlen(valor); i++) {
				if ( !isalnum(valor[i]) && !(valor[i] == '_') )
					erroSintaxe();
			}
			return 0;
	}
}

int confereTipo(char *valor) { /* Testa qual o tipo de valor (utilizada na segunda passada) */
	/* Codigos de retorno:
	 * 0 - Nome
	 * 1 - Binario +
	 * 2 - Binario -
	 * 3 - Octal +
	 * 4 - Octal -
	 * 5 - Decimal +
	 * 6 - Decimal -
	 * 7 - Hexa +
	 * 8 - Hexa -
	 */
	int i;
	int pos = 0;
	bool negativo = false;
	
	if (valor[pos] == '-') {
		negativo = true;
		pos++;
	}
	
	switch(valor[pos]) {
		/* Numero em base =/= 10 */
		case '0':
			pos++;
			/* Hexadecimal */
			if (valor[pos] == 'x') {
				if (negativo) return 8;
				else return 7;
			}
			/* Octal */
			else if (valor[pos] == 'o') {
				if (negativo) return 4;
				else return 3;
			}
			/* Binario */
			else if (valor[pos] == 'b') {
				if (negativo) return 2;
				else return 1;
			}
			/* Caso apenas um 0 ou um decimal comecado em 0 */
			else if (valor[pos] == '\0' || valor[pos] == '0' || valor[pos] == '1' || valor[pos] == '2' || valor[pos] == '3' || valor[pos] == '4' || valor[pos] == '5' || valor[pos] == '6' || valor[pos] == '7' || valor[pos] == '8' || valor[pos] == '9' ) {
				if (negativo) return 6;
				else return 5;
			}
			else
				erroSintaxe();
		/* Numero em base decimal */
		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			if (negativo) return 6;
			else return 5;
		/* Nome */
		default:
			return 0;
	}
}

long int * buscaNome(char nome[101], long int res[3]) { /* Busca uma label ou constante com nome "nome" e retorna o valor, ou NULL se nao estiver em nenhuma das listas */
	/* Retorno:
	 * [0] = Tipo (0 label, 1 constante)
	 * [1] = Valor
	 * [2] = Direita ou esquerda, caso seja label
	 */
	 
	 struct Label * label;
	 struct Const * constante;
	 
	 /* Busca na lista de labels, se nao achar apenas encerra o loop e a condicao */
	 if (comecoLabels) {
		 label = comecoLabels;
		 while (label) {
			 if(!strcmp(label->nome, nome)) {
				res[0] = 0;
				res[1] = label->endereco[0];
				res[2] = label->endereco[1];
				return res;
			 }
			 else
				label = label->next;
		 }
	 }
	 
	 /* Busca na lista de constantes, se nao achar apenas encerra o loop e a condicao */
	 if (comecoConst) {
		 constante = comecoConst;
		 while (constante) {
			 if(!strcmp(constante->nome, nome)) {
				res[0] = 1;
				res[1] = constante->valor;
				res[2] = 0;
				return res;
			 }
			 else
				constante = constante->next;
		 }
	 }
	 
	 return NULL;
}

char * isolaVariavel(char *argumento, int tipo) { /* Funcao que isola e confere o argumento de uma operacao, contido entre m() */
	/*Tipo 0 = Operacao com um argumento
	  Tipo 1 = Jump
	  Tipo 2 = Store*/
	
	char variavel[107];
	char temp[6];
	
	jumpType = 0;
	
	if (strlen(argumento) > 109)
		erroSintaxe();
	else {
		if (!(argumento[0] == 'm' && argumento[1] == '(' && argumento[strlen(argumento)-1] == ')'))
			erroSintaxe();
		else {
			switch(tipo) {
				case 0:
					if (strlen(argumento) > 103)
						erroSintaxe();
					else {
						strncpy(variavel, argumento+(sizeof(char)*2), strlen(argumento)-3);
						variavel[strlen(argumento)-3] = '\0';
						break;
					}
				case 1:
					strncpy(variavel, argumento+(sizeof(char)*2), strlen(argumento)-3);
					variavel[strlen(argumento)-3] = '\0';
					strncpy(temp, variavel+(strlen(variavel)-5), 5);
					temp[5] = '\0';
					if (!strcmp(temp, ",0:19")) {
						variavel[strlen(variavel)-5] = '\0';
						if (strlen(variavel) > 100)
							erroSintaxe();
						else {
							jumpType = 1;
							break;
						}
					}
					else if (!strcmp(temp, "20:39")) {
						if (variavel[strlen(variavel)-6] != ',')
							erroSintaxe();
						else {
							variavel[strlen(variavel)-6] = '\0';
							if (strlen(variavel) > 100)
								erroSintaxe();
							else {
								jumpType = 2;
								break;
							}
						}
					}
					else if (confereNumeroNome(variavel) != 0)
						erroSintaxe();
					else
						break;
				case 2:
					strncpy(variavel, argumento+(sizeof(char)*2), strlen(argumento)-3);
					variavel[strlen(argumento)-3] = '\0';
					strncpy(temp, variavel+(strlen(variavel)-5), 5);
					temp[5] = '\0';
					if (!strcmp(temp, ",8:19")) {
						variavel[strlen(variavel)-5] = '\0';
						if (strlen(variavel) > 100)
							erroSintaxe();
						else {
							jumpType = 1;
							break;
						}
					}
					else if (!strcmp(temp, "28:39")) {
						if (variavel[strlen(variavel)-6] != ',')
							erroSintaxe();
						else {
							variavel[strlen(variavel)-6] = '\0';
							if (strlen(variavel) > 100)
								erroSintaxe();
							else {
								jumpType = 2;
								break;
							}
						}
					}
					else if (confereNumeroNome(variavel) != 0)
						erroSintaxe();
					else
						break;
			}
		}
	}
	argumento = variavel;
	return argumento;
}

void armazenaConstante(char nome[101], long int valor) { 
	struct Const * newConst;
	int i;
	
	newConst = malloc(sizeof(struct Const));
		
	/* Seta a nova variavel */
	strcpy(newConst->nome, nome);
	newConst->valor = valor;
	newConst->next = NULL;
	
	/* Concatena na lista */
	if (comecoConst == NULL) {
		comecoConst = newConst;
		finalConst = newConst;
	}
	else {
		finalConst->next = newConst;
		finalConst = newConst;
	}
}

void analisaInstrucao(char *op, FILE *arquivoSaida) { /* Se arquivoSaida = NULL, primeira passada, senao segunda */
	char *token;
	int tipo;
	long int val;
	
	/* Reset */
	nameVal[0] = 0;
	nameVal[1] = 0;
	nameVal[2] = 0;
	
	if (arquivoSaida == NULL) { /* Na primeira passada, apenas confere se a sintaxe esta certa, e avanca o tracker */
		if (!strcmp(op, "ldmq")) {
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "ldmqm")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "str")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "load")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "ldn")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "ldabs")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "jmp")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 1);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "jgez")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 1);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "add")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "addabs")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "sub")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "subabs")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "mul")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "div")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "lsh")) {
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "rsh")) {
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else if (!strcmp(op, "stm")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 2);
			tipo = confereNumeroNome(token);
			if (tipo == 0)
				armazenaPendencia(token);
			else {
				val = converteStringNumero(token, tipo);
				if (val > 4095 || val < 0)
					erroEnderecoInvalido();
			}
			if (tracker[0] < 1024) {
				if (tracker[1] == 0)
					tracker[1] = 1;
				else {
					tracker[0] = tracker[0]+1;
					tracker[1] = 0;
				}
			}
			else
				erroEnderecoInvalido();
		}
		else
			erroSintaxe();
	}
	else { /* Na segunda passada, imprime a instrucao no mapa de memoria */
		if (!strcmp(op, "ldmq")) {
			imprime(arquivoSaida, 1, 0, 10);
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "ldmqm")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 9);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 9);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "str")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 33);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 33);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "load")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 1);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 1);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "ldn")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 2);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 2);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "ldabs")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 3);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 3);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "jmp")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 1);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[0] == 0) {
					if (jumpType != 0)
						erroSintaxe();
					else if (nameVal[2] == 0)
						imprime(arquivoSaida, 1, nameVal[1], 13);
					else
						imprime(arquivoSaida, 1, nameVal[1], 14);
				}
				else if (jumpType == 0)
					erroSintaxe();
				else if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else if (jumpType == 1)
					imprime(arquivoSaida, 1, nameVal[1], 13);
				else
					imprime(arquivoSaida, 1, nameVal[1], 14);
			}
			else {
				val = converteStringNumero(token, tipo);
				if (jumpType == 1)
					imprime(arquivoSaida, 1, val, 13);
				else if (jumpType == 2)
					imprime(arquivoSaida, 1, val, 14);
				else
					erroSintaxe();
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "jgez")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 1);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[0] == 0) {
					if (jumpType != 0)
						erroSintaxe();
					else if (nameVal[2] == 0)
						imprime(arquivoSaida, 1, nameVal[1], 15);
					else
						imprime(arquivoSaida, 1, nameVal[1], 16);
				}
				else if (jumpType == 0)
					erroSintaxe();
				else if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else if (jumpType == 1)
					imprime(arquivoSaida, 1, nameVal[1], 15);
				else
					imprime(arquivoSaida, 1, nameVal[1], 16);
			}
			else {
				val = converteStringNumero(token, tipo);
				if (jumpType == 1)
					imprime(arquivoSaida, 1, val, 15);
				else if (jumpType == 2)
					imprime(arquivoSaida, 1, val, 16);
				else
					erroSintaxe();
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "add")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 5);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 5);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "addabs")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 7);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 7);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "sub")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 6);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 6);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "subabs")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 8);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 8);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "mul")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 11);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 11);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "div")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 0);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else
					imprime(arquivoSaida, 1, nameVal[1], 12);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 1, val, 12);
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "lsh")) {
			imprime(arquivoSaida, 1, 0, 20);
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "rsh")) {
			imprime(arquivoSaida, 1, 0, 21);
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "stm")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			token = isolaVariavel(token, 2);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal[0] == 0) {
					if (jumpType != 0)
						erroSintaxe();
					else if (nameVal[2] == 0)
						imprime(arquivoSaida, 1, nameVal[1], 18);
					else
						imprime(arquivoSaida, 1, nameVal[1], 19);
				}
				else if (jumpType == 0)
					erroSintaxe();
				else if (nameVal[1] < 0 || nameVal[1] > 4095)
					erroEnderecoInvalido();
				else if (jumpType == 1)
					imprime(arquivoSaida, 1, nameVal[1], 18);
				else
					imprime(arquivoSaida, 1, nameVal[1], 19);
			}
			else {
				val = converteStringNumero(token, tipo);
				if (jumpType == 1)
					imprime(arquivoSaida, 1, val, 18);
				else if (jumpType == 2)
					imprime(arquivoSaida, 1, val, 19);
				else
					erroSintaxe();
			}
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else
			erroSintaxe();
	}
}

void analisaDiretiva(char *dir, FILE *arquivoSaida) { /* Se arquivoSaida = NULL, primeira passada, senao segunda */
	char *token;
	char nomeConst[101];
	int tipo;
	int n;
	int i;
	long int val;
	
	/* Reset */
	nameVal[0] = 0;
	nameVal[1] = 0;
	nameVal[2] = 0;
	
	if (arquivoSaida == NULL) { /* Na primeira passada, confere a sintaxe e os valores, avanca o tracker
								   e armazena as constantes da diretiva .set */
		if (!strcmp(dir, ".word")) {
			if (tracker[1] == 1)
				erroSintaxe();
			else {
				token = strtok(NULL, " \n");
				token = convertToLower(token);
				tipo = confereNumeroNome(token);
				if (tipo == 0)
					armazenaPendencia(token);
				else {
					val = converteStringNumero(token, tipo);
					if (val < -549755813888 || val > 549755813887)
						erroEnderecoInvalido();
				}
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
				token = convertToLower(token);
				tipo = confereNumeroNome(token);
				if (tipo == 0) {
					nameVal = buscaNome(token, nameVal);
					if (nameVal == NULL || nameVal[0] == 0)
						erroSintaxe();
					else {
						if (nameVal[1] <= 0)
							erroSintaxe();
						else if (nameVal[1] > 1024)
							erroEnderecoInvalido();
						else
							n = nameVal[1];
					}
				}
				else {
					n = converteStringNumero(token, tipo);
					if (n <= 0)
						erroSintaxe();
					else if (n > 1024)
						erroEnderecoInvalido();
				}
				token = strtok(NULL, " \n");
				token = convertToLower(token);
				tipo = confereNumeroNome(token);
				if (tipo == 0)
					armazenaPendencia(token);
				else {
					val = converteStringNumero(token, tipo);
					if (val < -549755813888 || val > 549755813887)
						erroSintaxe();
				}
				for (i = 0; i < n; i++)
					tracker[0] = tracker[0]+1;
				if (tracker[0] > 1024)
					erroEnderecoInvalido();
			}
		}
		else if(!strcmp(dir, ".org")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereNumeroNome(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal == NULL || nameVal[0] == 0)
					erroSintaxe();
				else {
					if (nameVal[1] < 0 || nameVal[1] > 1023)
						erroEnderecoInvalido();
					else
						n = nameVal[1];
				}
			}
			else {
				n = converteStringNumero(token, tipo);
				if (n < 0 || n > 1023)
					erroEnderecoInvalido();
				}
			tracker[0] = n;
			tracker[1] = 0;
		}
		else if(!strcmp(dir, ".align")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereNumeroNome(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal == NULL || nameVal[0] == 0)
					erroSintaxe();
				else {
					if (nameVal[1] <= 0)
						erroSintaxe();
					else if (nameVal[1] > 1024)
						erroEnderecoInvalido();
					else
						n = nameVal[1];
				}
			}
			else {
				n = converteStringNumero(token, tipo);
				if (n <= 0)
					erroSintaxe();
				else if (n > 1024)
					erroEnderecoInvalido();
			}
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
			token = convertToLower(token);
			tipo = confereNumeroNome(token);
			if (tipo != 0)
				erroSintaxe();
			else {
				strncpy(nomeConst, token, strlen(token));
				nomeConst[strlen(token)] = '\0';
			}
			confereConflitoNome(nomeConst);
			conferePendentes(nomeConst, 1);
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereNumeroNome(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				if (nameVal == NULL || nameVal[0] == 0)
					erroSintaxe();
				else
					val = nameVal[1];
			}
			else {
				val = converteStringNumero(token, tipo);
				if (val < -549755813888 || val > 549755813887)
					erroSintaxe();
			}
			armazenaConstante(nomeConst, val);
		}
		else
			erroSintaxe();
	}
	else { /* Na segunda passada imprime no mapa de memoria, e ignora a .set */
		if (!strcmp(dir, ".word")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				imprime(arquivoSaida, 0, nameVal[1], 0);
			}
			else {
				val = converteStringNumero(token, tipo);
				imprime(arquivoSaida, 0, val, 0);
			}
			tracker[0] = tracker[0]+1;
		}
		else if(!strcmp(dir, ".wfill")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				n = nameVal[1];
			}
			else {
				n = converteStringNumero(token, tipo);
			}
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				val = nameVal[1];
			}
			else {
				val = converteStringNumero(token, tipo);
			}
			for (i = 0; i < n; i++) {
				imprime(arquivoSaida, 0, val, 0);
				tracker[0] = tracker[0]+1;
			}
		}
		else if(!strcmp(dir, ".org")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				n = nameVal[1];
			}
			else
				n = converteStringNumero(token, tipo);
			tracker[0] = n;
			tracker[1] = 0;
		}
		else if(!strcmp(dir, ".align")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereTipo(token);
			if (tipo == 0) {
				nameVal = buscaNome(token, nameVal);
				n = nameVal[1];
			}
			else
				n = converteStringNumero(token, tipo);
			if (tracker[1] == 1) {
				imprime(arquivoSaida, 1, 0, 0);
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
			while (tracker[0]%n) {
				imprime(arquivoSaida, 0, 0, 0);
				tracker[0] = tracker[0]+1;
			}
		}
		else if(!strcmp(dir, ".set")) {
			token = strtok(NULL, " \n");
			token = strtok(NULL, " \n");
		}
		else
			erroSintaxe();
	}
}

void armazenaLabel(char *label) { 
	struct Label * newLabel;
	char nomeTemp[101] = "";
	int i;
	
	newLabel = malloc(sizeof(struct Label));
	
	/* Tamanho invalido */
	if (strlen(label) > 101)
		erroSintaxe();
	
	/* Retira : */
	strncpy (nomeTemp, label, strlen(label)-1);
	nomeTemp[strlen(label)-1] = '\0';
	
	i = confereNumeroNome(nomeTemp);
	if (i != 0)
		erroSintaxe();
	else
		confereConflitoNome(nomeTemp);
	
	/* Seta a nova variavel */
	strcpy(newLabel->nome, nomeTemp);
	newLabel->endereco[0] = tracker[0];
	newLabel->endereco[1] = tracker[1];
	newLabel->next = NULL;
	
	/* Concatena na lista */
	if (comecoLabels == NULL) {
		comecoLabels = newLabel;
		finalLabels = newLabel;
	}
	else {
		finalLabels->next = newLabel;
		finalLabels = newLabel;
	}
	
	/* Confere pendencias */
	conferePendentes(newLabel->nome, 0);
}

void primeiraPassada(FILE * arquivoEntrada) { /* Durante a primeira passada, nada eh impresso no mapa de memoria
												 o programa interpreta o arquivo de entrada, buscando por erros,
												 e armazenando as labels e constantes */
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
			token = convertToLower(token);
			while (token != NULL) {
				if (token[strlen(token)-1] == ':') { /*Label*/
					if (!readLabel && !readDirective && !readOperation) {
						armazenaLabel(token);
						readLabel = true;
						token = strtok(NULL, " \n");
						token = convertToLower(token);
					}
					else
						erroSintaxe();
				}
				
				else if (token[0] == '.') { /*Diretiva*/
					if (!readDirective && !readOperation) {
						analisaDiretiva(token, NULL);
						readDirective = true;
						token = strtok(NULL, " \n");
						token = convertToLower(token);
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
						token = convertToLower(token);
					}
					else
						erroSintaxe();
				}
			}
			readLabel = false;
			readDirective = false;
			readOperation = false;
		}
	}
	
	if (comecoPend != NULL)
		erroUsoNome();
}

void segundaPassada(FILE * arquivoEntrada, FILE * arquivoSaida) { /* Na segunda passada, o programa praticamente so imprime
																	 o mapa de memoria, apenas alguns erros que nao poderiam ter
																	 sido detectados na primeira passada sao conferidos */
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
			token = convertToLower(token);
			while (token != NULL) {
				if (token[strlen(token)-1] == ':') { /*Label*/
					token = strtok(NULL, " \n");
					token = convertToLower(token);
				}
				
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
	
	arquivoEntrada = fopen(argv[1], "r");
	if (arquivoEntrada == NULL) {
		printf("ERRO: Arquivo de entrada invalido.\n");
		exit(1);
	}
	
	nameVal = malloc(sizeof(long int)*3);
	nameVal[0] = 0;
	nameVal[1] = 0;
	nameVal[2] = 0;
	
	/*Execucao principal */
	primeiraPassada(arquivoEntrada);
	
	if (argc == 2) {
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
	
	rewind(arquivoEntrada);
	tracker[0] = 0;
	tracker[1] = 0;
	segundaPassada(arquivoEntrada, arquivoSaida);
	
	if (tracker[1] == 1) 
		imprime(arquivoSaida, 1, 0, 0);
	
	free(nameVal);
	freeAll();
	fclose(arquivoEntrada);
	fclose(arquivoSaida);
	return 0;
}
