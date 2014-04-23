/* MC504 - 1s2014 - Projeto 02 - Sushi
**
** Andre Seiji Tamanaha - RA116134
** Guilherme Costa Zanelato - RA119494
** João Victor Chencci Marques - RA119637
**
*/

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#define NO_OF_CUSTOMERS 20 //
#define TIMER 30000

void insert_client(int client_id);
void remove_client(int client_id);
void display_table(int client_id);
int choose_position(int i, int st); 
int client_eating();

pthread_t customers[NO_OF_CUSTOMERS];

int eating = 0, waiting = 0, sitting = 0, leaving = 0, all_leaving = 0; /*flags de estado*/
int no_of_customers, n_spaces; //PETER

sem_t block;
pthread_mutex_t mutex;
int must_wait = 0;

/* exibição de estados dos clientes */
/* Waiting, Sitting, Eating, Leaving, Out */
typedef enum {W, S, E, L, O} state_t;
state_t state[NO_OF_CUSTOMERS];

int spot[NO_OF_CUSTOMERS]; /*vetor de spot x dos clientes/sushis*/ //PETER


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

		/* Muda estado do cliente para COMENDO */
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

	int position; //PETER
	for(position=0; position<NO_OF_CUSTOMERS; position++) {
		spot[position] = 0;
	}

	srand ( time(NULL) );

	/* calcula o numero de espacos entre cada cliente */
	n_spaces = 45/(6)-1; //PETER

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

void remove_client(int client_id) {
	int i, n_clients = 0;

	for(i=0; i<NO_OF_CUSTOMERS; i++) {
		if(spot[i] != 0) //PETER
			n_clients++;
	}

	if(n_clients != 5) {
		for(i=0; i<NO_OF_CUSTOMERS; i++) {
			if(state[i] == L) {
				spot[i] = 0; //PETER
				leaving--;
			}
		}
	}
	else {
		if(leaving == 5) {
			all_leaving = 1;

			int index, j, k, p, found=0;

			for(k=0; k < (leaving*(n_spaces+1) + 9); k++) {
				usleep(TIMER);
				display_table(client_id);

				for(p=k; p<10; p++) {
					printf(" ");
				}

				for(index=1; index<=45; index++){
					for(j=0; j<NO_OF_CUSTOMERS; j++) {
						if((spot[j]-10) == index) { //PETER
							printf("S");
							found = 1;
							if(k>=10) {
								spot[j]--; 	/* decrementa a posicao do cliente */ //PETER
							}
						}
					}
					if(!found) {
						printf(" ");
					}
					found=0;
				}
				printf("\n");
			}

			all_leaving = 0;

			leaving = 0;
			int position;
			for(position=0; position<NO_OF_CUSTOMERS; position++) {
				spot[position] = 0;
			}
		}
	}
}

