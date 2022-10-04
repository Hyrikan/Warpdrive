//
// Created by joel on 03.05.22.
//
#include "warp_client.h"
#include <arrow-glib/arrow-glib.h>

/*
 * This function is not working and was for testing purpose. The pg_standalone_warpserver test has a more complete version of arrow receive.
 */
void testArrowReceive(const guint8 tempBuf, gint64 tempBufSize){

    /* create arrowbuffer from tempbuf and read recordbatch*/
    printf("Assuming arrow format. Trying to convert...\n");

    GArrowBuffer *arrowbuffer = garrow_buffer_new(tempBuf, tempBufSize);
    GArrowBufferInputStream *inputStream = garrow_buffer_input_stream_new(GARROW_BUFFER(arrowbuffer));
    printf("Made inputstream...\n");
    GError *error = NULL;

    GArrowRecordBatchStreamReader *sr = garrow_record_batch_stream_reader_new(GARROW_INPUT_STREAM(inputStream), &error);
    if(sr == NULL){
        fprintf(stderr, "Failed to create stream reader! Error message: %s\n", error->message);
        g_error_free(error);
    }

    GArrowRecordBatch *rb = NULL;
    error = NULL;
    unsigned long rowcount = 0;
    unsigned int columncount = garrow_schema_n_fields(garrow_record_batch_reader_get_schema(GARROW_RECORD_BATCH_READER(sr)));
    printf("Iterating RecordBatches of tempBuf...\n");
    int iteration = 1;
    while(NULL != (rb = garrow_record_batch_reader_read_next(GARROW_RECORD_BATCH_READER(sr), &error)))
    {
        rowcount += garrow_record_batch_get_n_rows(rb);
        printf("Iteration %d | Rows: %lu\n", iteration, garrow_record_batch_get_n_rows(rb));
        g_object_unref(rb);
        iteration++;
    }
    if(error != NULL){
        printf("Failed to create record batch from stream... Error message: %s\n", error->message);
    }
    printf("=== RecordBatch:\nRow count: %lu\nColumn count: %d\n", rowcount,
           columncount);

    g_object_unref(arrowbuffer);
    g_object_unref(inputStream);
    g_object_unref(sr);

    printf("Got a NULL, did i get all the data?\n");
}

/*
 * This function receives plain text and prints it.
 */
void testPlainTextReceive(unsigned char *tempBuf, size_t tempBufSize){

}

/*
 * This function receives data from postgres warpserver and assumes binary pg data with value length before each value.
 * it also assumes the query SELECT * FROM pg1_sf1_region;
 */
void testPGReceiveRegion(unsigned char *tempBuf, size_t tempBufSize){
    printf("---------------------------------\n");
    printf("Buffer size: %lu\n", tempBufSize);
    int *nextValLength;
    int id;
    char *comment;
    size_t bytesRead = 0;
    while(bytesRead < tempBufSize){
        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        id = *((uint32_t *)(tempBuf+bytesRead+4));
        printf("    ID: %d\n", id);
        bytesRead += sizeof(int) + *nextValLength;
        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    Name: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;
        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    Len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    comment: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;
    }
    printf("---------------------------------\n");
}


/*
 * Converts a postgres numeric into a string representation
 */
char *getStrFromPGNumeric(u_int16_t *numvar){
    u_int16_t ndigits = ntohs(numvar[0]); // how many u_int16_t at numvar[4]
    int16_t dscale = ntohs(numvar[3]); // how many char digits after decimal point
    int16_t weight = ntohs(numvar[1])+1; // weight+1 is how many u_int16_t from numvar[4] are before decimal point. here weight already gets +1 at initialization.
    char *result = (char *)malloc(sizeof(char)*(weight+dscale)+1+1+2); // +1+1 -> '\0' and '.'
    char *copyStr = (char *) malloc(sizeof (char)*5);
    int strindex = 0;
    int numvarindex = 0;
    while(weight>0){
        sprintf(copyStr, "%d", ntohs(numvar[numvarindex+4]));
        sprintf(&(result[strindex]), "%s", copyStr);
        strindex += strlen(copyStr);
        numvarindex++;
        weight--;
    }
    sprintf(&(result[strindex]), ".");
    strindex++;
    while(dscale>0){
        sprintf(copyStr, "%d", ntohs(numvar[numvarindex+4]));
        dscale -= strlen(copyStr);
        sprintf(&(result[strindex]), "%s", copyStr);
        strindex += strlen(copyStr);
        numvarindex++;
    }
    result[strindex+dscale] = '\0';
    free(copyStr);
    return result;
}

