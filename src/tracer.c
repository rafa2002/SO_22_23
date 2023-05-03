#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <time.h>
#include "monitor.h"

/*
int main(int argc, char ** argv){

typedef struct pedido{
  char * pid;
  char * nomeProg;
  char * inicioExec  // int em ms?
  char * fimExec   // int em ms?
  char ** args;
  struct pedido * prox;
} *pedidos;

//cria e adiciona pedido no início
int addPedido (pedidos *pp, char *pid, int pr, int cmds[7],char **args){
  pedidos p = malloc(sizeof(struct pedido));
  p->pid = pid;
  p->priority = pr;
  p->args = args;
  for (int i=0; i<7; i++) p->cmds[i]=cmds[i];
  p->prox = (*pp);
  (*pp) = p;
  return 0;
}

//adiciona pedido no inicio
int addPedido2 (pedidos *pp, pedidos p){
  p->prox = (*pp);
  (*pp) = p;
  return 0;
}

// free do pedido
void freePedido(pedidos p){
  free(p->pid);
  for (int i =0; p->args[i]; i++) free(p->args[i]);
  free(p->args);
  free(p);
}

 */


int main(int argc, char ** argv){
    
    printf("O que o utilizador mandou executar - [%s]\n",*argv);
    
    printf("argv[0] = %s,\n,argv[1] = %s\nargv[2] = %s",argv[0],argv[1],argv[2]);
    
    char *arg = argv[3];
    int count = 0;
    char * token;
    char **comandos = malloc(sizeof(char*)*256);
    if(arg!=NULL)
    {
        token = strtok(arg," ");
        while(token!=NULL){
            comandos[count] = malloc(sizeof(char)*strlen(token)+1);
            comandos[count] = token;
            count++;
            printf("O token é :%s",token);
            token = strtok(NULL," ");
            
        }
    }
    
    int canal = open("canal", O_WRONLY, 0600);
    char * message = malloc(sizeof(int) + sizeof(char)*50 + sizeof(time_t));
    int tamanho_str = 0;
    
    int pid;
    
    //se a operação a efetuar é o status
    if(strcmp(argv[1], "status")==0){
        if((pid=fork())==0){
            
            //abrir o pipe para comunicar com o servidor e pedir os programas a serem executados
            //if ((canal = open(buffer,O_WRONLY))==-1) perror ("open error Cliente->Servidor");
            
            //pede_status();
            
            close(canal);
            _exit(0);
        }
        close(canal);
    }
    else{
        //caso não haja espaço adiciona se à lista de pedidos
        // se há recursos para realizar o comando
        if((pid=fork())==0){
            //Este é o filho
            //Extrai o timestamp do inicio antes da execução
            time_t t = time(NULL);
            struct tm *timestamp = localtime(&t);
            // Faço uma copia da string que quero escrever
            char* copy = malloc(sizeof(char)*tamanho_str);
            strcpy(copy, tamanho_str);
            
            // Mensagem que o cliente escreve para o utilizador
            // Mensagem format: Running PID valor_pid valor_hora
            tamanho_str = sprintf(message,"Running PID %d\n", pid);
            write(1,canal,tamanho_str);
            
            // Mensagem que o cliente escreve para o servidor
            // tamanho_str = sprintf(message, ... ,pid,nome_do_programa,timestamp(ms));
            tamanho_str = sprintf(message,"%d %Y-%m-%d %H:%M:%S",pid,timestamp);
            write(canal,copy,tamanho_str);
            
            //Este printf é para imprimir no stdout o comando a executar informando assim o user
            printf("PID[%d] para executar: ",pid);
            for(int c = 1; c<strlen(comandos);c++){
                printf("%s ",comandos[c]);
                printf("\n");
            }
            
            execvp(comandos[0],comandos);
            
        }
        
        else{
            //Este é o pai
            
            // Extrai o timestamp do fim após a execução
            time_t tf = time(NULL);
            struct tm *timestampfinal = localtime(&tf);
            
            // Mensagem que o cliente escreve para o servidor a avisar que concluiu
            // tamanho_str = sprintf(message, ... ,pid,nome_do_programa,timestamp(ms));
            tamanho_str = sprintf(message,"%d %Y-%m-%d %H:%M:%S",pid,timestampfinal);
            write(canal,message,tamanho_str);
            while(count>=0){
                free(comandos[count]);
                count--;
            }
            free(comandos);
            wait(NULL);
        }
    }


    // caso n haja mais pedidos fecha servidor
    
    return 0;
}
