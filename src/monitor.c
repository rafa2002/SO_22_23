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
    char *comando;
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
    printf("Comando: %s\n",pedido->comando);
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

struct Pedidos * adicionar_pedido(struct Pedidos *lista, char *v_pid, char *comando, char *timestamp, int q_comandos) {
    // Cria um novo nó
    struct Pedidos *novo_pedido = (struct Pedidos*) malloc(sizeof(struct Pedidos));
    novo_pedido->pid = v_pid;
    novo_pedido->comando = comando;
    novo_pedido->timeStamp = timestamp;
    novo_pedido->prox_pedido = NULL;
    //printf("hey-----\n");
    // Se a lista estiver vazia, o novo nó será o primeiro e o último
    if (lista == NULL) {
        lista = novo_pedido;
        //imprimir_pedido(novo_pedido);
        //imprimir_lista(*lista);
    }
    // Caso contrário, o novo nó será adicionado no início da lista
    else {
        novo_pedido->prox_pedido = lista;
        lista = novo_pedido;
        //printf("else lista %lu\n",strlen(lista->comando));
        //printf("else novo_pedido %lu\n",strlen(novo_pedido->comando));
    }
    //imprimir_pedido(novo_pedido);
    //imprimir_lista(lista);
    return lista;
    //printf("bye\n");
}

char* remover_pedido(struct Pedidos *lista, char *pid) {
    struct Pedidos *pedido_atual = lista;
    struct Pedidos *pedido_anterior = NULL;
    char *time = malloc(sizeof(char)*10);
    while (pedido_atual != NULL) {
        if (strcmp(pedido_atual->pid, pid) == 0) {
            //printf("tempo no pedido: %s\n", pedido_atual->timeStamp);
            time = strdup(pedido_atual->timeStamp);
            if (pedido_anterior == NULL) {
                // o pedido a ser removido é o primeiro da lista
                lista = pedido_atual->prox_pedido;
            } else {
                // o pedido a ser removido está no meio da lista
                pedido_anterior->prox_pedido = pedido_atual->prox_pedido;
            }
            free(pedido_atual);
            //printf("tempo : %s\n",time);
            return time;
        }
        pedido_anterior = pedido_atual;
        pedido_atual = pedido_atual->prox_pedido;
    }
    //printf("tempo null: %s\n",time);
    return time;
}

