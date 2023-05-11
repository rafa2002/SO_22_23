#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#define MAX_COMMAND 200

bool running = true;

struct Pedidos {
    char *pid;
    char **comando;
    char *timeStamp;
    struct Pedidos *prox_pedido;
};

int contar_pedidos(struct Pedidos *lista) {
    int contador = 0;
    while (lista != NULL) {
        contador++;
        lista = lista->prox_pedido;
    }
    return contador;
}

void imprimir_pedido(struct Pedidos *pedido) {
    printf(" ---- ---- ---- Pedido ---- ---- --- \n\n");
    printf("PID: %s\n", pedido->pid);
    printf("Comando: \n");
    int n_comandos = strlen(pedido->comando);
    printf("%d\n",n_comandos);
    for (int i = 0; i<n_comandos; i++) {
        printf("%s ", pedido->comando[i]);
    }
    printf("\n");
    printf("Timestamp: %s\n", pedido->timeStamp);
    
    printf("\n ---- ---- ---- FIM ---- ---- --- \n\n");
}

void imprimir_lista(struct Pedidos *lista) {
    printf(" ====== ====== ====== LISTA ====== ====== ====== \n\n");
    for (struct Pedidos *pedido = lista; pedido != NULL; pedido = pedido->prox_pedido) {
        imprimir_pedido(pedido);
    }
    printf(" ====== ====== ====== FIM ====== ====== ====== \n\n");
}

void adicionar_pedido(struct Pedidos *lista, char *v_pid, char **comando, char *timestamp, int q_comandos) {
    // Cria um novo nó
    struct Pedidos *novo_pedido = (struct Pedidos*) malloc(sizeof(struct Pedidos));
    novo_pedido->pid = v_pid;
    //printf("Adicionar comandos %lu\n",strlen(comando));
    printf("Comando[0]: %s\n",comando[0]);
    printf("Comando[1]: %s\n",comando[1]);
    printf("Comando[2]: %s\n",comando[2]);
    printf("Comando[3]: %s\n",comando[3]);
    for(int i = 0;i<strlen(comando);i++) {
        printf("Comando: %s\n",comando[1]);
        novo_pedido->comando[i] = malloc(sizeof(comando[i]))
        novo_pedido->comando[i] = comando[i];
    }
    novo_pedido->timeStamp = timestamp;
    novo_pedido->prox_pedido = NULL;
    printf("hey-----\n");
    // Se a lista estiver vazia, o novo nó será o primeiro e o último
    if (lista == NULL) {
        lista = novo_pedido;
        printf("ifffffff lista %lu\n",strlen(lista->comando));
        printf("ifffffff novo_pedido %lu\n",strlen(novo_pedido->comando));
        //imprimir_pedido(novo_pedido);
        //imprimir_lista(*lista);
    }
    // Caso contrário, o novo nó será adicionado no início da lista
    else {
        novo_pedido->prox_pedido = lista;
        lista = novo_pedido;
        printf("else lista %lu\n",strlen(lista->comando));
        printf("else novo_pedido %lu\n",strlen(novo_pedido->comando));
        //imprimir_pedido(novo_pedido);
        //imprimir_lista(*lista);
    }
    imprimir_pedido(novo_pedido);
    
    printf("bye\n");
}

char* remover_pedido(struct Pedidos *lista, char *pid) {
    struct Pedidos *pedido_atual = lista;
    struct Pedidos *pedido_anterior = NULL;
    char *time = NULL;
    while (pedido_atual != NULL) {
        if (strcmp(pedido_atual->pid, pid) == 0) {
            time = strdup(pedido_atual->timeStamp);
            if (pedido_anterior == NULL) {
                // o pedido a ser removido é o primeiro da lista
                lista = pedido_atual->prox_pedido;
            } else {
                // o pedido a ser removido está no meio da lista
                pedido_anterior->prox_pedido = pedido_atual->prox_pedido;
            }
            free(pedido_atual);
            return time;
        }
        pedido_anterior = pedido_atual;
        pedido_atual = pedido_atual->prox_pedido;
    }
    return time;
}

