#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
//#include <wait.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include "monitor.h"

int inicio = 0;

int main(int argc, char ** argv){

    int heranca[2];
    if(pipe(heranca)==-1) perror("nao abriu o pipe heranca");
    char **comandos = malloc(sizeof(char*)*50);
    char * token;
    char *str = strdup(argv[3]);
    char *delim = "\"";
    token = strtok(str, delim);
    
    int count = 0;
    bool flag_sleep = false;
    
    char *new_token = strtok(str, " ");

    while (new_token != NULL) {

        // nao temos estado de sleep
        if(strcmp(new_token,"sleep")!=0 && !flag_sleep){
            comandos[count] = malloc(sizeof(new_token));
            strcpy(comandos[count], new_token);
            count++;
        }
        // vamos aplicar um sleep
        else if(strcmp(new_token,"sleep")!=0 && flag_sleep){
            comandos[count] = malloc(sizeof(new_token)+sizeof("sleep"));
            strcpy(comandos[count],"sleep ");
            strcat(comandos[count], new_token);
            flag_sleep = false;
            count++;
        }
        else if(strcmp(new_token,"sleep")==0){
            flag_sleep = true;
        }
        
        new_token = strtok(NULL, " ");
    }

    comandos[count] = NULL;
    count++;

    int canal = open("../fifos/canal", O_WRONLY,0666);
    
    char message[200];
    char message_runningCliente[20];
    char message_endedCliente[20];
    int tamanho_str = 0, pid, mili_seconds_inicial = 0, mili_seconds_final = 0;
    
    if(argc>1 && strcmp(argv[1], "status")==0){
            if((pid=fork())==0){  
                pid = getpid();
            
                // Mensagem que o cliente escreve para o utilizador
                // Mensagem format: status pid_valor
                tamanho_str = sprintf(message,"status %d\n",pid);
                write(1,message,tamanho_str);
                //printf("escrevemos aqui");
                // Mensagem que o cliente escreve para o servidor
                // Mensagem format: status pid_valor
                write(canal,message,tamanho_str);
                int d = dup(canal);
                //printf("valor do canal: %d\n",canal);
                //printf("estamos a escrever a mensagem\n");
                char pid_strg[200];
                sprintf(pid_strg,"../fifos/%d",pid);
                //printf("estamos a abrir o canal\n");

                int m = mkfifo(pid_strg,0666);
                int canal_status = open(pid_strg,O_RDONLY,0666);
                char buf[200];
                int bytes_lidos = 0;
                //printf("pid string : %s\n", pid_strg);
                //printf("valor canal status: %d\n", canal_status);
                //printf("estamos a imprimir os pedidos todos\n");
                while((bytes_lidos = read(canal_status,&buf,200*sizeof(char)))>0)
                {
                    //printf("%s\n", buf);
                    for (int i = 0; i < bytes_lidos; i++) {
                        write(1, &buf[i], 1);
                    if (buf[i] == '\n') {
                        break;
                        }
                    }
                }
                //printf("bytes lidos = %d\n");
                //printf("%s\n", buf);
                //printf("Devias ter recebido o pedido!!!!!!!!!!!!!!\n");
                close(canal);
                close(canal_status);
                _exit(0);
            }else{
                wait(NULL);
                close(canal);
            }
        }
    else{
         if(argc>1 && strcmp(argv[1], "close")==0){
            if((pid=fork())==0){  
                pid = getpid();
            
                // Mensagem que o cliente escreve para o utilizador
                // Mensagem format: Running PID valor_pid
                tamanho_str = sprintf(message,"close %d\n",pid);
                write(1,message,tamanho_str);
                
                // Mensagem que o cliente escreve para o servidor
                // Mensagem format: Running comands valor_pid valor_timestamp
                write(canal,message,tamanho_str);
                int d = dup(canal);
                
                char pid_strg[15];
                sprintf(pid_strg,"../fifos/%d",pid);
                
                int canal_status = open(pid_strg,O_RDONLY);
                char buf[2000];
                int bytes_lidos = 0;
                while((bytes_lidos = read(canal_status,&buf,2000*sizeof(char)))>0)
                {
                    write(1,buf,sizeof(bytes_lidos));
                }
                while((bytes_lidos = read(canal,&buf,2000*sizeof(char)))>0)
                {
                    write(1,buf,sizeof(bytes_lidos));
                }
                close(canal);
                _exit(0);
            }else{
                wait(NULL);
                close(canal);
            }
         }
         else{
        if((pid=fork())==0){
            pid = getpid();
            //Este é o filho
            //Extrai o timestamp do inicio antes da execução
            
            struct timeval timestamp_inicial;
            gettimeofday(&timestamp_inicial, NULL);
            mili_seconds_inicial = timestamp_inicial.tv_usec/1000;
            inicio = mili_seconds_inicial;
            close(heranca[0]);
            char inicio_str[10];
            sprintf(inicio_str,"%d",inicio);
            write(heranca[1],inicio_str,sizeof(inicio_str));

            strcpy(message,"Running ");
            strcpy(message_runningCliente,"Running PID ");
            for(int i = 0; i<count-1;i++){
                strcat(message,comandos[i]);
                strcat(message," ");
            
            }
            char resto[30];
            char message_runningCliente_resto[10];

            sprintf(resto,"%d %d", pid, mili_seconds_inicial);
            sprintf(message_runningCliente_resto,"%d \n", pid);
            
            strcat(message,resto);
            strcat(message,"\n");
            strcat(message_runningCliente,message_runningCliente_resto);
            
            // Mensagem que o cliente escreve para o utilizador
            // Mensagem format: Running PID valor_pid
            write(1,message_runningCliente,sizeof(message_runningCliente));

            // Mensagem que o cliente escreve para o servidor
            // Mensagem format: Running comandos valor_pid valor_timestamp
            write(canal,message,sizeof(message));

            //printf("Inicio da execução: %d\n", inicio);
            sleep(5);
            execvp(comandos[0],comandos);
            
        }
        else{
            //Este é o pai
            wait(NULL);
            // Extrai o timestamp do fim após a execução
            struct timeval timestamp_final;
            gettimeofday(&timestamp_final, NULL);
            mili_seconds_final = timestamp_final.tv_usec/1000;
            
            // Mensagem que o cliente escreve para o servidor a avisar que concluiu
            int b = sizeof(message);
            
            memset(message, 0, b);
            strcpy(message,"Ended ");
            strcpy(message_endedCliente,"Ended in ");
            close(heranca[1]);
            char inicio_str[10];
            read(heranca[0],&inicio_str,sizeof(inicio_str));
            close(heranca[0]);
            int inicio = atoi(inicio_str);
            
            int duracao = (mili_seconds_final - inicio);
            
            //printf("Fim da execução: %d\n", mili_seconds_final);
            //printf("Duração da execução: %d\n", duracao);
            for(int i = 0; i<count-1;i++){
                //printf("comandos[%d]: %s\n",i,comandos[i]);
                strcat(message,comandos[i]);
                strcat(message," ");
            }
            
            char resto[30];
            char message_endedCliente_resto[20];
            sprintf(resto,"%d %d", pid, mili_seconds_final);
            
            sprintf(message_endedCliente_resto,"%d", duracao);
            strcat(message,resto);
            strcat(message_endedCliente,message_endedCliente_resto);
            strcat(message,"\n");
            strcat(message_endedCliente," ms\n");
            
           //printf("Esta é a mensagem de fim da execução: %s\n",message);
            
            // Mensagem que o cliente escreve para o utilizador
            // Mensagem format: Ended in duracao
            write(1,message_endedCliente,sizeof(message_endedCliente));

            // Mensagem que o cliente escreve para o servidor
            // Mensagem format: Ended comandos valor_pid valor_timestamp
            write(canal,message,sizeof(message));

            while(count>0){
                free(comandos[count]);
                count--;
            }
            free(comandos);
            
        }
    }
    // caso n haja mais pedidos fecha cliente
    return 0;
}
}
