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
#define MAX_COMMAND 256

int running = true;

int main(int argc, char **argv) {

    // criar o qpipe para receber pedidos dos clientes
    if (mkfifo("canal", 0600) == -1 && errno != EEXIST) {
        perror("Erro ao criar o canal nomeado");
        //exit(1);
    }

    int canal = open("canal", O_RDONLY | O_WRONLY, 0600);
    
    if (canal == -1) {
        perror("Erro ao abrir o canal nomeado");
        exit(1);
    }
    
    //ir lendo os pedidos enviados por clientes
    char buffer[MAX_COMMAND] = "";
    
    while (running) {
        int bytes_lidos = 0;
        char byte_lido;
        while((bytes_lidos += read(canal,&byte_lido, 1))){
            // Se o byte lido for o caractere nulo, processa a mensagem
            if (byte_lido == '\0') {
                printf("Mensagem recebida: %s\n", buffer);
                strncat(buffer, &byte_lido, 1);
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
            write(canal,"NO MORE REQUESTS\n",sizeof("NO MORE REQUESTS\n"));
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
                
                // executa o comando
                execvp(info[0], info);
                perror("Erro ao executar o comando");
                
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

    close(canal);
    unlink("canal");

    return 0;
}


/*
void armazenar_info_processo(int pid, char ** comando, struct tm *timestamp){
    int log = open("log.txt",O_WRONLY | O_CREAT | O_TRUNC,0660);
    char *buf = malloc(36 * sizeof(char));
    char tempo[20];
    strftime(tempo,strlen(tempo),"%Y-%m-%d %H:%M:%S",timestamp);
    sprintf(buf,"Data formatada: %s\n",tempo);
    write(log,&buf,strlen(buf));
}


int main(){
    int comunicacao_cliente_servidor = mkfifo("comunicacao.txt");

}*/