int main(int argc, char ** argv) {
    
    // Criação da lista de pedidos
    struct Pedidos *pedidos = NULL;

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

    int canal = open("canal", O_RDONLY | O_CREAT | O_TRUNC);
    printf("valor do canal [%d]\n",canal);
    
    if(canal == -1) {
        perror("Erro ao abrir o canal nomeado");
        exit(1);
    }
    
    
    //ir lendo os pedidos enviados por clientes
    char buffer[MAX_COMMAND] = "";
    ssize_t bytes_lidos;
    
    while(running)
    {
        while((bytes_lidos = read(canal,&buffer,MAX_COMMAND))>0) {
            char token[100];
            //procurar até ao /n e guardar em string
            strcpy(token, strtok(buffer, "\n"));
            // Separação dos argumentos do pedido em string
            char ** pedido = malloc(40*sizeof(char *));
            int i = 0;
            char *token_aux = strtok(token," ");
            while (token_aux != NULL) {
                    printf("Este é o token_aux: %s\n",token_aux);
                    pedido[i] = malloc(sizeof(token_aux));
                    strcpy(pedido[i], token_aux);
                    token_aux = strtok(NULL, " ");
                    i++;
                }
            
            int n = 0;
            while (pedido[n] != NULL) {
                n++;
            }
            char *estado = pedido[0];
            if(!strcmp(estado,"Running"))
            {

                char *pid = pedido[n-2];
                char *timestamp = pedido[n-1];
                char ** comando = malloc((n-3)*sizeof(char *));
                for(int i = 1; i<n-2;i++){
                    int bytes = sizeof(pedido[i]);
                    comando[i-1] = malloc(bytes);
                    strcpy(comando[i-1], pedido[i]);
                }
                //printf("valor de n : %d\n",n);
                
                printf(" ----- Antes de invocar adicionar_pedido ----\n");
                adicionar_pedido(pedidos,pid,comando,timestamp,n-3);
                printf(" ----- Antes de invocar adicionar_pedido ----\n");
                //imprimir_lista(*pedidos);
                printf(" ----- Antes de invocar adicionar_pedido ----\n");
                /*for(int i = 0; i<n-3;i++)
                {
                    
                    printf("comando[%d]: %s\n",i,comando[i]);
                    free(comando[i]);
                }
                free(comando);*/
            }
            printf("PASSSAA AQUII\n");
            if(!strcmp(estado,"Ended")){
                printf("OK ended------ \n");
                // A mensagem_final é a linha que se escreve em cada pid.txt com o seguinte formato:
                // PID COMANDOS TIME(duracao)
                char mensagem_final[n];
                char *txt = ".txt";
                char *pid = pedido[n-2];
                strcpy(mensagem_final,pid);
                char *timestamp = pedido[n-1];
                char ** comando = malloc((n-3)*sizeof(char *));
                for(int i = 1; i<n-2;i++){
                    strcat(mensagem_final,pedido[i]);
                    int bytes = sizeof(pedido[i]);
                    comando[i-1] = malloc(bytes);
                    strcpy(comando[i-1], pedido[i]);
                }
                
                // Guardar pedido na lista de pedidos
                char *timestampinicial = remover_pedido(pedidos,pid);
                int duracao = atoi(timestamp) - atoi(timestampinicial);
                char *duracao_str = malloc(sizeof(char)*10);
                sprintf(duracao_str,"%d",duracao);
                strcat(mensagem_final,duracao_str);
                
                int j = strlen(txt) + strlen(pid);
                char f[j];
                strcpy(f,pid);
                strcat(f,txt);
                printf("nome do ficheiro file_pid: %s",f);
                
                // criar ficheiro com pid do processo como nome
                int file_pid = open(f,O_RDWR | O_CREAT | O_TRUNC);
                write(file_pid,mensagem_final,sizeof(mensagem_final));
                
                printf("valor do file_pid: %d\n",file_pid);
                close(file_pid);
                
            }
            if(!strcmp(estado,"status")){
                printf("OK status------ \n");
                char *pid = pedido[1];
                struct Pedidos *pedido_atual = pedidos;
                int canal_status = open(pid,O_CREAT | O_TRUNC | O_WRONLY);
                
                while (pedido_atual != NULL) {
                    
                    struct timeval timestamp_at_moment;
                    gettimeofday(&timestamp_at_moment, NULL);
                    int mili_seconds_atmoment = timestamp_at_moment.tv_usec/1000;
                    
                    int at_moment = mili_seconds_atmoment - atoi(pedido_atual->timeStamp);
                    
                    char mensagem_status[200];
                    strcpy(mensagem_status, "status");
                    for(int i = 0; i<strlen(*pedido_atual->comando);i++)
                        strcat(mensagem_status,pedido_atual->comando[i]);
                    
                    char resto[20];
                    sprintf(resto,"%s %d\n",pedido_atual->pid, at_moment);
                    strcat(mensagem_status,resto);

                    free(pedido_atual);
                    pedido_atual = pedido_atual->prox_pedido;
                    write(canal_status,mensagem_status,sizeof(mensagem_status));
                    //free(mensagem_status);
                    close(canal_status);
                }
            }
        }
        if (bytes_lidos < 0) {
            perror("Erro na leitura do pedido");
            continue;
        }
        /*if (bytes_lidos == 0 && pedidos==NULL) {
            printf("OK entra no if bytes_lidos == 0 && pedidos==NULL------ \n");
            running = false;
        }*/
    }
    printf("final do codigo\n");
    close(canal);
    unlink("canal");
    return 0;
}
