//
// Created by joel on 03.05.22.
//
#include "warp_server.h"
#include <stdio.h>

int stop = 10;
char *string = NULL;

int testquerycb(bool transferFormat, char *querystring){
    printf("Query Callback called with message:\n%s\n", querystring);
    string = calloc(3,1);
    return 0;
}

void testdatacb(void **data, size_t *length){
    if(stop <= 0) {
        *data = NULL;
        free(string);
        return;
    }
    snprintf(string,3, "%d", stop);
    *data = string;
    *length = strlen(*data);
    stop--;
}

int main(int argc, char **argv){
    warpserver_initialize(NULL, 13338, 0, testquerycb, testdatacb);
        warpserver_start();
        while(getchar() != '\n');
    warpserver_stop();
}