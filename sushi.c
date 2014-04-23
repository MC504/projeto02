/* MC504 - 1s2014 - Projeto 02 - Sushi
**
** Andre Seiji Tamanaha - RA116134
** Guilherme Costa Zanelato - RA119494
** João Victor Chencci Marques - RA119637
*/

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#define NO_OF_CUSTOMERS 20 /* numero de threads == clientes */
#define TIMER 30000

void insert_client(int client_id);
void remove_client(int client_id);
void display_table(int client_id);
void verify_state(int i, int c_sitting);

pthread_t customers[NO_OF_CUSTOMERS];


int eating = 0, waiting = 0, sitting = 0, leaving = 0, all_leaving = 0; /* flags de estado */

sem_t block;
pthread_mutex_t mutex;
int must_wait = 0;

/* exibição de estados dos clientes */
/* Waiting, Sitting, Eating, Leaving, Out */
typedef enum {W, S, E, L, O} state_t;
state_t state[NO_OF_CUSTOMERS];

int active[NO_OF_CUSTOMERS];


void* sushi_bar(void* arg) { 
	int client_id = *(int *) arg;

	while(1){

		pthread_mutex_lock(&mutex);

		if(must_wait) { 
			waiting+=1;

			/* CLIENTE ESPERANDO */
			state[client_id] = W;
			display_table(client_id);

			/* MUTEX LIBERADO */
			pthread_mutex_unlock(&mutex);

			sem_wait(&block);
			waiting -= 1;
		} 
		eating+=1;
		must_wait = (eating == 5);

		/* muda o estado do cliente para SITTING */
		state[client_id] = S;
		insert_client(client_id);

		/* muda estado do cliente para COMENDO */
		state[client_id] = E;
		display_table(client_id);


		/* MUTEX LIBERADO */
		if(waiting && !must_wait) 
			sem_post(&block);
		else
			pthread_mutex_unlock(&mutex);

		/* Sleep randomico para cada thread comer */
		sleep(rand() % 10 + 1);

		pthread_mutex_lock(&mutex);
		eating-=1;
		state[client_id] = L;

		/* CLIENTE SAINDO */
		display_table(client_id);
		leaving++;
		remove_client(client_id);

		if(eating == 0)
			must_wait = 0;

		/* Muda o estado do cliente para OUT (saiu do Sushi Bar) */
		state[client_id] = O;
		if(waiting && !must_wait)
			sem_post(&block);
		else
			pthread_mutex_unlock(&mutex);

		/* cliente espera 3s fora do bar */
		sleep(3);
	} 
} 

int main() { 
	int i=0;
	char c;
	int customer_id[NO_OF_CUSTOMERS];

	for(i=0; i<NO_OF_CUSTOMERS; i++) {
		active[i] = 0;
	}

	srand ( time(NULL) );

	/* inicia o id dos clientes e os estados como WAITING */
	for(i=0;i<NO_OF_CUSTOMERS;i++) {
		customer_id[i]=i;
		state[i] = W;
	}

  /* inicia o mutex */
	pthread_mutex_init(&mutex,0);

  /* inicia o semaphore block */
	sem_init(&block,0,0);

	/* cria as Threads */
	for(i=0;i<NO_OF_CUSTOMERS;i++) {
		pthread_create(&customers[i],0,sushi_bar,&customer_id[i]);
	}

	/* tentativa de sair do programa com a tecla 'q' - ainda nao sucedida */
	while(c != 'q') {
		c = getchar();
	};

	return 0;
} 

void display_table(int client_id) {

	int j, sitting = 0, eating = 0, leaving=0;

	system("clear"); /* limpa a tela */

	/* checa quantidade comendo */
	for(i=0; i<NO_OF_CUSTOMERS; i++) {
		if(state[i]==E)
			eating += 1;
	}

	/* checa se alguem esta sentando */
	for(i=0; i<NO_OF_CUSTOMERS; i++) {
		if(state[i]==S)
			sitting = 1;
	}

	/* checa se alguem esta saindo */
	for(i=0; i<NO_OF_CUSTOMERS; i++) {
		if(state[i]==L)
			leaving = 1;
	}

	/* imprime mesa  */
	printf("\nMC504 - Projeto 02 - Sushi\n\n");

	printf("CLIENTES NA FILA: %d\n\n", waiting);

	printf("\n\n");

	printf("	|‾‾‾‾‾‾‾‾‾‾‾|\n");
	printf("	|          "); verify_state(0, sitting); printf("\n");
	printf("	|           |\n");
	printf("	|          "); verify_state(1, sitting); printf("\n");
	printf("	|           |\n");
	printf("	|          "); verify_state(2, sitting); printf("\n");
	printf("	|           |\n");
	printf("	|          "); verify_state(3, sitting); printf("\n");
	printf("	|           |\n");
	printf("	|          "); verify_state(4, sitting); printf("\n");
	printf("	|           |\n");
	printf("	|___________|\n\n\n");
}

/* imprime clientes entrando */
void insert_client(int client_id) {
	int i;  

	if(eating == 1) {
		for(i=0; i<NO_OF_CUSTOMERS; i++) {
			usleep(TIMER);
			display_table(client_id); 
		}
		active[client_id] = 1;
	}
	else {
		for(i=0; i<(NO_OF_CUSTOMERS); i++) {
			usleep(TIMER);
			display_table(client_id); 
		}
		active[client_id] = 1;
	}
}

/* imprime clientes saindo */
void remove_client(int client_id) {

	int i, n_clients = 0;

	for(i=0; i<NO_OF_CUSTOMERS; i++) {
		if(active[i] != 0)
			n_clients++;
	}

	if(n_clients != 5) {
		for(i=0; i<NO_OF_CUSTOMERS; i++) {
			if(state[i] == L) {
				active[i] = 0;
				leaving--;
			}
		}
	}
	else {
		if(leaving == 5) {
			all_leaving = 1;

			for(i=0; i<NO_OF_CUSTOMERS; i++) {
				usleep(TIMER);
				display_table(client_id);
			}

			all_leaving = 0;

			leaving = 0;

			for(i=0; i<NO_OF_CUSTOMERS; i++) {
				active[i] = 0;
			}
		}
	}
}

/* verifica estado do cliente */
void verify_state(int i, int c_sitting) {
	if(state[i]==E || state[i+5]==E || state[i+10]==E || state[i+15]==E){
		printf("@|o/  ||  CLIENTE COMENDO");
	}else if(state[i]==S || state[i+5]==S || state[i+10]==S || state[i+15]==S){
		printf(" |  <<  CLIENTE ENTRANDO");
	}else if(all_leaving){
		printf(" |  >>  CLIENTE SAINDO");
	}else if (!c_sitting){
		printf(" |o/  ||  CLIENTE ESPERANDO");
	}else{
		printf(" |");
	}
}
