#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>
#include <io.h>
// #include <unistd.h>
// #include <wait.h>
#include <limits.h>
#include <time.h>
#include "tracer.h"

void armazenar_info_processo(int pid, char ** comando, struct tm *timestamp){
    int log = open("log.txt",O_WRONLY | O_CREAT | O_TRUNC,0660);
    char *buf = malloc(36 * sizeof(char));
    char tempo[20];
    strftime(tempo,strlen(tempo),"%Y-%m-%d %H:%M:%S",timestamp);
    sprintf(buf,"Data formatada: %s\n",tempo);
    write(log,&buf,strlen(buf));
}