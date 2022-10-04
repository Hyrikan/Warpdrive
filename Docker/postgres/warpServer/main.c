#include "pg_warpserver.h"



int pg_warpServer_querycb(bool transferFormat, char *querystring);
void pg_warpServer_datacb(void **data, size_t *length);

/*
 * Helper functions
 */
static void exit_nicely(int code);
static void show_binary_results(PGresult *res);
static int pg_warpServer_pgconnect(void);
static PGresult *pgsql_begin_query(const char *query);

/*
 * Variables
 */
#define CURSOR_NAME		"curr_pg2arrow2"
PGconn     *conn;
PGresult   *warpres;
int numberTuples;
int tuplesused;
int numberCols;
GArrowSchema *currentSchema;
int queryOngoing = 0;
GArrowRecordBatch *currentRecordBatch;
GArrowRecordBatchStreamWriter *streamWriter = NULL;
GArrowBufferOutputStream *bufferStream = NULL;
GArrowResizableBuffer *buffer = NULL;
size_t alreadySend = 0;
bool rowFormat = false;

static size_t	batch_segment_sz = 0;
static char	   *pgsql_hostname = NULL;
static char	   *pgsql_portno = NULL;
static char	   *pgsql_username = NULL;
static char	   *pgsql_database = NULL;
static char	   *pgsql_password = NULL;

/**
 * First, initializes connection to the local postgres server.
 * Second, initializes warp server and starts listening for incoming queries.
 * Third, wait for user input '\n' to stop the warp server.
 * Fourth, releases resources.
 * @param argc
 * @param argv Reads local server listen address, port and Ipv6 flag from command line.
 * @return Error codes
 */
int main(int argc, char **argv) {
    int ret;
    PGresult   *res;
    // TODO Handle command ops later
    pgsql_database = "db1";
    pgsql_hostname = "localhost";
    pgsql_portno = "5432";
    pgsql_username = "postgres";
    pgsql_password = "post1234";//"iu7trdu8OpOIfud6";

    /* Make a connection to the database */
    ret = pg_warpServer_pgconnect();
    if(ret != 0){
        elog_debug("Failed to connect to local postgresql instance!\n");
        return 1;
    }

    int myserverport = 13337;


    ret = warpserver_initialize(NULL, myserverport, 0, pg_warpServer_querycb, pg_warpServer_datacb);
    if(ret != 0){
        elog_debug("Failed to initialize warp server! Error code: %d\n", ret);
        exit_nicely(ret);
    }
    elog_debug("Warpserver initialization successful!\n");

    ret = warpserver_start();
    if(ret != 0){
        elog_debug("Failed to start warp server! Error code: %d\n", ret);
        exit_nicely(ret);
    }
    elog_debug("Starting warp server successful!\n");
    printf("Server started! (Press Enter to stop the server)");

    while(getchar() != '\n');

    exit_nicely(0);
    printf("Server stopped!");

    return 0;
}


/*
 * pgsql_next_result
 */
static PGresult *
pgsql_next_result()
{
    PGresult   *res;
    /* fetch results per half million rows */
    res = PQexecParams(conn,
                       "FETCH FORWARD 52000 FROM " CURSOR_NAME,
            0, NULL, NULL, NULL, NULL,
            1);	/* results in binary mode */
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        elog_debug("SQL execution failed: %s", PQresultErrorMessage(res));
    if (PQntuples(res) == 0)
    {
        PQclear(res);
        return NULL;
    }
    return res;
}

/*
 * pgsql_begin_query
 */
static PGresult *
pgsql_begin_query(const char *query)
{
    PGresult   *res;
    char	   *buffer;

    /* set transaction read-only */
    res = PQexec(conn, "BEGIN READ ONLY");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        elog_debug("unable to begin transaction: %s", PQresultErrorMessage(res));
    PQclear(res);

    /* declare cursor */
    buffer = malloc(strlen(query) + 2048);
    sprintf(buffer, "DECLARE %s BINARY CURSOR FOR %s", CURSOR_NAME, query);
    res = PQexec(conn, buffer);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        elog_debug("unable to declare a SQL cursor: %s", PQresultErrorMessage(res));
    PQclear(res);
    free(buffer);

    return pgsql_next_result();
}

