#ifndef MEMORY_H_GUARD
#define MEMORY_H_GUARD

// Nomes usados na criação de zonas de memoria partilhada
#define STR_SHM_STOCK 				"shm_stock"
#define STR_SHM_RECEIPT_PTR 		"shm_receipt_ptr"
#define STR_SHM_RECEIPT_BUFFER 		"shm_receipt_buffer"
#define STR_SHM_ORDER_PTR 			"shm_order_ptr"
#define STR_SHM_ORDER_BUFFER 		"shm_order_buffer"
#define STR_SHM_CURRENCY_PTR 		"shm_currency_ptr"
#define STR_SHM_CURRENCY_BUFFER 	"shm_currency_buffer"
#define STR_SHM_SCHEDULER 			"shm_scheduler"
#define STR_SHM_PRODEXCHANGES		"shm_currencies_exchanges"

// estrutura que contêm os ponteiros para um buffer circular
struct pointers {
	int in;  // ponteiro de escrita
	int out; // ponteiro de leitura
};

// estrutura onde são guardadas as descricoes de projetos pessoais do cliente para o intermediario
struct request_d {
	struct service *buffer;   // ponteiro para a lista de descricoes
	struct pointers *ptr; // ponteiro para a estrutura de índices de escrita e leitura
};

// estrutura onde são guardados os pedidos de orcamento da intermediario para a empresa
struct request_b {
	struct service *buffer; // ponteiro para a lista de pedidos de orcamento
	int *ptr;             // ponteiro para a lista de inteiros que indicam
						  // se a posição respetiva está livre ou ocupada
};

// estrutura onde a empresa disponibiliza as propostas
struct response_p {
	struct service *buffer; // ponteiro para a lista de propostas
	int *ptr;             // ponteiro para a lista de inteiros que indicam
						  // se a posição respetiva está livre ou ocupada
};

void * memory_create(char*, int);
void memory_create_capacidade_servicos();
void memory_create_buffers();
void memory_create_scheduler();
void memory_destroy_all();
void memory_request_d_write(int, struct service *);
int  memory_request_d_read(int, struct service *);
void memory_request_b_write(int, struct service *);
int  memory_request_b_read(int, struct service *);
void memory_response_p_write(int, struct service *);
void memory_response_p_read(int, struct service *);
void memory_create_statistics();
void memory_destroy(char * name, void * ptr, int size);

#endif
