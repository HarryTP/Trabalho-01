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
	long int valor;
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

void erroEnderecoInvalido() {
	printf("Entrou em erroEnderecoInvalido\n");
	exit(1);
}
void erroSintaxe() { 
	printf("Entrou em erroSintaxe\n");
	exit(1);
}
void erroUsoNome() { 
	printf("Entrou em erroUsoNome\n");
	exit(1);
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
	
	switch (tipo) {
		case 1:
			result = strtol(valor+(sizeof(char)*2), NULL, 2);
		case 2:
			result = strtol(valor+(sizeof(char)*3), NULL, 2);
			result = -result;
		case 3:
			result = strtol(valor+(sizeof(char)*2), NULL, 8);
		case 4:
			result = strtol(valor+(sizeof(char)*3), NULL, 8);
			result = -result;
		case 5: case 6: case 7: case 8:
			result = strtol(valor, NULL, 0);
		default:
			erroSintaxe();
	}
	return result;
}

void armazenaPendencia(char *nome) {
	struct Label * label = NULL;
	struct Const * set = NULL;
	struct Pend * pend = NULL;
	struct Pend * newPend;
	
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
	
	if (comecoPend == NULL) {
		newPend = malloc(sizeof(struct Pend));
		
		newPend->nome = nome;
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
		
		newPend->nome = nome;
		newPend->next = NULL;
		
		finalPend->next = newPend;
		finalPend = newPend;
	}
}

void confereConflitoNome(char *nome) { /*Confere se um nome ja nao foi usado*/
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

void conferePendentes(char nome[101], int tipo) { /*Procura na lista de pendencias e remove*/ 
	/*  Tipo 0 = Label
		Tipo 1 = Constante */
	 
	 struct Pend * last = NULL;
	 struct Pend * current = NULL;
	 
	if (comecoPend != NULL) {
		current = comecoPend;
		if (tipo == 0) { 
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
		
		else if (tipo == 1) {
			while (current) {
				if (!strcmp(current->nome, nome))
					erroUsoNome();
				else
					current = current->next;
			}
		}
	}
}

char * convertToLower(char *str) {
	int i;
	
	if (str != NULL) {
		for (i = 0; i < strlen(str); i++)
			str[i] = tolower(str[i]);
	}
	
	return str;
}

int confereNumeroNome(char *valor) { /*Testa se o numero ou nome eh valido*/
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
			else if (valor[pos] == NULL) /* Caso apenas um 0 (decimal) */
				return 5;
			else if ( isdigit(valor[pos]) ) { /* Caso seja um decimal comecando em 0 */
				for (i = pos; i < strlen(valor); i++)
					if (!isdigit(valor[i]))
						erroSintaxe();
				if (negativo) return 6;
				else return 5;
			}
			else
				erroSintaxe();
		/* Numero em base decimal */
		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			for (i = pos; i < strlen(valor); i++)
				if (!isdigit(valor[i]))
					erroSintaxe();
			if (negativo) return 6;
			else return 5;
		/* Nome */
		default:
			if (negativo == true || strlen(valor) > 100)
				erroSintaxe();
			for (i = 0; i < strlen(valor); i++) {
				if ( !isalpha(valor[i]) && !(valor[i] == '_') )
					erroSintaxe();
			}
			return 0;
	}
}

char * isolaVariavel(char *argumento, int tipo) {
	/*Tipo 0 = Operacao com um argumento
	  Tipo 1 = Jump
	  Tipo 2 = Store*/
	
	char *variavel;
	
	if (!(argumento[0] == 'm' && argumento[1] == '(' && argumento[strlen(argumento)-1] == ')'))
		erroSintaxe();
	else {
		switch(tipo) {
			case 0:
				strncpy(variavel, argumento+(sizeof(char)*2), strlen(argumento)-3);
				variavel[strlen(argumento)-3] = '\0';
				break;
			case 1:
				
		}
	}
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
	
	if (arquivoSaida == NULL) {
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa, podendo haver 0:19 ou 20:39*/
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
			/*Isola o valor dentro de M(), e analisa, podendo haver 0:19 ou 20:39*/
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa*/
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
			/*Isola o valor dentro de M(), e analisa, podendo haver 8:19 ou 28:39*/
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
	else {
		if (!strcmp(op, "ldmq")) {
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
			
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "lsh")) {
			if (tracker[1] == 0)
				tracker[1] = 1;
			else {
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
		}
		else if (!strcmp(op, "rsh")) {
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
	int val;
	
	if (arquivoSaida == NULL) {
		if (!strcmp(dir, ".word")) {
			if (tracker[1] == 1)
				erroSintaxe();
			else {
				token = strtok(NULL, " \n");
				token = convertToLower(token);
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
				token = convertToLower(token);
				tipo = confereNumeroNome(token);
				/*Trata o primeiro argumento de wfill e coloca em n*/
				token = strtok(NULL, " \n");
				token = convertToLower(token);
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
			token = convertToLower(token);
			tipo = confereNumeroNome(token);
			/*Trata o numero e coloca em n*/
			tracker[0] = n;
			tracker[1] = 0;
		}
		else if(!strcmp(dir, ".align")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
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
			/*Trata o numero e coloca em n*/
			armazenaConstante(nomeConst, n);
		}
		else
			erroSintaxe();
	}
	else {
		if (!strcmp(dir, ".word")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereNumeroNome(token);
			if (tipo == 0) {
				/*Busca nas listas de nomes e devolve o valor em val*/
			}
			else {
				/*Converte para int e coloca em val*/
			}
			/*Imprime val no arquivo*/
			tracker[0] = tracker[0]+1;
		}
		else if(!strcmp(dir, ".wfill")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereNumeroNome(token);
			/*Converte para int e coloca em n*/
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereNumeroNome(token);
			if (tipo == 0) {
				/*Busca nas listas de nomes e devolve o valor em val*/
			}
			else {
				/*Converte para int e coloca em val*/
			}
			for (i = 0; i < n; i++) {
				/*Imprime val no arquivo*/
				tracker[0] = tracker[0]+1;
			}
		}
		else if(!strcmp(dir, ".org")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereNumeroNome(token);
			/*Converte o numero e coloca em n*/
			tracker[0] = n;
			tracker[1] = 0;
		}
		else if(!strcmp(dir, ".align")) {
			token = strtok(NULL, " \n");
			token = convertToLower(token);
			tipo = confereNumeroNome(token);
			/*Converte o numero e coloca em n*/
			if (tracker[1] == 1) {
				/*Imprime 0s*/
				tracker[0] = tracker[0]+1;
				tracker[1] = 0;
			}
			while (tracker[0]%n) {
				/*Imprime 0s*/
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
		}
	}
	
	if (comecoPend != NULL)
		erroUsoNome();
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
	
	fclose(arquivoEntrada);
	fclose(arquivoSaida);
	return 0;
}