/*
 * Initializes connection to local pgserver
 */
static int pg_warpServer_pgconnect(void)
{
    const char *keys[20];
    const char *values[20];
    int			index = 0;
    int			status;

    if (pgsql_hostname)
    {
        keys[index] = "host";
        values[index] = pgsql_hostname;
        index++;
    }
    if (pgsql_portno)
    {
        keys[index] = "port";
        values[index] = pgsql_portno;
        index++;
    }
    if (pgsql_database)
    {
        keys[index] = "dbname";
        values[index] = pgsql_database;
        index++;
    }
    if (pgsql_username)
    {
        keys[index] = "user";
        values[index] = pgsql_username;
        index++;
    }
    if (pgsql_password)
    {
        keys[index] = "password";
        values[index] = pgsql_password;
        index++;
    }
    keys[index] = "application_name";
    values[index] = "Warpdrive_Server";
    index++;
    /* terminal */
    keys[index] = NULL;
    values[index] = NULL;

    conn = PQconnectdbParams(keys, values, 0);
    if (!conn){
        elog_debug("connection failed\n");
        return -1;
    }
    status = PQstatus(conn);
    if (status != CONNECTION_OK){
        elog_debug("failed on PostgreSQL connection: %s\n",
             PQerrorMessage(conn));
        return -1;
    }

    return 0;
}

/*
 * Converts PG Datatype as id into corresponding Arrow datatype
 */
GArrowDataType *getArrowDatatypeFromOID(const char *colname, Oid pgDatatypeID){
    GArrowDataType *result;
    if(pgDatatypeID == 1700){
        elog_debug("Got a Decimal datatype!\n");
        PGresult *tempres;
        const char *paramValues[2];
        char *tableoid = (char *) malloc(sizeof(char)*10);
        sprintf(tableoid, "%d", PQftable(warpres, 0));
        paramValues[0] = tableoid;
        tempres = PQexecParams(conn, "SELECT relname FROM pg_class WHERE oid = $1", 1, NULL, paramValues, NULL, NULL, 0);
        if (PQresultStatus(tempres) != PGRES_TUPLES_OK) {
            elog_debug("unable to begin transaction: %s", PQresultErrorMessage(tempres));
            exit_nicely(2);
        }
        paramValues[0] = PQgetvalue(tempres, 0, 0);
        paramValues[1] = colname;
        PQclear(tempres);
        tempres = PQexecParams(conn, "SELECT (atttypmod - 4) >> 16 & 65535 AS precision,\n"
                                                  "       (atttypmod - 4) & 65535 AS scale\n"
                                                  "FROM pg_attribute\n"
                                                  "WHERE attrelid = $1::regclass\n"
                                                  "  AND attname = $2;", 2, NULL, paramValues, NULL, NULL, 0);
        if (PQresultStatus(tempres) != PGRES_TUPLES_OK) {
            elog_debug("unable to begin transaction: %s", PQresultErrorMessage(tempres));
            exit_nicely(3);
        }
        int precision = atoi(PQgetvalue(tempres, 0,0));
        int scale = atoi(PQgetvalue(tempres, 0, 1));
        elog_debug("Precision: %d\nScale: %d\n", precision, scale);
        result = (GArrowDataType *) garrow_decimal_data_type_new(precision, scale, NULL);
        if(result == NULL) elog_debug("garrow_type creation failed!\n");
        PQclear(tempres);
        return result;
    } //handle Numerics seperately

    switch(pgDatatypeID){
        case 20: // pg int8 (bigint)
            elog_debug("Got a Long datatype!\n");
            result = (GArrowDataType *) garrow_int64_data_type_new();
            if(result == NULL) elog_debug("garrow_type creation failed!\n");
            return result;
        case 23: // pg int4 (integer)
            elog_debug("Got an Integer datatype!\n");
            result = (GArrowDataType *) garrow_int32_data_type_new();
            if(result == NULL) elog_debug("garrow_type creation failed!\n");
            return result;
        case 25: // pg text (probably char)
            elog_debug("Got a Text datatype!\n");
            result = (GArrowDataType *) garrow_string_data_type_new();
            if(result == NULL) elog_debug("garrow_type creation failed!\n");
            return result;
        case 1082: //pg date
            elog_debug("Got a Date datatype!\n");
            result = (GArrowDataType *) garrow_date32_data_type_new();
            if(result == NULL) elog_debug("garrow_type creation failed!\n");
            return result;
        case 1043: //varchar
        case 1042: //bpchar
            elog_debug("Got a Varchar/bgchar datatype!\n");
            result = (GArrowDataType *) garrow_string_data_type_new();
            if(result == NULL) elog_debug("garrow_type creation failed!\n");
            return result;
        default: // TODO some "basic" type available? probably text?...
            elog_debug("Got an unknown datatype! pgDatatypeID: %d\n", pgDatatypeID);
            return NULL;
    }
}


