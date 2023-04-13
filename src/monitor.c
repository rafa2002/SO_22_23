#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <time.h>
#include "client.h"

void armazenar_info_processo(int pid, char ** comando, struct tm *timestamp){
    int log = open("log.txt",O_WRONLY | O_CREAT | O_TRUNC,0660);
    char *buf;
    char tempo[20];
    strftime(tempo,sizeof(tempo),"%Y-%m-%d %H:%M:%S",timestamp);
    printf("Data formatada: %s\n",tempo);
    write(log,&buf,sizeof(buf));
}