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
#include "memory.h"
#include "prodcons.h"
#include "control.h"
#include "file.h"
#include "sotime.h"
#include "scheduler.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
extern struct scheduler Schedule;
extern struct service Service;
extern struct statistics Ind;
//==============================================

struct response_p BProposta;  	// buffer empresa-cliente
struct request_b BOrcamento; 	// buffer intermediario-empresa
struct request_d BDescricao; 	// buffer cliente-intermediario

//******************************************
// CRIAR ZONAS DE MEMORIA
//
void * memory_create(char * name, int size) {
	//==============================================
	// FUNÇÃO GENÉRICA DE CRIAÇÃO DE MEMÓRIA PARTILHADA
	//
	// usar getuid() para gerar nome unico na forma:
	// sprintf(name_uid,"/%s_%d", name, getuid())
	// usar name_uid em shm_open
	// usar tambem: ftruncate e mmap
	int *ptr;
	int ret;
	char *name_uid;
	sprintf(name_uid,"/%s_%d", name, getuid());
	int fd = shm_open(name_uid, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1){
		perror(name_uid);
		exit(1);
	}
	ret = ftruncate(fd, size);
	if (ret == -1){
		perror(name_uid);
		exit(2);
	}
	ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED){
		perror("shm-mmap");
		exit(3);
	}

	so_memory_create(name, size);
	//==============================================
}
void memory_create_capacidade_servicos() {
	//==============================================
	// CRIAR ZONA DE MEMÓRIA PARA A CAPACIDADE DE SERVICOS
	//
	// utilizar a função genérica memory_create(char *,int)
	// para criar ponteiro que se guarda em Config.capacidade_servicos
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [SERVICOS] para inteiro
	Config.capacidade_servico = memory_create(STR_SHM_STOCK, sizeof(int) * Config.SERVICOS );
}	


	
	// logso_memory_create_capacidade_servicos();
	
	// log//==============================================

	// log memory_create_buffers() {
	
	// log//==============================================
	
	// log// CRIAR ZONAS DE MEMÓRIA PARA OS BUFFERS: DESCRICAO, ORCAMENTO e PROPOSTAS
	
	// log//
void memory_create_buffers(){
	// log// utilizar a função genérica memory_create(char *,int)
	//
	// para criar ponteiro que se guarda em BProposta.ptr
	// que deve ter capacidade para um vetor unidimensional
	// de inteiros com a dimensao [BUFFER_PROPOSTA]
	// para criar ponteiro que se guarda em BProposta.buffer
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [BUFFER_PROPOSTA] para struct service
	//
	// para criar ponteiro que se guarda em BOrcamento.ptr
	// que deve ter capacidade para um vetor unidimensional
	// de inteiros com a dimensao [BUFFER_ORCAMENTO]
	// para criar ponteiro que se guarda em BOrcamento.buffer
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [BUFFER_ORCAMENTO] para struct service
	//
	// para criar ponteiro que se guarda em BDescricao.ptr
	// que deve ter capacidade para struct pointers
	// para criar ponteiro que se guarda em BDescricao.buffer
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [BUFFER_DESCRICAO] para struct service
	BProposta.ptr = memory_create(STR_SHM_RECEIPT_PTR , sizeof(int) * Config.BUFFER_PROPOSTA);
	BProposta.buffer = memory_create(STR_SHM_RECEIPT_BUFFER, sizeof(int) * Config.BUFFER_PROPOSTA);
	BOrcamento.ptr = memory_create(STR_SHM_CURRENCY_PTR, sizeof(int) * Config.BUFFER_ORCAMENTO);
	BOrcamento.buffer = memory_create(STR_SHM_CURRENCY_BUFFER, sizeof(int) * Config.BUFFER_ORCAMENTO);
	BDescricao.ptr = memory_create(STR_SHM_ORDER_PTR, sizeof(struct pointers));
	BDescricao.buffer = memory_create(STR_SHM_ORDER_BUFFER, sizeof(int)*Config.BUFFER_DESCRICAO);
}	

	// so_memory_create_buffers();
	//==============================================

void memory_create_scheduler() {
	//==============================================
	// CRIAR ZONA DE MEMÓRIA PARA O MAPA DE ESCALONAMENTO
	//
	// utilizar a função genérica memory_create(char *,int)
	// para criar ponteiro que se guarda em Schedule.ptr
	// que deve ter capacidade para um vetor bidimensional
	// com a dimensao [SERVICOS,EMPRESAS] para inteiro
	Schedule.ptr = memory_create(STR_SHM_SCHEDULER, sizeof(int)*Config.SERVICOS * sizeof(int)*Config.EMPRESAS);
	so_memory_create_scheduler();
	//==============================================
}