void display_table(int client_id) {

	int i, j, found = 0, sitting = 0, eating = 0, leaving = 0;

	system("clear"); /* limpa a tela */

	/* checa quantidade comendo */
	for(j=0; j<NO_OF_CUSTOMERS; j++) {
		if(state[j]==E)
			eating += 1;
	}

	/* checa se alguem esta sentando */
	for(j=0; j<NO_OF_CUSTOMERS; j++) {
		if(state[j]==S)
			sitting = 1;
	}

	/* checa se alguem esta saindo */
	for(j=0; j<NO_OF_CUSTOMERS; j++) {
		if(state[j]==L)
			leaving = 1;
	}

	/* imprime mesa do sushibar */
	printf("\n\nMC504 - Projeto 02 - Sushi\n\n");

	printf("CLIENTES NA FILA: %d\n\n", waiting);

	printf("\n\n");

	printf("	|‾‾‾‾‾‾‾‾‾‾‾|\n");
	printf("	|          ");
	if(choose_position(0,E)){
		printf("@|o/  ||  CLIENTE COMENDO");
	} else if(choose_position(0,S)){
		printf(" |  <<  CLIENTE ENTRANDO");
	}else if(all_leaving){
		printf(" |  >>  CLIENTE SAINDO");
	}else if (!sitting){
		printf(" |o/  ||  CLIENTE ESPERANDO");
	}else{
		printf(" |");
	}
	printf("\n");
	printf("	|           |\n");
	printf("	|          ");
	if(choose_position(1,E)){
		printf("@|o/  ||  CLIENTE COMENDO");
	} else if(choose_position(1,S)){
		printf(" |  <<  CLIENTE ENTRANDO");
	}else if(all_leaving){
		printf(" |  >>  CLIENTE SAINDO");
	}else if (!sitting){
		printf(" |o/  ||  CLIENTE ESPERANDO");
	}else{
		printf(" |");
	}
	printf("\n");
	printf("	|           |\n");
	printf("	|          ");
	if(choose_position(2,E)){
		printf("@|o/  ||  CLIENTE COMENDO");
	} else if(choose_position(2,S)){
		printf(" |  <<  CLIENTE ENTRANDO");
	}else if(all_leaving){
		printf(" |  >>  CLIENTE SAINDO");
	}else if (!sitting){
		printf(" |o/  ||  CLIENTE ESPERANDO");
	}else{
		printf(" |");
	}
	printf("\n");
	printf("	|           |\n");
	printf("	|          ");
	if(choose_position(3,E)){
		printf("@|o/  ||  CLIENTE COMENDO");
	} else if(choose_position(3,S)){
		printf(" |  <<  CLIENTE ENTRANDO");
	}else if(all_leaving){
		printf(" |  >>  CLIENTE SAINDO");
	}else if (!sitting){
		printf(" |o/  ||  CLIENTE ESPERANDO");
	}else{
		printf(" |");
	}
	printf("\n");
	printf("	|           |\n");
	printf("	|          ");
	if(choose_position(4,E)){
		printf("@|o/  ||  CLIENTE COMENDO");
	} else if(choose_position(4,S)){
		printf(" |  <<  CLIENTE ENTRANDO");
	}else if(all_leaving	){
		printf(" |  >>  CLIENTE SAINDO");
	}else if (!sitting){
		printf(" |o/  ||  CLIENTE ESPERANDO");
	}else{
		printf(" |");
	}
	printf("\n");
	printf("	|           |\n");
	printf("	|___________|\n\n\n");

}

/*imprime clientes entrando*/
void insert_client(int client_id) {
	int i, j, k;      

	if(eating == 1) {
		/* posicao final do cliente (quanto maior, mais pra esquerda anda) */
		for(i=0; i<61-n_spaces; i++) {
			usleep(TIMER);
			display_table(client_id); 

			/* posicao inicial de entrada (quanto maior, mais longe sai da fila) */
			for(j=i; j<70; j++) {
				printf(" ");
			}

			printf("C\n");
		}
		spot[client_id] = j-i+1;
	}
	else {
		/* posicao final do cliente (quanto maior, mais pra esquerda anda) */
		for(i=0; i<(71-n_spaces*eating-n_spaces); i++) {
			usleep(TIMER);
			display_table(client_id); 

			/* posicao inicial do primeiro */
			for(j=(61-n_spaces); j<70; j++) {
				printf(" ");
			}
			printf("C");

			for(j=2; j<eating; j++) {
				for(k=0; k<n_spaces; k++) {
					printf(" ");
				}
				printf("C");
			}
			/* posicao inicial de entrada (quanto maior, mais longe sai da fila) */
			for(j=i; j<(70-eating*n_spaces); j++) {
				printf(" ");
			}

			printf("C\n");
		}
		spot[client_id] = eating*(n_spaces+1) + 9;
	}
}

int choose_position(int i, int st) {	
	return (state[i]==st || state[i+5]==st || state[i+10]==st || state[i+15]==st);
}

int client_eating() {
	int i;
	for (i=0; i<20; i++) {
		if (state[i] == E) {
			return 1;
		}
	}
	return 0;
}
