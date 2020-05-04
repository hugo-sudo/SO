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
#include <signal.h>
#include <limits.h>

#include "main.h"
#include "so.h"
#include "prodcons.h"
#include "control.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
//==============================================

struct prodcons ProdCons;

//******************************************
// SEMAFORO_CRIAR
//
sem_t * semaphore_create(char * name, int value) {
	//==============================================
	// FUNÇÃO GENÉRICA DE CRIAÇÃO DE UM SEMÁFORO
	//
	int shm_unlink = sem_unlink(name);
	sem_t *sem;

	if(shm_unlink == -1){
		perror(name);
	}
	sem = sem_open(name, O_CREAT | O_EXCL, 0xFFFFFFFF, value);

	if (sem == SEM_FAILED) {
		perror(name);
		exit(1);
	}
	return sem;
	// return so_semaphore_create(name, value);
	//==============================================
}


void prodcons_create_capacidade_servico() {
	//==============================================
	// CRIAR MUTEX PARA CONTROLAR O ACESSO A CAPACIDADE_SERVICOO
	//
	// utilizar a função genérica semaphore_create
	ProdCons.stock_mutex = semaphore_create(STR_SEM_CAPACIDADE_SERVICO_MUTEX, 1);

	// so_prodcons_create_capacidade_servicos();
	//==============================================
}

void prodcons_create_buffers() {
	//==============================================
	// CRIAR SEMAFOROS PARA CONTROLAR O ACESSO AOS 3 BUFFERS
	//
	// utilizar a função genérica semaphore_create
	ProdCons.response_p_full = semaphore_create(STR_SEM_PROPOSTA_FULL, 0);
	ProdCons.response_p_empty = semaphore_create(STR_SEM_PROPOSTA_EMPTY, Config.BUFFER_PROPOSTA);
	ProdCons.response_p_mutex = semaphore_create(STR_SEM_PROPOSTA_MUTEX, 1);
	ProdCons.request_b_full= semaphore_create(STR_SEM_ORCAMENTO_FULL, 0);
	ProdCons.request_b_empty = semaphore_create(STR_SEM_ORCAMENTO_EMPTY, Config.BUFFER_ORCAMENTO);
	ProdCons.request_b_mutex = semaphore_create(STR_SEM_ORCAMENTO_MUTEX, 1);
	ProdCons.request_d_full = semaphore_create(STR_SEM_DESCRICAO_FULL, 0);
	ProdCons.request_d_empty = semaphore_create(STR_SEM_DESCRICAO_EMPTY, Config.BUFFER_DESCRICAO);
	ProdCons.request_d_mutex = semaphore_create(STR_SEM_DESCRICAO_MUTEX, 1);
	// so_prodcons_create_buffers();
	//==============================================
}

void semaphore_destroy(char * name, void * ptr) {
	//==============================================
	// FUNÇÃO GENÉRICA DE DESTRUIÇÃO DE UM SEMÁFORO
	//


	int shm_close = sem_close(ptr);

	if(shm_close == -1){
		perror(name);	
	}
	int shm_unlink = sem_unlink(name);
	if (shm_unlink == -1) {
		perror(name);
	}	
}
	//==============================================
void prodcons_destroy() {
	//==============================================
	// DESTRUIR SEMÁFORO E RESPETIVO NOME
	//
	// utilizar a função genérica semaphore_destroy
	semaphore_destroy(STR_SEM_DESCRICAO_FULL, ProdCons.request_d_full);
	semaphore_destroy(STR_SEM_DESCRICAO_EMPTY, ProdCons.request_d_empty);
	semaphore_destroy(STR_SEM_DESCRICAO_MUTEX, ProdCons.request_d_mutex);

	semaphore_destroy(STR_SEM_ORCAMENTO_FULL, ProdCons.request_b_full);
	semaphore_destroy(STR_SEM_ORCAMENTO_EMPTY, ProdCons.request_b_empty);
	semaphore_destroy(STR_SEM_ORCAMENTO_MUTEX, ProdCons.request_b_mutex);
	
	semaphore_destroy(STR_SEM_PROPOSTA_FULL, ProdCons.response_p_full);
	semaphore_destroy(STR_SEM_PROPOSTA_EMPTY, ProdCons.response_p_empty);
	semaphore_destroy(STR_SEM_PROPOSTA_MUTEX, ProdCons.response_p_mutex);
	
	semaphore_destroy(STR_SEM_CAPACIDADE_SERVICO_MUTEX, ProdCons.stock_mutex);
	// so_prodcons_destroy();
	//==============================================
}