void memory_destroy(char * name, void * ptr, int size) {
	//==============================================
	// FUNÇÃO GENÉRICA DE DESTRUIÇÃO DE MEMÓRIA PARTILHADA
	//
	// usar getuid() para gerar nome unico na forma:
	// sprintf(name_uid,"/%s_%d", name, getuid())
	// usar name_uid em shm_unlink
	// usar tambem: munmap
	int ret;
	char *name_uid;
	sprintf(name_uid,"/%s_%d", name, getuid());
	ret = munmap(ptr, size);
	if (ret == -1){
		perror("/shm");
		exit(7);
	}
	ret = shm_unlink(name_uid);
	if (ret == -1){
		perror(name_uid);
		exit(8);
	}
	



	so_memory_destroy(name, ptr, size);
	//==============================================
}

//******************************************
// MEMORIA_DESTRUIR
//
void memory_destroy_all() {
	//==============================================
	// DESTRUIR MAPEAMENTO E NOME DE PÁGINAS DE MEMÓRIA
	//
	// utilizar a função genérica memory_destroy(char *,void *,int)

	memory_destroy(STR_SHM_SCHEDULER, Schedule.ptr, sizeof(int)*Config.SERVICOS * sizeof(int)*Config.EMPRESAS);
	memory_destroy(STR_SHM_STOCK, Config.capacidade_servico ,sizeof(int) * Config.SERVICOS );
	
	memory_destroy(STR_SHM_RECEIPT_PTR, BProposta.ptr, sizeof(int) * Config.BUFFER_PROPOSTA);
	memory_destroy(STR_SHM_RECEIPT_BUFFER,BProposta.buffer, sizeof(int) * Config.BUFFER_PROPOSTA);
	memory_destroy(STR_SHM_CURRENCY_PTR, BOrcamento.ptr, sizeof(int) * Config.BUFFER_ORCAMENTO);
	memory_destroy(STR_SHM_CURRENCY_BUFFER, BOrcamento.buffer, sizeof(int) * Config.BUFFER_ORCAMENTO);
	memory_destroy(STR_SHM_ORDER_PTR, BDescricao.ptr, sizeof(struct pointers));
	memory_destroy(STR_SHM_ORDER_BUFFER, BDescricao.buffer,sizeof(int)*Config.BUFFER_DESCRICAO);

	free(Ind.capacidade_inicial_servicos);
	free(Ind.clientes_servidos_por_intermediarios);
	free(Ind.servicos_recebidos_por_clientes);
	free(Ind.clientes_servidos_por_empresas);


	// so_memory_destroy_all();
	//==============================================
}

//******************************************
// MEMORIA_REQUEST_D_ESCREVE
//
void memory_request_d_write(int id, struct service *pProduto) {
	prodcons_request_d_produce_begin();

	// registar hora do pedido de servico
	time_register(&pProduto->time_descricao);

	//==============================================
	// ESCREVER DESCRICAO DE PROJETO PESSOAL NO BUFFER DESCRICOES
	//
	// copiar conteudo relevante da estrutura pProduto para
	// a posicao BDescricoes.ptr->in do buffer BDescricoes
	// conteudo: cliente, id, time_request
	// colocar disponibilidade = 1 nessa posicao do BDescricoes
	// e atualizar BDescricoes.ptr->in


	int pos = BDescricao.ptr->in;
	
	BDescricao.buffer[pos].id = pProduto->id;
	BDescricao.buffer[pos].cliente = pProduto->cliente;
	BDescricao.buffer[pos].time_descricao = pProduto->time_descricao;
	BDescricao.buffer[pos].disponibilidade = 1;


	
	// so_memory_request_d_write(id, pProduto);
	
	//==============================================

	prodcons_request_d_produce_end();

	// informar INTERMEDIARIO de pedido de SERVICO
	control_cliente_submete_descricao(id);

	// log
	file_write_log_file(1, id);
}
//******************************************
// MEMORIA_REQUEST_D_LE
//
int memory_request_d_read(int id, struct service *pProduto) {
	// testar se existem investors e se o SO_PERPRO esta open
	if (control_intermediario_esperapor_descricao(id) == 0)
		return 0;

	prodcons_request_d_consume_begin();

	//==============================================
	// LER DESCRICAO DO BUFFER DE DESCRICOES
	//
	// copiar conteudo relevante da posicao BDescricao.ptr->out
	// do buffer BDescricao para a estrutura pProduto
	// conteudo: cliente, id, time_request
	// colocar available = 0 nessa posicao do BDescricao
	// e atualizar BDescricao.ptr->out

	int pos = BDescricao.ptr->out;
	pProduto->cliente = BDescricao.buffer[pos].cliente;
	pProduto->id = BDescricao.buffer[pos].id;
	pProduto->time_descricao = BDescricao.buffer[pos].time_descricao;
	BDescricao.buffer[pos].disponibilidade = 0;
	// so_memory_request_d_read(id, pProduto);
	//==============================================

	// testar se existe capacidade de servico para servir cliente
	if (prodcons_update_capacidade_servico(pProduto->id) == 0) {
		pProduto->disponibilidade = 0;
		prodcons_request_d_consume_end();
		return 2;
	} else
		pProduto->disponibilidade = 1;

	prodcons_request_d_consume_end();

	// log
	file_write_log_file(2, id);

	return 1;
}

