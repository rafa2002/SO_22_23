#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#define MAX_COMMAND 256

bool running = true;
bool iniciar = true;

struct Pedidos {
    char *pid;
    char *comando;
    char *timeStamp;
    struct Pedidos *prox_pedido;
};

void adicionar_pedido(struct Pedidos **lista, char *pid, char *comando, char *timeStamp) {
    struct Pedidos *novo_pedido = (struct Pedidos*)malloc(sizeof(struct Pedidos));
    novo_pedido->pid = pid;
    novo_pedido->comando = comando;
    novo_pedido->timeStamp = timeStamp;
    novo_pedido->prox_pedido = *lista;
    *lista = novo_pedido;
}

void remover_pedido(struct Pedidos **lista, char *pid) {
    struct Pedidos *pedido_atual = *lista;
    struct Pedidos *pedido_anterior = NULL;

    while (pedido_atual != NULL) {
        if (strcmp(pedido_atual->pid, pid) == 0) {
            if (pedido_anterior == NULL) {
                // o pedido a ser removido é o primeiro da lista
                *lista = pedido_atual->prox_pedido;
            } else {
                // o pedido a ser removido está no meio da lista
                pedido_anterior->prox_pedido = pedido_atual->prox_pedido;
            }
            free(pedido_atual);
            return;
        }
        pedido_anterior = pedido_atual;
        pedido_atual = pedido_atual->prox_pedido;
    }
}

int main(int argc, char ** argv) {

    int m = mkfifo("canal",0666);
    printf("valor do makefifo %d\n",m);
    printf("valor do errno %d\n",errno);
    // criar o qpipe para receber pedidos dos clientes
    if (m == -1) {
        if (errno == EEXIST) {
            unlink("canal");
            m = mkfifo("canal", 0666);
        }
    }

    int canal = open("canal", O_RDONLY);
    printf("valor do canal [%d]\n",canal);
    
    if(canal == -1) {
        perror("Erro ao abrir o canal nomeado");
        exit(1);
    }
    printf("Estamos aqui fora!!!");
    //ir lendo os pedidos enviados por clientes
    char buffer[MAX_COMMAND] = "";
    
    while(running) {
        printf("Estamos dentro!!");
        int bytes_lidos = 0;
        char *byte_lido = malloc(sizeof(char));
        while((bytes_lidos += read(canal,&byte_lido, 1))){
            // Se o byte lido for o caractere nulo, processa a mensagem
            if (!strcmp(byte_lido,"\0") && !iniciar) {
                iniciar = false;
                printf("Mensagem recebida: %s\n", buffer);
                strncat(buffer, &byte_lido, 1);
                close(canal);
                break;
            } else {
                // Se o byte lido não for o caractere nulo, adiciona ao buffer
                strncat(buffer, &byte_lido, 1);
            }
        }
        
        if (bytes_lidos == -1) {
            perror("Erro ao ler do canal");
            exit(EXIT_FAILURE);
        } else if (bytes_lidos == 0) {
            //Aqui vamos ter de fazer alguma coisa
            iniciar = false;
            close(canal);
            break;
        }
        
        if (bytes_lidos > 0) {
            buffer[bytes_lidos] = '\0';
            
            time_t t = time(NULL);
            struct tm *timestamp = localtime(&t);
            
            printf("Comando recebido: %s\n", buffer);
            
            //parse do comando recebido do cliente
            int pid = fork();
            if (pid == 0) {
                // filho
                printf("Executando comando...\n");
                
                // divide o comando em tokens
                char *token;
                char **info = malloc(sizeof(char*) * (MAX_COMMAND/2 + 1));
                int count = 0;
                token = strtok(buffer, " ");
                while (token != NULL) {
                    info[count] = token;
                    count++;
                    token = strtok(NULL, " ");
                }
                info[count] = NULL;
                
                free(info);
                //exit(PID);
            }
            else {
                if (pid > 0) {
                    // pai
                    int status;
                    wait(&status);
                    
                    time_t tf = time(NULL);
                    struct tm *timestampfinal = localtime(&tf);
                    
                    printf("Comando finalizado com status %d\n", status);
                    
                    char message[MAX_COMMAND];
                    int tamanho_str = sprintf(message, "Ended PID %d %04d-%02d-%02d %02d:%02d:%02d\n", pid,
                                              timestampfinal->tm_year + 1900, timestampfinal->tm_mon + 1, timestampfinal->tm_mday,
                                              timestampfinal->tm_hour, timestampfinal->tm_min, timestampfinal->tm_sec);
                    
                    write(1, message, tamanho_str);
                }
                else {

                    perror("Erro ao criar processo filho");
                }
            }
        }
    }
    printf("final do codigo\n");
    close(canal);
    unlink("canal");
    return 0;
}
