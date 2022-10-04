//
// Created by joel on 10.05.22.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// is that a callback declaration? YES!
typedef int (*callback)(char *msg);

// can i store the function here? YES!
callback mycb;

void getValues(void *pointer){
    int a = 10;
    int b = 54321;
    *(int*)pointer = a;
    int * temp = (int *) pointer;
    temp[1] = b;
    printf("Inside of function: %d, %d\n", *((int *)pointer), *(temp+1));
}


int myCb(char *msg){
    printf("Callback called! Message: %s\n", msg);
    return strlen(msg);
}

void cooleInitFunktion(callback cb){
    mycb = cb;
}

void testCB(char * msg) {
    printf("Length of previous message: %d\n", mycb(msg));
}



void testFiles(){

    // What exactly gets written with FILE, fwrite ...
    FILE *fhandle = fopen("/home/joel/bachelorarbeit/Bachelorarbeit/Code/UCP/test_field_ucp/randomoutput.txt", "w");
    if(fhandle == NULL) {
        fprintf(stderr, "opening file failed.\n");
        exit(1);
    }
    char buf1[] = "Nachricht Nummer 1.\n";
    char buf2[] = "Eine weitere Nachricht.\n";
    int written = fwrite(&buf1, 1,sizeof(buf1)-1, fhandle);
    printf("%d bytes written.\n", written);

    written = fwrite(&buf2, 1,sizeof(buf2)-1, fhandle);
    printf("%d bytes written.\n", written);


    fclose(fhandle);
}


void callByReferenceTest(){
    // Some call by reference tests
    void *test = malloc(sizeof(int)*2);
    getValues(test);
    printf("Outside of function: %d, %d\n", *((int*)test), *((int*)test+1));

    int c = *((int *)test+1);
    printf("Copied again: %d\n", c);
    *((int*)test+1) = 56;
    printf("Copied again again: %d\n", c);
    printf("Inside pointer: %d\n", *((int*)test+1));

    printf("Size of int: %ld\n", sizeof (int));

}


int testquerycb(char *querystring){
    printf("Query Callback called with message:\n%s\n", querystring);
    return 0;
}

// void ** is needed, otherwise one could not overwrite the address of a void * pointer.
// if *data gets a static string assigned, it is not freeable!
void testdatacb(void **data, size_t *length){
    void *test = "Data callback result.";
    *length = strlen(test)+1;
    //*data = malloc(*length);
    *data = test;
    //memcpy(*data, test, *length);
    printf("%s\n", *data);
}

void functionCallbackTest(){

    //some callback function tests
    cooleInitFunktion(myCb);
    testCB("So ne geile Sache!");


    testquerycb("Nachricht, jaja!");
    void *data = "hallo";
    size_t length;
    // important to give address to void *! so that there can be a new address to viable memory.
    testdatacb(&data, &length);
    printf("Data callback: %s\n", data);


    //free(data); // only free data if it got malloced in callback
}

void stringPrintTest(){
    char *string1 = (char *) calloc(30, sizeof(char));
    memcpy(string1, "test", 4 );
    string1[4] = '\0';
    printf("Teste den String: %s\n", string1);
    memcpy(string1+5, "Ein schöner Tag", 8);
    string1[13] = '\0';
    printf("Teste stop bei \\0: %s\n", string1);
    printf("Teste weiter als \\0: %.*s\n", 10, string1);

}

void testCommandLineArgv(int argc, char**argv){
    printf("----------------------  Test argv ------------------\n");
    int len = 0;
    for(int i=0; i < argc; i++){
        len += strlen(argv[i])+1;
        printf("%.*s", len, argv[i]);
    }
}


void testreadBitsFromVoid(){
    printf("----------------------------- Test read bits -----------------------------\n");
    void *test = "Hallo, Welt!\n";
    void *buf = malloc(10);
    printf("Read one byte in hexa from void pointer: %02x\n", ((unsigned char *) test)[2]); // converts third byte into readable ascii hexa. l is 6c in hexa ascii, which is 108 in dezimal.
    unsigned char hexa = 0x40;
    *(unsigned char *)(buf+1) = hexa;
    printf("Copy one byte into void pointer: %02x\n", ((unsigned char *)buf)[1]);
    // was it a real copy by value or was it only by reference?
    hexa = 0x60;
    printf("Copy one byte into void pointer: %02x\n", ((unsigned char *)buf)[1]);
    u_int16_t zahl = 259;
    u_int nochmehrdahinter = 999999999;
    *(u_int16_t *)(buf+2) = zahl;
    *(u_int *)(buf+4) = nochmehrdahinter;
    u_int16_t zahl2 = *(u_int16_t *)(buf+2);
    printf("Copy from u_int16 into void pointer: %u", zahl2);
    free(buf);
}

void testSomeArithmeticWithSizeOf(){
    printf("----------------------------- Test sizeof arithmetic -----------------------------\n");
    int a = 4;
    int b = a+sizeof(int);
    printf("Integer: %d, now with added sizeof(int): %d \n", a, b);
}

void testIncrements(){
    printf("----------------------------- Test increments of i++ -----------------------------\n");
    int i = 0;
    int a[] = {1,2,3,4,5};
    printf("a[i] = %d\n", a[i]);
    printf("a[i++] = %d\n", a[i++]);
    printf("a[i++] = %d\n", a[i++]);
    printf("a[i] = %d\n", a[i]);
}

int main(){
    testFiles();

    callByReferenceTest();

    functionCallbackTest();

    stringPrintTest();

    int argc = 2;
    char *arg1 = "./random";
    char *arg2 = "--help";
    char **argv = malloc(sizeof(char*) *2);
    argv[0] = arg1;
    argv[1] = arg2;
    testCommandLineArgv(argc, argv);


    testreadBitsFromVoid();

    testSomeArithmeticWithSizeOf();

    testIncrements();

    exit(0);
}