/*
 * Callback function for querying the postgres database for arrow data
 */
int pg_warpServer_querycb_arrow(bool transferFormat, char *querystring){
    if(queryOngoing) return -1;
    elog_debug("Query Callback called with message:\n%s\n", querystring);

    /* run SQL command and get first result batch*/
    warpres = pgsql_begin_query(querystring);
    if (!warpres) elog_debug("SQL command returned an empty result");

    /* read number of rows */
    numberTuples = PQntuples(warpres);
    elog_debug("Number Tuples: %d \n", numberTuples);

    /* read number of columns */
    numberCols = PQnfields(warpres);
    elog_debug("Number Columns: %d \n", numberCols);

    /* create schema: read column names and data types and write them as fields into schema */
    GList *fields = NULL;
    for(int i = 0; i < numberCols; i++){
        char *colname = PQfname(warpres, i);
        elog_debug("Column name %d: %s \n", i, colname);
        GArrowField *nextField = garrow_field_new(colname, getArrowDatatypeFromOID(colname,PQftype(warpres, i)));
        fields = g_list_append(fields, nextField);
    }
    currentSchema = garrow_schema_new(fields);
    if(currentSchema == NULL){
        elog_debug("Schema creation failed!\n");
    }else elog_debug("Created Schema: %s\n", garrow_schema_to_string(currentSchema));

    /* Create data buffer and streamwriter for record batches*/
    GError *error = NULL;
    buffer = garrow_resizable_buffer_new(4096, &error);
    if(buffer == NULL){
        elog_debug("Failed to initialize resizable buffer! Error message: %s\n", error->message);
        g_error_free(error);
    }
    bufferStream = garrow_buffer_output_stream_new(buffer);
    streamWriter = garrow_record_batch_stream_writer_new(GARROW_OUTPUT_STREAM(bufferStream), currentSchema, &error);
    if(streamWriter == NULL){
        elog_debug("Failed to create Record batch stream writer! Error message: %s\n", error->message);
        g_error_free(error);
    }

    alreadySend = 0;

    return 0;
}


/*
 * Callbac function for querying the postgres database (pg-click test version)
 */