int main(int argc, char ** argv) {
    
    // Criação da lista de pedidos
    struct Pedidos *pedidos = NULL;

    int m = mkfifo("fifos/canal",0666);
    //printf("valor do makefifo %d\n",m);
    //printf("valor do errno %d\n",errno);
    // criar o qpipe para receber pedidos dos clientes
    if (m == -1) {
        if (errno == EEXIST) {
            unlink("fifos/canal");
            m = mkfifo("fifos/canal", 0666);
        }
    }

    int canal = open("fifos/canal", O_RDONLY | O_CREAT | O_TRUNC);
    //printf("valor do canal [%d]\n",canal);
    
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
            // Procurar até ao /n e guardar em string
            strcpy(token, strtok(buffer, "\n"));
            // Separação dos argumentos do pedido em string
            char ** pedido = malloc(40*sizeof(char *));
            int i = 0;
            char *token_aux = strtok(token," ");
            while (token_aux != NULL) {
                    //printf("Este é o token_aux: %s\n",token_aux);
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
            //printf("%s\n", estado);
            if(!strcmp(estado,"Running"))
            {

                char *pid = pedido[n-2];
                char *timestamp = pedido[n-1];
                int bytes_comandos = 0;
                for(int i = 1; i<n-2;i++) bytes_comandos+=sizeof(pedido[i]);
                char * str_comandos = malloc(bytes_comandos*sizeof(char));
                strcpy(str_comandos,pedido[1]);
                strcat(str_comandos," ");
                for(int i = 2; i<n-2;i++){
                    strcat(str_comandos,pedido[i]);
                    strcat(str_comandos," ");
                }
                //printf(" ----- Antes de invocar adicionar_pedido ----\n");
                pedidos = adicionar_pedido(pedidos,pid,str_comandos,timestamp,n-3);
                //imprimir_lista(*pedidos);
                free(str_comandos);
            }
            if(!strcmp(estado,"Ended")){
                //printf("OK ended------ \n");
                // A mensagem_final é a linha que se escreve em cada pid.txt com o seguinte formato:
                // PID COMANDOS TIME(duracao)
                int bytes_comandos = 0;
                for(int i = 1; i<n-2;i++) bytes_comandos+=sizeof(pedido[i]);
                char mensagem_final[n+bytes_comandos];
                char *txt = ".txt";
                char *pid = pedido[n-2];
                strcpy(mensagem_final,pid);
                strcat(mensagem_final," ");
                char *timestamp = pedido[n-1];
                for(int i = 1; i<n-2;i++){
                    strcat(mensagem_final,pedido[i]);
                    strcat(mensagem_final," ");
                }
                char *timestampinicial = remover_pedido(pedidos,pid);
                
                //printf("timestamp final: %s\n",timestamp);
                //printf("timestamp inicial: %s\n",timestampinicial);
                int duracao = atoi(timestamp) - atoi(timestampinicial);
                char *duracao_str = malloc(sizeof(char)*10);
                sprintf(duracao_str,"%d",duracao);
                strcat(mensagem_final,duracao_str);
                strcat(mensagem_final,"\0");
                int j = strlen(txt) + strlen(pid);
                char f[j];
                strcpy(f,pid);
                strcat(f,txt);
                //printf("nome do ficheiro file_pid: %s\n",f);
                // criar ficheiro com pid do processo como nome
                int file_pid = open(f,O_RDWR | O_CREAT | O_TRUNC, 0666);
                write(file_pid,mensagem_final,sizeof(mensagem_final));
                //printf("%s-%lu\n", mensagem_final, strlen(mensagem_final));
                write(file_pid,"\0",1);
                //printf("valor do file_pid: %d\n",file_pid);
                close(file_pid);
                
            }
            if(!strcmp(estado,"status")){
                //printf("OK status------ \n");
                char *pid = pedido[1];
                struct Pedidos *pedido_atual = pedidos;
                int canal_status = open(pid,O_CREAT | O_TRUNC | O_WRONLY,0666);

                //printf("PID para status: %s\n", pid);
                //printf("vamos enviar os pedidos todos ~ antes do while\n");

                while (pedido_atual != NULL) {
                    
                    struct timeval timestamp_at_moment;
                    gettimeofday(&timestamp_at_moment, NULL);
                    int mili_seconds_atmoment = timestamp_at_moment.tv_usec/1000;
                    //printf("timestamp_at_moment: %d\n",mili_seconds_atmoment);
                    //printf("pedido_atual->timeStamp: %s",pedido_atual->timeStamp);
                    int at_moment = mili_seconds_atmoment - atoi(pedido_atual->timeStamp);
                    
                    char mensagem_status[200];
                    strcpy(mensagem_status, pedido_atual->pid);
                    char resto[30];
                    sprintf(resto," %s %d ms\n",pedido_atual->comando, at_moment);
                    strcat(mensagem_status,resto);

                    //free(pedido_atual);
                    pedido_atual = pedido_atual->prox_pedido;
                    //printf("status mensagem ------> %s\n", mensagem_status);
                    write(canal_status,mensagem_status,sizeof(mensagem_status));
                    //free(mensagem_status);
                    
                }
                close(canal_status);
                //printf("enviado!!!!!!!!!!!!!");
            }
            if (!strcmp(estado,"close")) {
            running = false;
            char mensagem_close[10];
            strcpy(mensagem_close, "Closing\n");
            write(1,mensagem_close,sizeof(mensagem_close));
            sleep(3);
            break;
            }   
        }
        
        if (bytes_lidos < 0) {
            perror("Erro na leitura do pedido");
            continue;
        }
        
    }
    close(canal);
    unlink("fifos/canal");
    return 0;
}