/*
 * This function receives data from postgres warpserver and assumes binary pg data with value length before each value.
 * it also assumes the query SELECT * FROM pg1_sf1_lineitem LIMIT 10;
 */
void testPGReceiveLineitem(unsigned char *tempBuf, size_t tempBufSize){
    printf("---------------------------------\n");
    printf("Buffer size: %lu\n", tempBufSize);
    int *nextValLength;
    int32_t normalint;
    uint64_t bigint;
    char *comment;
    size_t bytesRead = 0;
    while(bytesRead < tempBufSize){
        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        bigint = __bswap_64( *((uint64_t *)(tempBuf+bytesRead+4)));
        printf("    l_orderkey: %lu\n", bigint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint = ntohl( *((int32_t *) (tempBuf+bytesRead+4)));
        printf("    l_partkey: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint = ntohl( *((int32_t *) (tempBuf+bytesRead+4)));
        printf("    l_suppkey: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint = ntohl( *((int32_t *) (tempBuf+bytesRead+4)));
        printf("    l_linenumber: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = getStrFromPGNumeric((u_int16_t *)(tempBuf+bytesRead+4));
        printf("    l_quantity: %s\n", comment);
        bytesRead += sizeof(int) + *nextValLength;
        free(comment);

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = getStrFromPGNumeric((u_int16_t *)(tempBuf+bytesRead+4));
        printf("    l_extendedprice: %s\n", comment);
        bytesRead += sizeof(int) + *nextValLength;
        free(comment);

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = getStrFromPGNumeric((u_int16_t *)(tempBuf+bytesRead+4));
        printf("    l_discount: %s\n", comment);
        bytesRead += sizeof(int) + *nextValLength;
        free(comment);

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = getStrFromPGNumeric((u_int16_t *)(tempBuf+bytesRead+4));
        printf("    l_tax: %s\n", comment);
        bytesRead += sizeof(int) + *nextValLength;
        free(comment);

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_returnflag: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_linestatus: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint = ntohl( *((int32_t *) (tempBuf+bytesRead+4)));
        printf("    l_shipdate: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint = ntohl( *((int32_t *) (tempBuf+bytesRead+4)));
        printf("    l_commitdate: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint = ntohl( *((int32_t *) (tempBuf+bytesRead+4)));
        printf("    l_receiptdate: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_shipinstruct: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_shipmode: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_comment: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;
    }
    printf("---------------------------------\n");
}


/*
 * This function receives data from postgres warpserver and assumes binary pg data with value length before each value.
 * it also assumes the query SELECT * FROM pg1_sf1_lineitem LIMIT 10;
 */
void testPGReceiveLineitemReduced(unsigned char *tempBuf, size_t tempBufSize){
    printf("---------------------------------\n");
    printf("Buffer size: %lu\n", tempBufSize);
    int *nextValLength;
    int32_t normalint;
    uint64_t bigint;
    char *comment;
    size_t bytesRead = 0;
    while(bytesRead < tempBufSize){
        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        bigint = __bswap_64( *((uint64_t *)(tempBuf+bytesRead+4)));
        printf("    l_orderkey: %lu\n", bigint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint = ntohl( *((int32_t *) (tempBuf+bytesRead+4)));
        printf("    l_partkey: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint = ntohl( *((int32_t *) (tempBuf+bytesRead+4)));
        printf("    l_suppkey: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint = ntohl( *((int32_t *) (tempBuf+bytesRead+4)));
        printf("    l_linenumber: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_returnflag: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_linestatus: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_shipinstruct: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_shipmode: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_comment: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;
    }
    printf("---------------------------------\n");
}
/*
 * This function receives data from clickhouse warpserver and assumes value length before each value.
 * it also assumes the query does not select decimals or dates.
 */
void testClickReceiveLineitem(unsigned char *tempBuf, size_t tempBufSize){
    printf("---------------------------------\n");
    printf("Buffer size: %lu\n", tempBufSize);
    int *nextValLength;
    int32_t normalint;
    uint64_t bigint;
    char *comment;
    size_t bytesRead = 0;
    while(bytesRead < tempBufSize){
        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        bigint = *((uint64_t *)(tempBuf+bytesRead+4));
        printf("    l_orderkey: %lu\n", bigint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint =  *((int32_t *) (tempBuf+bytesRead+4));
        printf("    l_partkey: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint =  *((int32_t *) (tempBuf+bytesRead+4));
        printf("    l_suppkey: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        normalint =  *((int32_t *) (tempBuf+bytesRead+4));
        printf("    l_linenumber: %d\n", normalint);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_returnflag: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_linestatus: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_shipinstruct: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_shipmode: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;

        nextValLength = (int *) (tempBuf+bytesRead);
        printf("    len: %d\n", *nextValLength);
        comment = tempBuf+bytesRead+4;
        printf("    l_comment: %.*s\n", *nextValLength, comment);
        bytesRead += sizeof(int) + *nextValLength;
    }
    printf("---------------------------------\n");
}

int main(int argc, char **argv){
    int status = 0;
    void *data;
    char *testquery;
    if(argc < 2){
        printf("Too few arguments! ./test_warp_client <port> [query]\n");
        exit(1);
    }else if(argc == 3){
        testquery = argv[2];
    }
    else{
        testquery = "SELECT * FROM *;";
    }
    int port = atoi(argv[1]);
    printf("Starting client test... querying port %d (hopefully)\n", port);

    /* Send foreign query */
    status = warpclient_queryServer(NULL, port, 0, true,testquery);
    if(status != 0){
        printf("Failed to query server.\n");
        return status;
    }

    size_t length = 0;
    printf("Putting all data in one buffer...\n");
    /* receive all the data */
    size_t tempBufSize = DEFAULT_BUFFER_SIZE;
    unsigned char *tempBuf = malloc(DEFAULT_BUFFER_SIZE);
    size_t currentTempBufPosition = 0;
    while(NULL != (data = warpclient_getData(&length))){
        if((currentTempBufPosition + length) > tempBufSize){
            tempBufSize *= 2;
            tempBuf = realloc(tempBuf, tempBufSize);
            if(tempBuf == NULL){
                fprintf(stderr,"Failed to realloc buffer!\n");
                /* Call cleanup */
                status = warpclient_cleanup();
                free(tempBuf);
                return status;
            }
        }
        memcpy(tempBuf+currentTempBufPosition,data, length);
        //fprintf(stderr, "Copied %lu bytes into tempBuf...\n", length);
        currentTempBufPosition += length;
        printf("Sum of bytes received: %ld.\n", currentTempBufPosition);
    }
    if(currentTempBufPosition != 0) {
        //testArrowReceive(tempBuf, tempBufSize);
        //testPGReceiveLineitem(tempBuf, currentTempBufPosition);
        testPGReceiveLineitemReduced(tempBuf, currentTempBufPosition);
        //testPGReceiveRegion(tempBuf, currentTempBufPosition);
        //testPlainTextReceive(tempBuf, currentTempBufPosition);
        //testClickReceiveLineitem(tempBuf, currentTempBufPosition);

        //printf("data: \n%.*s\n", currentTempBufPosition, (char *) tempBuf);
    }else {
        printf("Timed out!\n");
    }
    /* Call cleanup */
    status = warpclient_cleanup();
    free(tempBuf);

    return status;
}