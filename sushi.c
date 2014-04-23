/*SUSHI BAR
**
** Andre Nogueira Brandao - RA116130
** Andre Seiji Tamanaha - RA116134
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

#define ANSI_COLOR_RED     "\033[22;31m"
#define ANSI_COLOR_GREEN   "\033[22;32m"
#define ANSI_COLOR_YELLOW  "\033[01;33m"
#define ANSI_COLOR_BLUE    "\033[22;34m"
#define ANSI_COLOR_MAGENTA "\033[22;35m"
#define ANSI_COLOR_CYAN    "\033[22;36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define NO_OF_CUSTOMERS 5
#define TIMER 30000
#define TABLE_SIZE 45

void insert_sushibar(int client_id);
void remove_client(int client_id);
void display_table(int client_id);

pthread_t customers[NO_OF_CUSTOMERS];

int eating = 0, waiting = 0, sitting = 0, leaving = 0, all_leaving = 0; /*flags de estado*/
int no_of_customers, n_spaces;
int no_of_chairs = 5;

sem_t block;
pthread_mutex_t mutex;
int must_wait = 0;

/* exibição de estados dos clientes */
/* Waiting, Sitting, Eating, Leaving, Out */
typedef enum {W, S, E, L, O} state_t;
state_t state[NO_OF_CUSTOMERS];

int spot[NO_OF_CUSTOMERS]; /*vetor de spot x dos clientes/sushis*/


void* sushi_bar(void* arg) { 
	int client_id = *(int *) arg;

	while(1){
		int i, n;

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
		must_wait = (eating == no_of_chairs);

		/* muda o estado do cliente para SITTING */
		state[client_id] = S;
		insert_sushibar(client_id);

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

	int position;
	for(position=0; position<NO_OF_CUSTOMERS; position++) {
		spot[position] = 0;
	}

	srand ( time(NULL) );

	/* calcula o numero de espacos entre cada cliente */
	n_spaces = TABLE_SIZE/(no_of_chairs+1)-1; 

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
		if(spot[i] != 0)
			n_clients++;
	}

	if(n_clients != no_of_chairs) {
		for(i=0; i<NO_OF_CUSTOMERS; i++) {
			if(state[i] == L) {
				spot[i] = 0;
				leaving--;
			}
		}
	}
	else {
		if(leaving == no_of_chairs) {
			all_leaving = 1;

			int index, j, k, p, found=0;

			for(k=0; k < (leaving*(n_spaces+1) + 9); k++) {
				usleep(TIMER);
				display_table(client_id);

				for(p=k; p<10; p++) {
					printf(" ");
				}

				for(index=1; index<=TABLE_SIZE; index++){
					for(j=0; j<NO_OF_CUSTOMERS; j++) {
						if((spot[j]-10) == index) {
							printf("S");
							found = 1;
							if(k>=10) {
								spot[j]--; 	/* decrementa a posicao do cliente */
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
	system("clear");
	/*imprime mesa do sushibar*/
	printf("\n\n");
	printf("                          ___________\n");
	printf("                         | SUSHI BAR |\n"); 
	printf("                          ‾‾‾‾‾‾‾‾‾‾‾\n");                                        
	printf("\n");
	printf("                漢"ANSI_COLOR_RED"o"ANSI_COLOR_RESET"字         漢"ANSI_COLOR_RED"o"ANSI_COLOR_RESET"字          漢"ANSI_COLOR_RED"o"ANSI_COLOR_RESET"字         \n");                                            
	printf("         |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|");

	printf("   CLIENTES ESPERANDO\n");

	printf("         |        "ANSI_COLOR_YELLOW"_______"ANSI_COLOR_RESET"            "ANSI_COLOR_YELLOW"_______"ANSI_COLOR_RESET"           |");   
	
	printf("        |‾‾‾‾‾‾|\n");

	printf("         |     "ANSI_COLOR_YELLOW"-<|"ANSI_COLOR_GREEN"@@@@@@@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_GREEN"@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_RESET"      "ANSI_COLOR_YELLOW"-<|"ANSI_COLOR_GREEN"@@@@@@@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_GREEN"@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_RESET"        |");

	printf("        |  %d  ", waiting);
	if(waiting < 10)
		printf(" ");
	printf("|\n");

	printf("         |     "ANSI_COLOR_YELLOW"-<|"ANSI_COLOR_GREEN"@@@@@@@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_GREEN"@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_RESET"      "ANSI_COLOR_YELLOW"-<|"ANSI_COLOR_GREEN"@@@@@@@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_GREEN"@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_RESET"        |");  

	printf("        |______|\n");

	printf("         |        "ANSI_COLOR_YELLOW"‾‾‾‾‾‾‾"ANSI_COLOR_RESET"            "ANSI_COLOR_YELLOW"‾‾‾‾‾‾‾"ANSI_COLOR_RESET"           |\n");   

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

	printf("         |");
	
	/*imprime sushi caso cliente esteja comendo*/
	for(i=1; i<=45; i++){
		for(j=0; j<NO_OF_CUSTOMERS; j++) {
			if(state[j]==E && (spot[j]-10) == i) {
				printf(ANSI_COLOR_GREEN"@"ANSI_COLOR_RESET);
				found = 1;
			}
		}
		if(!found)
			printf("_");
		found=0;
	}

	printf("|");

	/*imprime estados da execucao do programa*/
	if(eating == no_of_chairs) {
		printf("    :: MESA CHEIA ::\n");
	}
	else if(all_leaving) {
		printf("  <== CLIENTES SAINDO\n");	
	}
	else if(sitting) {
		printf("  [CLIENTES ENTRANDO]\n");	
	}
	else
		printf("\n");

	/*imprime clientes que estao comendo*/
	if(!sitting && !leaving) {
		printf("          ");
		for(i=1; i<=45; i++){
			for(j=0; j<NO_OF_CUSTOMERS; j++) {
				if((spot[j]-10) == i) {
					printf("C");
					found = 1;
				}
			}
			if(!found)
				printf(" ");
			found=0;
		}
		printf("\n");
	}

	if(leaving && !all_leaving) {
		printf("          ");
		for(i=1; i<=45; i++){
			for(j=0; j<NO_OF_CUSTOMERS; j++) {
				if((spot[j]-10) == i) {
					printf("C");
					found = 1;
				}
			}
			if(!found)
				printf(" ");
			found=0;
		}
		printf("\n");
	}

	if(eating == no_of_chairs) {
		usleep(100000);
	}

}

/*imprime clientes entrando*/
void insert_sushibar(int client_id) {
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