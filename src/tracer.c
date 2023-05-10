#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include "monitor.h"

int inicio = 0;

int main(int argc, char ** argv){
    
/*    printf("O que o utilizador mandou executar - [");
    for(int count =0 ; count<argc;count++){
        printf(" %s",argv[count]);
    }
    printf("]\n");
*/	
    int heranca[2];
    if(pipe(heranca)==-1) perror("nao abriu o pipe heranca");
    char **comandos = malloc(sizeof(char*)*50);
    char * token;
    char *str = strdup(argv[3]);
    char *delim = "\"";
    token = strtok(str, delim);
//    printf("token das aspas: %s\n", token);
    
    int count = 0;
    bool flag_sleep = false;
    
    char *new_token = strtok(str, " ");
    while (new_token != NULL) {
//        printf("token: %s\n", new_token);
        // nao temo estado de sleep
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
/*
    for(int i = 0; i<count;i++){
        printf("valoreeeeee  i:: %d\n",i);
        printf("comandos[%d]: %s\n",i,comandos[i]);
        
    }
*/
    int canal = open("canal", O_WRONLY,0666);
    
//    printf("- - - - - - - - - 999999 - - - - - - - - -  \n");
    char message[200];
    char message_runningCliente[20];
    char message_endedCliente[20];
//    printf("- - - - - - - - - 555555 - - - - - - - - -  \n");
    int tamanho_str = 0, pid, mili_seconds_inicial = 0, mili_seconds_final = 0;
//    printf("- - - - - - - - - 00000 - - - - - - - - -  \n");
    
    if(argc>1 && strcmp(argv[1], "status")==0){
            if((pid=fork())==0){  
                pid = getpid();
            
                // Mensagem format: Running PID valor_pid valor_timestamp
                // Mensagem que o cliente escreve para o utilizador
                tamanho_str = sprintf(message,"status %d\n",pid);
                write(1,message,tamanho_str);
                
                // Mensagem que o cliente escreve para o servidor
                write(canal,message,tamanho_str);
                int d = dup(canal);
//                printf("valor do canal --- %d\n",d);
//                printf("PID[%d] para executar: ",pid);
                
                char pid_strg[15];
                // ler a resposta
                sprintf(pid_strg,"%d",pid);
                
                int canal_status = open(pid_strg,O_RDONLY);
                char buf[2000];
                int bytes_lidos = 0;
                while((bytes_lidos = read(canal_status,&buf,2000*sizeof(char)))>0)
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
            // Mensagem format: Running PID valor_pid valor_timestamp
            // Mensagem que o cliente escreve para o utilizador
            strcpy(message,"Running ");
            strcpy(message_runningCliente,"Running PID ");
            for(int i = 0; i<count-1;i++){
//                printf("Este é o valor para iiiiii :: %d\n",i);
//                printf("comandos[%d]: %s\n",i,comandos[i]);
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
            
            write(1,message_runningCliente,sizeof(message_runningCliente));
            // Mensagem que o cliente escreve para o servidor
            write(canal,message,sizeof(message));
            
            int d = dup(canal);
//            printf("valor do canal --- %d\n",d);
//            printf("PID[%d] para executar: \n",pid);
            
            // FUNCIONALIDADE AVANÇADA ~
            // Separa os comandos por " | " e depois mete los em comandos
            // Para cada comando fzr fork e cada filho trata do comando
            // enquanto que o filho que recebeu o pedido múltiplo fica à espera
            
            printf("Inicio da execução: %d\n", inicio);
            execvp(comandos[0],comandos);
            
//            printf("Comandos executados!\n");
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
            int inicio = atoi(inicio_str);
            
            int duracao = (mili_seconds_final - inicio);
            
            printf("Fim da execução: %d\n", mili_seconds_final);
            printf("Duração da execução: %d\n", duracao);
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
            
           printf("Esta é a mensagem de fim da execução: %s\n",message);
            
            write(1,message_endedCliente,sizeof(message_endedCliente));
            write(canal,message,tamanho_str);
//            printf("Vamos agr fazer o free dos comandos\n");
            while(count>0){
                free(comandos[count]);
                count--;
            }
            free(comandos);
            
//            printf("ehehe acabamos\n");
        }
    }
    // caso n haja mais pedidos fecha cliente
    return 0;
}
