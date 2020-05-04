#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h> //mmap
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <ctype.h>
#include <signal.h>
#include <limits.h>

#include "main.h"
#include "so.h"
#include "memory.h"
#include "prodcons.h"
#include "file.h"
#include "sotime.h"
#include "scheduler.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
//==============================================

struct file Ficheiros; // informação sobre nomes e handles de ficheiros

void file_begin(char *fic_in, char *fic_out, char *fic_log) {
	//==============================================
	// GUARDAR NOMES DOS FICHEIROS NA ESTRUTURA Ficheiros
	//
	if(fic_in!= NULL){
		Ficheiros.entrada = strdup(fic_in);
	}
	if(fic_out != NULL){
		Ficheiros.saida = strdup(fic_out);
	}
	if(fic_log!= NULL){
		Ficheiros.log = strdup(fic_log);
	}
	// so_file_begin(fic_in, fic_out, fic_log);
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE ENTRADA
	// em modo de leitura
	if(Ficheiros.entrada != NULL){
		Ficheiros.h_in = fopen(Ficheiros.entrada,"r");
	}
	if(Ficheiros.h_in == NULL){
		perror("Ficheiro de entrada nao existe");
		exit(1);
	}
	// so_file_open_file_in();
	//==============================================

	// parse do ficheiro de teste
	// esta funcao prepara os campos da estrutura Config (char *)
	// com os dados do ficheiro de entrada
	if (ini_parse_file(Ficheiros.h_in, handler, &Config) < 0) {
		printf("Erro a carregar o ficheiro de teste'\n");
		exit(1);
	}

	// agora e' preciso inicializar os restantes campos da estrutura Config

	//==============================================
	// CONTAR SERVICOS
	// usar strtok para percorrer Config.list_servicos
	// guardar resultado em Config.SERVICOS
	char* listaServicos = strdup(Config.list_servicos);
	int countServicos = 0;
	char* tokenServ = strtok(Config.list_servicos," ");
	while(tokenServ != NULL){
		tokenServ = strtok(NULL," ");
		countServicos++;
	}
	Config.SERVICOS = countServicos;
	// so_file_count_servicos();
	//==============================================

	// iniciar memoria para o vetor com o capacidade por servico e semaforo
	memory_create_capacidade_servicos();
	prodcons_create_capacidade_servico();

	//==============================================
	// LER CAPACIDADE DE CADA SERVICO
	// percorrer Config.list_servicos e
	// guardar cada valor no vetor Config.capacidade_servicos
	int i = 0;
	char* tokenCapServ = strtok(listaServicos, " ");

	while(tokenCapServ != NULL){
	    Config.capacidade_servico[i] = atoi(tokenCapServ);
	    tokenCapServ = strtok(NULL, " ");
	    i++;
	}

	// so_file_read_capacidade_servicos();
	//==============================================

	//==============================================
	// CONTAR CLIENTES
	// usar strtok para percorrer Config.list_clientes
	// guardar resultado em Config.CLIENTES
	int countClientes = 0;
    char* tokenClientes = strtok(Config.list_clientes," ");
	while(tokenClientes != NULL){
		tokenClientes = strtok(NULL," ");
		countClientes++;
	}
	Config.CLIENTES = countClientes;
	// so_file_count_clientes();
	//==============================================

	//==============================================
	// CONTAR INTERMEDIARIOS
	// usar strtok para percorrer Config.list_intermediarios
	// guardar resultado em Config.INTERMEDIARIOS
	int countIntermediarios = 0; 
	char* tokenIntermediarios = strtok(Config.list_intermediarios, " ");
	while(tokenIntermediarios != NULL){
		tokenIntermediarios = strtok(NULL," ");
		countIntermediarios++;
	}
	Config.INTERMEDIARIO = countIntermediarios;
	// so_file_count_intermediarios();
	//==============================================

	//==============================================
	// CONTAR EMPRESAS
	// usar strtok para percorrer Config.list_empresas
	// guardar resultado em Config.EMPRESAS
	int countEmpresas = 0;
	char* tokenEmpresas = strtok(Config.list_empresas, ",");
	while(tokenEmpresas !=NULL){
		tokenEmpresas = strtok(NULL,",");
		countEmpresas++;
	}
	Config.EMPRESAS = countEmpresas;
	// so_file_count_empresas();
	//==============================================

	so_file_read_servicos();

	//==============================================
	// LER CAPACIDADES DOS BUFFERS
	// usar strtok para percorrer Config.list_buffers
	// guardar primeiro tamanho em Config.BUFFER_DESCRICAO
	// guardar segundo tamanho em Config.BUFFER_ORCAMENTO
	// guardar terceiro tamanho em Config.BUFFER_PROPOSTA
	Config.BUFFER_DESCRICAO = atoi(strtok(Config.list_buffers, " "));
	Config.BUFFER_ORCAMENTO = atoi(strtok(NULL, " "));
	Config.BUFFER_PROPOSTA = atoi(strtok(NULL, " "));
	// so_file_read_capacidade_buffer();
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE SAIDA (se foi especificado)
	// em modo de escrita
	if (Ficheiros.saida != NULL){
		Ficheiros.h_out = fopen(Ficheiros.saida,"w");
	} 
	if (Ficheiros.h_out == NULL){
		perror("ficheiro de saida nao existe");
		exit(1);
	}
	// so_file_open_file_out();
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE LOG (se foi especificado)
	// em modo de escrita
	if (Ficheiros.log != NULL){
		Ficheiros.h_log = fopen(Ficheiros.log,"w");
	}
	if (Ficheiros.h_log == NULL){
		perror("ficheiro de log nao existe");
		exit(1);
	}
	// so_file_open_file_log();
	//==============================================
}
void file_destroy() {
	//==============================================
	// LIBERTAR ZONAS DE MEMÓRIA RESERVADAS DINAMICAMENTE
	// que podem ser: Ficheiros.entrada, Ficheiros.saida, Ficheiros.log
	free(Ficheiros.entrada);
	free(Ficheiros.saida);
	free(Ficheiros.log);
	// so_file_destroy();
	//==============================================
}