//******************************************
void prodcons_request_d_produce_begin() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER DESCRICAO
	// por fazer
	int sem_request_d_empty = sem_wait(ProdCons.request_d_empty);
	int sem_request_d_mutex = sem_wait(ProdCons.request_d_mutex);

	if(sem_request_d_empty == -1){
		perror("sem_request_d_empty");
		exit(2);
	}
	
	if(sem_request_d_mutex == -1){
		perror("sem_request_d_mutex");
		exit(3);
	}
	
	// so_prodcons_request_d_produce_begin();
	//==============================================
}

//******************************************
void prodcons_request_d_produce_end() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER DESCRICAO
	//
	int sem_request_d_mutex = sem_post(ProdCons.request_d_mutex);
	int sem_request_d_full = sem_post(ProdCons.request_d_full);

	if(sem_request_d_mutex == -1){
		perror("sem_request_d_mutex");
		exit(4);
	}
	if(sem_request_d_full == -1){
		perror("sem_request_d_full");
		exit(5);
	}
	// so_prodcons_request_d_produce_end();
	//==============================================
}

//******************************************
void prodcons_request_d_consume_begin() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER DESCRICAO
	//
	int sem_request_d_full = sem_wait(ProdCons.request_d_full);
	int sem_request_d_mutex = sem_wait(ProdCons.request_d_mutex);

	if(sem_request_d_full == -1){
		perror("sem_descricao_d_full");
		exit(6);
	}
	if(sem_request_d_mutex == -1){
		perror("sem_descricao_d_mutex");
		exit(7);
	}
	// so_prodcons_request_d_consume_begin();
	//==============================================
}

//******************************************
void prodcons_request_d_consume_end() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER DESCRICAO
	//
	int sem_request_d_mutex = sem_post(ProdCons.request_d_mutex);
	int sem_request_d_empty = sem_post(ProdCons.request_d_empty);
	

	if(sem_request_d_mutex == -1){
		perror("sem_descricao_d_mutex");
		exit(8);
	}
	if(sem_request_d_empty == -1){
		perror("sem_descricao_d_empty");
		exit(9);
	}
	// so_prodcons_request_d_consume_end();
	//==============================================
}

//******************************************
void prodcons_request_b_produce_begin() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER ORCAMENTO
	//
	int sem_request_b_empty = sem_wait(ProdCons.request_b_empty);
	int sem_request_b_mutex = sem_wait(ProdCons.request_b_mutex);
	if(sem_request_b_empty == -1){
		perror("sem_request_b_empty");
		exit(10);
	}
	if(sem_request_b_mutex == -1){
		perror("sem_request_b_mutex");
		exit(11);
        }
	// so_prodcons_request_b_produce_begin();
	//==============================================
}

//******************************************
void prodcons_request_b_produce_end() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER ORCAMENTO
	//
	int sem_request_b_full = sem_post(ProdCons.request_b_full);
	int sem_request_b_mutex = sem_post(ProdCons.request_b_mutex);

	if(sem_request_b_full == -1){
		perror("sem_request_b_full");
		exit(12);
	}
	if(sem_request_b_mutex == -1){
		perror("sem_request_b_mutex");
		exit(13);
        }
	// so_prodcons_request_b_produce_end();
	//==============================================
}

//******************************************
void prodcons_request_b_consume_begin() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER ORCAMENTO
	//
	int sem_request_b_full = sem_wait(ProdCons.request_b_full);
	int sem_request_b_mutex = sem_wait(ProdCons.request_b_mutex);
	
	if(sem_request_b_full == -1){
		perror("sem_request_b_empty");
		exit(14);
	}
	if(sem_request_b_mutex == -1){
		perror("sem_request_b_mutex");
		exit(15);
        }
	// so_prodcons_request_b_consume_begin();
	//==============================================
}

//******************************************
void prodcons_request_b_consume_end() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER ORCAMENTO
	//
	int sem_request_b_mutex = sem_post(ProdCons.request_b_mutex);
	int sem_request_b_empty = sem_post(ProdCons.request_b_empty);

	if(sem_request_b_mutex == -1){
		perror("sem_request_b_mutex");
		exit(16);
        }
	if(sem_request_b_empty == -1){
		perror("sem_request_b_empty");
		exit(17);
        }
	
	
	// so_prodcons_request_b_consume_end();
	//==============================================
}

