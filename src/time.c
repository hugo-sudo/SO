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
#include <math.h>

#include "main.h"
#include "so.h"
#include "sotime.h"

struct timespec t_inicial;
long intervalo_alarme;

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
//==============================================

struct timespec t_inicial;
long intervalo_alarme;

void time_begin(long intervalo) {
    //==============================================
    // INICIAR ESTRUTURA t_inicial COM VALOR DE RELOGIO (CLOCK_REALTIME)
	//
	// funções de tempo:
	// - clock_gettime() dá um resultado em nanosegundos
	// - gettimeofday()  dá um resultado em milisegundos
	// como a função clock_gettime() dá um valor mais preciso do que gettimeofday()
	// deve ser usada clock_gettime()
	//
    // fazer:
	// - se intervalo!=0 então intervalo_alarme = intervalo
	// - se intervalo!=0 então chamar time_setup_alarm();
	// - iniciar estrutura t_inicial com clock_gettime usando CLOCK_REALTIME

    if (intervalo!=0){
        intervalo_alarme = intervalo;
        time_setup_alarm();
    }
    clock_gettime(CLOCK_REALTIME, &t_inicial);
    // so_time_begin(intervalo);
    //==============================================
}

void time_destroy(long intervalo)
{
    //==============================================
    // DESATIVAR ALARME
    //
	// ignorar SIGALRM
    signal(SIGALRM,SIG_IGN);

    //so_time_destroy(intervalo);
    //==============================================
}

void time_setup_alarm() {
    //==============================================
    // ARMAR ALARME DE ACORDO COM intervalo_alarme (SIGNAL E SETTIMER)
    //
	// fazer:
	// - associar SIGALRM com a função time_write_log_timed
	// - usar setitimer preenchendo apenas os campos value da estrutura
    signal(SIGALRM, time_write_log_timed);
    struct itimerval timer;
 
    timer.it_value.tv_sec = intervalo_alarme / 1000000;
    timer.it_value.tv_usec = intervalo_alarme;
    timer.it_interval = timer.it_value;

    setitimer (ITIMER_REAL , &timer, 0);
    // so_time_setup_alarm();
    //==============================================
}

void time_write_log_timed(int signum) {
    //==============================================
    // ESCREVER LOG NO ECRAN DE FORMA TEMPORIZADA 
    //
	// rearmar alarme chamando novamente time_setup_alarm
	// escrever para o ecrã a informação esperada
    so_time_write_log_timed(signum);
    //==============================================
}

double time_difference(struct timespec t1, struct timespec t2) {
    //==============================================
    // CALCULAR A DIFERENCA, EM NANOSEGUNDOS, ENTRE t1 E t2
    // o resultado deve estar em segundos representado como um double
	// realizar as operações aritméticas necessárias para obter o resultado
    double pow(double x, double y);
    double difference = abs(t1.tv_nsec - t2.tv_nsec) * pow(10,-9);
    return difference; 
    // return so_time_difference(t1,t2);
    //==============================================
}

double time_untilnow() {
    //==============================================
    // CALCULAR O INTERVALO DE TEMPO ENTRE t_inicial E O INSTANTE ATUAL
    //
	// fazer:
	// - obter o tempo atual com clock_gettime
	// - chamar time_difference
    struct timespec t2;
    clock_gettime(CLOCK_REALTIME, &t2);
    return time_difference(t_inicial, t2 );
    //==============================================
}

void time_register(struct timespec *t) {
    //==============================================
    // REGISTAR O TEMPO ATUAL EM t (CLOCK_REALTIME)
    //
	// usar clock_gettime com CLOCK_REALTIME
    clock_gettime(CLOCK_REALTIME, t);
    // so_time_register(t);
    //==============================================
}

void time_processing_order() {
	//==============================================
	// ADORMECER POR 1 MILISEGUNDO
	//
	// usar usleep
    usleep(1000);
	// so_time_processing_order();
	//==============================================
}