void file_write_log_file(int etapa, int id) {
	double t_diff;

	if (Ficheiros.h_log != NULL) {

		prodcons_buffers_begin();

		// guardar timestamp
		t_diff = time_untilnow();

		//==============================================
		// ESCREVER DADOS NO FICHEIRO DE LOG
		//
		// o log deve seguir escrupulosamente o formato definido
		
		
		
		
		so_file_write_log_file(etapa, id, t_diff);
		//==============================================

		prodcons_buffers_end();
	}
}

void file_write_line(char * linha) {
	//==============================================
	// ESCREVER UMA LINHA NO FICHEIRO DE SAIDA
	//
	if (Ficheiros.h_out == NULL){
		perror("Sem acesso a ficheiro_out_linha");
		exit(1);
	}

	fputs(linha, Ficheiros.h_out);

	// so_file_write_line(linha);
	//==============================================
}

int stricmp(const char *s1, const char *s2) {
	if (s1 == NULL)
		return s2 == NULL ? 0 : -(*s2);
	if (s2 == NULL)
		return *s1;

	char c1, c2;
	while ((c1 = tolower(*s1)) == (c2 = tolower(*s2))) {
		if (*s1 == '\0')
			break;
		++s1;
		++s2;
	}

	return c1 - c2;
}

int handler(void* user, const char* section, const char* name,
		const char* value) {
	struct configuration* pconfig = (struct configuration*) user;

#define MATCH(s, n) stricmp(section, s) == 0 && stricmp(name, n) == 0
	if (MATCH("servicos", "capacidade_servicos")) {
		pconfig->list_servicos = strdup(value);
	} else if (MATCH("clientes", "servico")) {
		pconfig->list_clientes = strdup(value);
	} else if (MATCH("intermediarios", "list")) {
		pconfig->list_intermediarios = strdup(value);
	} else if (MATCH("empresas", "servicos")) {
		pconfig->list_empresas = strdup(value);
	} else if (MATCH("buffers", "capacidade_buffer")) {
		pconfig->list_buffers = strdup(value);
	} else {
		return 0; /* unknown section/name, error */
	}
	return 1;
}