//******************************************
// MEMORIA_REQUEST_B_ESCREVE
//
void memory_request_b_write(int id, struct service *pProduto) {
	int pos, exchange;

	prodcons_request_b_produce_begin();

	// decidir a que empresa se destina
	exchange = scheduler_get_empresa(id, pProduto->id);

	//==============================================
	// ESCREVER ORCAMENTO NO BUFFER DE PEDIDOS DE ORCAMENTOS
	//
	// procurar posicao vazia no buffer BOrcamento
	// copiar conteudo relevante da estrutura pProduto para
	// a posicao BOrcamento.ptr do buffer BOrcamento
	// conteudo: cliente, id, disponibilidade, intermediario, empresa, time_request
	// colocar BOrcamento.ptr[n] = 1 na posicao respetiva

	pos = 0;
	while (BOrcamento.ptr[pos] != 0 || pos<Config.BUFFER_ORCAMENTO){
		pos++;
	}
	


	
	BOrcamento.buffer[pos].cliente =  pProduto->cliente;
	BOrcamento.buffer[pos].id = pProduto->id;
	BOrcamento.buffer[pos].disponibilidade = pProduto->disponibilidade;
	BOrcamento.buffer[pos].intermediario = pProduto->intermediario;
	BOrcamento.buffer[pos].empresa = pProduto->empresa;
	BOrcamento.buffer[pos].time_descricao = pProduto->time_descricao;
	BOrcamento.ptr[pos] = 1;



	pos = so_memory_request_b_write(id, pProduto, exchange);
	//==============================================

	prodcons_request_b_produce_end();

	// informar empresa respetiva de pedido de orcamento
	control_intermediario_submete_orcamento(exchange);

	// registar hora pedido (orcamento)
	time_register(&BProposta.buffer[pos].time_orcamento);

	// log
	file_write_log_file(3, id);
}
//******************************************
// MEMORIA_REQUEST_B_LE
//
int memory_request_b_read(int id, struct service *pProduto) {
	// testar se existem pedidos e se o SO_PerPro esta open
	if (control_empresa_esperapor_orcamento(id) == 0)
		return 0;

	prodcons_request_b_consume_begin();

	//==============================================
	// LER PEDIDO DO BUFFER DE PEDIDOS DE ORCAMENTOS
	//
	// procurar pedido de orcamento para a empresa no buffer BOrcamento
	// copiar conteudo relevante da posicao encontrada
	// no buffer BOrcamento para pProduto
	// conteudo: cliente, id, disponibilidade, intermediario, time_request, time_order
	// colocar BOrcamento.ptr[n] = 0 na posicao respetiva
	int pos = 0;
	while (BOrcamento.ptr[pos] != 1 || pos<Config.BUFFER_ORCAMENTO){
		pos++;
	}
	pProduto->cliente = BOrcamento.buffer[pos].cliente;
	pProduto->id = BOrcamento.buffer[pos].id;
	pProduto->disponibilidade = BOrcamento.buffer[pos].disponibilidade;
	pProduto->intermediario = BOrcamento.buffer[pos].intermediario;
	pProduto->empresa = BOrcamento.buffer[pos].empresa;
	pProduto->time_descricao = BOrcamento.buffer[pos].time_descricao;
	BOrcamento.ptr[pos] = 0;

	


	// so_memory_request_b_read(id, pProduto);
	//==============================================

	prodcons_request_b_consume_end();

	// log
	file_write_log_file(4, id);

	return 1;
}