int pg_warpServer_querycb(bool transferFormat, char *querystring){
    if(queryOngoing) return -1; else queryOngoing = 1;
    elog_debug("Query Callback called with query:\n%s\nRequested transferformat: %s\n", querystring, transferFormat ? "row" : "column");

    //save transferFormat
    rowFormat = transferFormat;

    /* run SQL command and get first result batch*/
    warpres = pgsql_begin_query(querystring);
    if (!warpres) elog_debug("SQL command returned an empty result");

    /* read number of rows */
    numberTuples = PQntuples(warpres);
    tuplesused = 0;
    elog_debug("Number Tuples: %d \n", numberTuples);

    /* read number of columns */
    numberCols = PQnfields(warpres);
    elog_debug("Number Columns: %d \n", numberCols);

    alreadySend = 0;

    return 0;
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
 * Consumes PGresult buffer and builds record batch
 * 1. make record batch builder
 * 2. make list of array builders for the columns
 * 3. iterate rows from warpres until warpres is NULL and convert these rows into append values on the array builders.
 * 4. call flush on record batch builder
 */
GArrowRecordBatch *buildRecordBatch(PGresult *res){
    if(res == NULL) return NULL;
    GArrowRecordBatch *resultBatch = NULL;

    // 1.
    GError *error = NULL;
    GArrowRecordBatchBuilder *builder = garrow_record_batch_builder_new(currentSchema, &error);
    garrow_record_batch_builder_set_initial_capacity(builder, numberTuples);

    // 2.
    GArrowArrayBuilder **colBuilders = malloc(sizeof(GArrowArrayBuilder *)* numberCols);
    for(int i=0; i < numberCols; i++){
        colBuilders[i] = garrow_record_batch_builder_get_column_builder(builder, i);
    }

    char *nextVal = NULL;
    GArrowArrayBuilder *currentBuilder = NULL;
    // 3. //TODO better way of iterating rows and columns?
    for(int i=0; i<numberTuples; i++){
        for(int j=0; j<numberCols; j++){
            nextVal = PQgetvalue(res,i,j);
            currentBuilder = colBuilders[j];
            switch(garrow_array_builder_get_value_type(currentBuilder)){
                case(GARROW_TYPE_DECIMAL128):
                    garrow_decimal128_array_builder_append_value(currentBuilder, garrow_decimal128_new_string(getStrFromPGNumeric((u_int16_t *)nextVal), &error), &error);
                    break;
                case(GARROW_TYPE_STRING):
                    garrow_string_array_builder_append_string(GARROW_STRING_ARRAY_BUILDER(currentBuilder), nextVal, &error);
                    break;
                case(GARROW_TYPE_INT32):
                    garrow_int32_array_builder_append_value(GARROW_INT32_ARRAY_BUILDER(currentBuilder), ntohl(*((uint32_t *) nextVal)), &error);
                    break;
                case(GARROW_TYPE_INT64):
                    garrow_int64_array_builder_append_value(currentBuilder, ntohl(*((uint64_t *) nextVal)), &error);
                    break;
                case(GARROW_TYPE_DATE32):
                    garrow_date32_array_builder_append_value(currentBuilder, ntohl(*((uint32_t *) nextVal)), &error);
                    break;
                default:
                    garrow_array_builder_append_null(currentBuilder, &error);
                    elog_debug("Found unknown datatype while building recordbatch!\n");
                    break;
            }
        }
    }

    // 4.
    resultBatch = garrow_record_batch_builder_flush(builder, &error);
    if(resultBatch == NULL){
        elog_debug("Failed to create RecordBatch! Error message: %s\n", error->message);
        g_error_free(error);
    }

    free(colBuilders);
    PQclear(res);

    return resultBatch;
}

/*
 * Cleans up after the client got his data
 */
void finishWarpTransaction(){
    g_object_unref(streamWriter);
    g_object_unref(bufferStream);
    g_object_unref(buffer);
}


/*
 * Callback function for giving more data to the warpServer (arrow version). TODO maybe do not make one giant record batch from pg result??? -> Record Batch size can be just determined from cursor fetch size!
 * TODO do not send the whole buffer each time data gets appended!!!
 */
void pg_warpServer_datacb_arrow(void **data, size_t *length){

    /*
     * 0. check warpres for NULL and call pgsql_next_result(conn) if it is NULL. If it is still NULL, clear everything and return NULL, you are finished.
     * 1. build Record batch
     * 5. write record batch into stream buffer
     * 6. set *data on the data pointer in buffer and length on size pointer in buffer
     */

    // 0.
    elog_debug("datacb start...\n");
    if(currentRecordBatch != NULL){
        elog_debug("unreffing objects...\n");
        g_object_unref(currentRecordBatch);
        currentRecordBatch = NULL;
    }

    elog_debug("Is PGresult ready?\n");
    if(warpres == NULL){
        elog_debug("empty pgresult, get new one...\n");
        warpres = pgsql_next_result();
        if(warpres == NULL){
            elog_debug("consumed all pg data... returning null\n");
            *data = NULL;
            *length = 0;
            warpres = PQexec(conn, "CLOSE " CURSOR_NAME ";" );
            if (PQresultStatus(warpres) != PGRES_COMMAND_OK)
                elog_debug("close cursor: %s", PQresultErrorMessage(warpres));
            PQclear(warpres);
            warpres = NULL;
            finishWarpTransaction();
            return;
        }
        numberTuples = PQntuples(warpres);
    }

    // 1.
    GError *error = NULL;
    currentRecordBatch = buildRecordBatch(warpres);
    if(currentRecordBatch == NULL){
        *data = NULL;
        *length = 0;
        return;
    }
    warpres = NULL;
    elog_debug("=== Data Callback result === \nRecordBatch rows: %lu\nRecordBatch columns: %d\n", garrow_record_batch_get_n_rows(currentRecordBatch),
            garrow_record_batch_get_n_columns(currentRecordBatch));

    // 5.
    gboolean test = garrow_record_batch_writer_write_record_batch(GARROW_RECORD_BATCH_WRITER(streamWriter), currentRecordBatch, &error);
    if(!test){
        elog_debug("Failed to write Record batch! Error message: %s\n", error->message);
        g_error_free(error);
    }

    // 6.
    *data = (void *) ((unsigned char *) g_bytes_get_data(garrow_buffer_get_data(GARROW_BUFFER(buffer)), length))+alreadySend;
    if(*data == NULL){
        elog_debug("Didn't get bytes pointer. Failed to allocate memory??\n");
    }else {
        *length -= alreadySend;
        elog_debug("Raw data size in bytes: %lu, already Send: %lu\n", *length, alreadySend);
        alreadySend += *length;
    }
}


/*
 * Reads data from PGResult and serializes it into row format
 * returns packets sizes of max 4096 for now
 * format:
 * tuple column types are known to receiver, so just write
 * <length of next data value><next data value>
 */
void readFromPGResultAsRow(void **data, size_t *length){
    elog_debug("Serialize next row buffer...\n");
    *length = 0;
    int nexttuplesize;
    // iterate pgresult until all tuples used
    while(tuplesused < numberTuples){
        nexttuplesize = 0;
        // check if next tuple fits in buffer, if not, return current data buffer
        elog_debug("Next Tuple contains:  ");
        for(int currentCol=0; currentCol<numberCols; currentCol++){
            int nextvallength = PQgetlength(warpres, tuplesused, currentCol);
            nexttuplesize += nextvallength;
            elog_debug("%d  ", nextvallength);
        }
        nexttuplesize += numberCols*sizeof(int);
        elog_debug("  as packet:  %d bytes.\n", nexttuplesize);
        if(*length+nexttuplesize > DEFAULT_BUFFER_SIZE) return;
        // it fits! so put next tuple into data buffer
        for(int currentCol=0; currentCol<numberCols; currentCol++){
            int nextvallength = PQgetlength(warpres, tuplesused, currentCol);
            *((int *)(*data+*length)) = nextvallength;
            *length += sizeof(int);
            memcpy((*data+*length), PQgetvalue(warpres, tuplesused, currentCol), nextvallength);
            /*if(nextvallength==25)
                elog_debug("This is the region: %.*s\n", 25, PQgetvalue(warpres, tuplesused, currentCol));
            if(nextvallength==4)
                elog_debug("This is the id: %d\n", ntohl(*(int *) PQgetvalue(warpres, tuplesused, currentCol)));*/
            *length += nextvallength;
        }
        tuplesused++;
    }
    PQclear(warpres);
    warpres = NULL;
}

/*
 * Reads data from PGResult and serializes it into column
 * format:
 */
void * readFromPGResultAsColumn(size_t *length){
    *length = 0;
    return NULL;
}

/*
 * Callback function for giving more data to the warpserver (pg-click-test)
 */
void pg_warpServer_datacb(void **data, size_t *length){

    /*
     * 1. check if we still got data locally in warpres (pgresult), if not, query next batch from server. if all data consumed, return null.
     * 2. read data from warpres
     * 3. serialize it as row or even as column (more to the format at the functions for row and column serialization)
     * 4. return it in the data pointer to the warpserver.
     */

    if(*data == NULL){
        elog_debug("Allocating data pointer...\n");
        //make buffer
        *data = calloc(DEFAULT_BUFFER_SIZE, 1);
    }

    // 1.
    elog_debug("Is PGresult ready?\n");
    if(warpres == NULL){
        elog_debug("empty PGresult, get new one...\n");
        warpres = pgsql_next_result();
        if(warpres == NULL){
            elog_debug("consumed all pg data... returning null\n");
            free(*data);
            *data = NULL;
            *length = 0;
            warpres = PQexec(conn, "CLOSE " CURSOR_NAME ";" );
            if (PQresultStatus(warpres) != PGRES_COMMAND_OK)
                elog_debug("close cursor: %s", PQresultErrorMessage(warpres));
            warpres = PQexec(conn, "COMMIT;" );
            if (PQresultStatus(warpres) != PGRES_COMMAND_OK)
                elog_debug("commit: %s", PQresultErrorMessage(warpres));
            PQclear(warpres);
            warpres = NULL;
            queryOngoing = 0;
            return;
        }
        numberTuples = PQntuples(warpres);
        tuplesused = 0;
    }

    // 2. + 3. serialize as row or column format
    elog_debug("Reading from PGresult...\n");
    if(rowFormat) readFromPGResultAsRow(data, length);
    else *data = readFromPGResultAsColumn(length);

    elog_debug("Returning data pointer with %lu bytes to warpserver.\n", *length);
}

/*
 * ========================  Helper Functions ===================================
 */


static void
exit_nicely(int code)
{
    elog_debug("Stopping server...\n");
    int ret;
    ret = warpserver_stop();
    if(ret != 0) elog_debug("Failed to stop warp server! Error code: %d\n", ret);
    else elog_debug("Stopping warp server successful!\n");
    PQclear(warpres);
    PQfinish(conn);
    elog_debug("Finished postgres connection!\n");
    exit(code);
}

/*
 * This function prints a query result that is a binary-format fetch from
 * a table defined as in the comment above.  We split it out because the
 * main() function uses it twice.
 * TODO this is only specialised for the tpc-h regions table. Can you make it unspecialized? But low priority!
 */
static void
show_binary_results(PGresult *res)
{
    int         i,
            j;
    int         key_fnum,
            name_fnum,
            comment_fnum;

    /* Use PQfnumber to avoid assumptions about field order in result */
    key_fnum = PQfnumber(res, "r_regionkey");
    name_fnum = PQfnumber(res, "r_name");
    comment_fnum = PQfnumber(res, "r_comment");

    for (i = 0; i < PQntuples(res); i++)
    {
        char       *keyptr;
        char       *nameptr;
        char       *commentptr;
        int         commentlen;
        int         keyval;

        /* Get the field values (we ignore possibility they are null!) */
        keyptr = PQgetvalue(res, i, key_fnum);
        nameptr = PQgetvalue(res, i, name_fnum);
        commentptr = PQgetvalue(res, i, comment_fnum);

        /*
         * The binary representation of INT is in network byte order, which
         * we'd better coerce to the local byte order.
         */
        keyval = ntohl(*((uint32_t *) keyptr));

        /*
         * The binary representation of TEXT is, well, text, and since libpq
         * was nice enough to append a zero byte to it, it'll work just fine
         * as a C string.
         *
         * The binary representation of BYTEA is a bunch of bytes, which could
         * include embedded nulls so we have to pay attention to field length.
         */
        commentlen = PQgetlength(res, i, comment_fnum);

        printf("tuple %d: got\n", i);
        printf(" key = (%d bytes) %d\n",
               PQgetlength(res, i, key_fnum), keyval);
        printf(" name = (%d bytes) '%s'\n",
               PQgetlength(res, i, name_fnum), nameptr);
        printf(" region = (%d bytes) '%s'\n", PQgetlength(res, i, comment_fnum), commentptr);
        printf("\n\n");
    }
}