//******************************************
void prodcons_response_p_produce_begin() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER PROPOSTA
	//
	int sem_response_p_empty = sem_wait(ProdCons.response_p_empty);
	int sem_response_p_mutex = sem_wait(ProdCons.response_p_mutex);
	
	if(sem_response_p_empty == -1){
		perror("sem_response_p_empty");
		exit(18);
	}
	if(sem_response_p_mutex == -1){
		perror("sem_response_p_mutex");
		exit(19);
	}
	// so_prodcons_response_p_produce_begin();
	//==============================================
}

//******************************************
void prodcons_response_p_produce_end() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER PROPOSTA
	//
	int sem_response_p_full = sem_post(ProdCons.response_p_full);
	int sem_response_p_mutex = sem_post(ProdCons.response_p_mutex);
	
	if(sem_response_p_full == -1){
		perror("sem_response_p_full");
		exit(20);
	}
	if(sem_response_p_mutex == -1){
		perror("sem_response_p_mutex");
		exit(21);
	}
	// so_prodcons_response_p_produce_end();
	//==============================================
}

//******************************************
void prodcons_response_p_consume_begin() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER PROPOSTA
	//
	int sem_response_p_full = sem_wait(ProdCons.response_p_full);
	int sem_response_p_mutex = sem_wait(ProdCons.response_p_mutex);
	
	if(sem_response_p_full == -1){
		perror("sem_response_p_full");
		exit(22);
	}
	if(sem_response_p_mutex == -1){
		perror("sem_response_p_mutex");
		exit(23);
	}
	// so_prodcons_response_p_consume_begin();
	//==============================================
}

//******************************************
void prodcons_response_p_consume_end() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER PROPOSTA
	//
	int sem_response_p_mutex = sem_post(ProdCons.response_p_mutex);
	int sem_response_p_empty = sem_post(ProdCons.response_p_empty);
	
	if(sem_response_p_mutex == -1){
		perror("sem_response_p_mutex");
		exit(24);
	}
	if(sem_response_p_empty == -1){
		perror("sem_response_p_empty");
		exit(25);
	}

	// so_prodcons_response_p_consume_end();
	//==============================================
}

//******************************************
void prodcons_buffers_begin() {
	//==============================================
	// GARANTIR EXCLUSÃO MÚTUA NO ACESSO AOS 3 BUFFERS
	//
	int sem_request_d_mutex = sem_wait(ProdCons.request_d_mutex);
	int sem_request_b_mutex = sem_wait(ProdCons.request_b_mutex);
	int sem_response_p_mutex = sem_wait(ProdCons.response_p_mutex);

	if(sem_request_d_mutex == -1){
		perror("sem_request_d_mutex");
		exit(26);
	}
	if(sem_request_b_mutex == -1){
		perror("sem_request_b_mutex");
		exit(27);
	}
	if(sem_response_p_mutex == -1){
		perror("sem_response_p_mutex");
		exit(28);
	}
	// so_prodcons_buffers_begin();
	//==============================================
}

//******************************************
void prodcons_buffers_end() {
	//==============================================
	// FIM DA ZONA DE EXCLUSÃO MÚTUA NO ACESSO AOS 3 BUFFERS
	//
	int sem_request_d_mutex = sem_post(ProdCons.request_d_mutex);
	int sem_request_b_mutex = sem_post(ProdCons.request_b_mutex);
	int sem_response_p_mutex = sem_post(ProdCons.response_p_mutex);

	if(sem_request_d_mutex == -1){
		perror("sem_request_d_mutex");
		exit(29);
	}
	if(sem_request_b_mutex == -1){
		perror("sem_request_b_mutex");
		exit(30);
	}
	if(sem_response_p_mutex == -1){
		perror("sem_response_p_mutex");
		exit(31);
	}
	// so_prodcons_buffers_end();
	//==============================================
}

//******************************************
int prodcons_update_capacidade_servico(int servico) {
	//==============================================
	// OBTER MUTEX DA CAPACIDADE SERVICO E ATUALIZAR CAPACIDADE SERVICO
	//
	// se capacidade_servicos do servico > 0 então decrementá-lo de uma unidade e
	//   função devolve 1
	// se capacidade_servicos do servico = 0 então função devolve 0

	
	int capacidadeServ = 0;

	sem_wait(ProdCons.stock_mutex); // begin

	if (Config.capacidade_servico[servico] > 0) {
		Config.capacidade_servico[servico]--;
		capacidadeServ = 1;
	}
	sem_post(ProdCons.stock_mutex); //end

	return capacidadeServ;
	// return so_prodcons_update_capacidade_servico(servico);
	//==============================================
}