//******************************************
// MEMORIA_RESPONSE_P_ESCREVE
//
void memory_response_p_write(int id, struct service *pProduto) {
	int pos;

	prodcons_response_p_produce_begin();

	//==============================================
	// ESCREVER PROPOSTA NO BUFFER DE PROPOSTAS
	//
	// procurar posicao vazia no buffer BProposta
	// copiar conteudo relevante da estrutura pProduto para
	// a posicao BProposta.ptr do buffer BProposta
	// conteudo: cliente, id, disponibilidade, intermediario, empresa, time_request, time_order
	// colocar BProposta.ptr[n] = 1 na posicao respetiva

	pos = 0;
	while (BProposta.ptr[pos] != 0 || pos<Config.BUFFER_PROPOSTA){
		pos++;
	}
	BProposta.buffer[pos].cliente =pProduto->cliente;
	BProposta.buffer[pos].id = pProduto->id;
	BProposta.buffer[pos].disponibilidade = pProduto->disponibilidade;
	BProposta.buffer[pos].intermediario = pProduto->intermediario;
	BProposta.buffer[pos].empresa = pProduto->empresa;
	BProposta.buffer[pos].time_descricao = pProduto->time_descricao;
	BProposta.buffer[pos].time_orcamento = pProduto->time_orcamento;
	BProposta.ptr[pos] = 1; 
	
	pos = so_memory_response_p_write(id, pProduto);
	//==============================================

	prodcons_response_p_produce_end();

	// informar cliente de que a proposta esta pronta
	control_empresa_submete_proposta(pProduto->cliente);

	// registar hora pronta (proposta)
	time_register(&BProposta.buffer[pos].time_proposta);

	// log
	file_write_log_file(5, id);
}
//******************************************
// MEMORIA_RESPONSE_P_LE
//
void memory_response_p_read(int id, struct service *pProduto) {
	// aguardar proposta
	control_cliente_esperapor_proposta(pProduto->cliente);

	prodcons_response_p_consume_begin();

	//==============================================
	// LER PROPSOTA DO BUFFER DE PROPOSTAS
	//
	// procurar proposta para o cliente no buffer BProposta
	// copiar conteudo relevante da posicao encontrada
	// no buffer BProposta para pProduto
	// conteudo: cliente, disponibilidade, intermediario, empresa, time_request, time_order, time_receipt
	// colocar BProposta.ptr[n] = 0 na posicao respetiva

	int pos = 0;
	while (BProposta.ptr[pos] != 1 || pos<Config.BUFFER_PROPOSTA){
		pos++;
	}
	pProduto->cliente = BProposta.buffer[pos].cliente;
	pProduto->id = BProposta.buffer[pos].id;
	pProduto->disponibilidade = BProposta.buffer[pos].disponibilidade;
	pProduto->intermediario = BProposta.buffer[pos].intermediario;
	pProduto->empresa = BProposta.buffer[pos].empresa;
	pProduto->time_descricao = BProposta.buffer[pos].time_descricao;
	pProduto->time_orcamento = BProposta.buffer[pos].time_orcamento; 
	BProposta.ptr[pos] = 0; 
	// so_memory_response_p_read(id, pProduto);
	//==============================================

	prodcons_response_p_consume_end();

	// log
	file_write_log_file(6, id);
}

//******************************************
// MEMORIA_CRIAR_INDICADORES
//
void memory_create_statistics() {
	//==============================================
	// CRIAR ZONAS DE MEMÓRIA PARA OS INDICADORES
	//
	// criação dinâmica de memória
	// para cada campo da estrutura indicadores
	//so_memory_create_statistics();

	Ind.pid_clientes = calloc(Config.CLIENTES,sizeof(int));
	Ind.pid_intermediarios = calloc(Config.INTERMEDIARIO,sizeof(int));
	Ind.pid_empresas = calloc(Config.EMPRESAS,sizeof(int));
	Ind.propostas_entregues_por_empresas = memory_create("empresas",Config.SERVICOS*Config.EMPRESAS*sizeof(int));

	// iniciar indicadores relevantes:
	// - Ind.capacidade_inicial_servicos (c/ Config.stock respetivo)
	// - Ind.clientes_servidos_por_intermediarios (c/ 0)
	// - Ind.clientes_servidos_por_empresas (c/ 0)
	// - Ind.serviços_recebidos_por_clientes (c/ 0)

	Ind.capacidade_inicial_servicos = malloc(sizeof(Config.capacidade_servico));
	memcpy(Ind.capacidade_inicial_servicos,Config.capacidade_servico,Config.SERVICOS*sizeof(int));
	
	Ind.clientes_servidos_por_intermediarios = calloc(Config.INTERMEDIARIO,sizeof(int));
	Ind.servicos_recebidos_por_clientes = calloc(Config.CLIENTES,sizeof(int));
	Ind.clientes_servidos_por_empresas = calloc(Config.EMPRESAS,sizeof(int));


	// so_memory_prepare_statistics();
	//==============================================
}

