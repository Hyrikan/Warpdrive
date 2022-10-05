//
// Created by joel on 29.06.22.
//
#include "pg_warpserver.h"

#define DEC_DIGITS 4

typedef struct NumericVar
{
    int         ndigits;        /* # of digits in digits[] - can be 0! */
    int         weight;         /* weight of first digit */
    int         sign;           /* NUMERIC_POS, _NEG, _NAN, _PINF, or _NINF */
    int         dscale;         /* display scale */
    u_int16_t   *digits;       /* base-NBASE digits */
} NumericVar;

void print_binary(unsigned char x) {
    int b = 128;

    while (b != 0) {

        if (b <= x) {
            x -= b;
            printf("1");
        } else
            printf("0");

        b = b >> 1;
    }
}


GArrowArray *buildArray(){
    GArrowArray *array;

    {
        GArrowInt32ArrayBuilder *builder;
        gboolean success = TRUE;
        GError *error = NULL;

        builder = garrow_int32_array_builder_new();
        if (success) {
            success = garrow_int32_array_builder_append_value(builder, 29, &error);
        }
        if (success) {
            success = garrow_int32_array_builder_append_value(builder, 2929, &error);
        }
        if (success) {
            success = garrow_int32_array_builder_append_value(builder, 292929, &error);
        }
        if (!success) {
            g_print("failed to append: %s\n", error->message);
            g_error_free(error);
            g_object_unref(builder);
            return NULL;
        }
        array = garrow_array_builder_finish(GARROW_ARRAY_BUILDER(builder), &error);
        if (!array) {
            g_print("failed to finish: %s\n", error->message);
            g_error_free(error);
            g_object_unref(builder);
            return NULL;
        }
        g_object_unref(builder);
    }

    {
        gint64 i, n;

        n = garrow_array_get_length(array);
        g_print("length: %" G_GINT64_FORMAT "\n", n);
        for (i = 0; i < n; i++) {
            gint32 value;

            value = garrow_int32_array_get_value(GARROW_INT32_ARRAY(array), i);
            g_print("array[%" G_GINT64_FORMAT "] = %d\n",
                    i, value);
        }
    }
    return array;
}

void printFirstInt16s (char *dest, u_int16_t *src, int len)
{
    for(int i = 0; i < len; i++){
        sprintf(dest,"%d", src[i]);
    }
}

char *getStrFromNumeric(u_int16_t *numvar){
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
    return result;
}

void testarrowfreeing(){

    // Make List of Arrow Arrays, which make the column of a table.
    GList *arrays = g_list_alloc();
    arrays = g_list_append(arrays, buildArray());

    // Make Chunked Array ( one column of an Arrow Table)
    GArrowChunkedArray *charray = NULL;
    //charray = garrow_chunked_array_new(arrays); // THIS IS NOT WORKING? SIGSEGV!

    // Test chunked array
    if(charray == NULL){
        printf("Empty list!\n");
        g_list_free(arrays);
    } else {
        printf("Not Empty list!\n");
        g_list_free(arrays);
        g_object_unref(charray);
    }
}

static void
print_array(GArrowArray *array)
{
    GArrowType value_type;
    gint64 i, n;

    value_type = garrow_array_get_value_type(array);

    g_print("[");
    n = garrow_array_get_length(array);

#define ARRAY_CASE(type, Type, TYPE, format)                            \
  case GARROW_TYPE_ ## TYPE:                                            \
    {                                                                   \
      GArrow ## Type ## Array *real_array;                              \
      real_array = GARROW_ ## TYPE ## _ARRAY(array);                    \
      for (i = 0; i < n; i++) {                                         \
        if (i > 0) {                                                    \
          g_print(", ");                                                \
        }                                                               \
        g_print(format,                                                 \
                garrow_ ## type ## _array_get_value(real_array, i));    \
      }                                                                 \
    }                                                                   \
    break

    switch (value_type) {
        ARRAY_CASE(uint8,  UInt8,  UINT8,  "%hhu");
        ARRAY_CASE(uint16, UInt16, UINT16, "%" G_GUINT16_FORMAT);
        ARRAY_CASE(uint32, UInt32, UINT32, "%" G_GUINT32_FORMAT);
        ARRAY_CASE(uint64, UInt64, UINT64, "%" G_GUINT64_FORMAT);
        ARRAY_CASE( int8,   Int8,   INT8,  "%hhd");
        ARRAY_CASE( int16,  Int16,  INT16, "%" G_GINT16_FORMAT);
        ARRAY_CASE( int32,  Int32,  INT32, "%" G_GINT32_FORMAT);
        ARRAY_CASE( int64,  Int64,  INT64, "%" G_GINT64_FORMAT);
        ARRAY_CASE( float,  Float,  FLOAT, "%g");
        ARRAY_CASE(double, Double, DOUBLE, "%g");
        default:
            break;
    }
#undef ARRAY_CASE

    g_print("]\n");
}


static void
print_record_batch(GArrowRecordBatch *record_batch)
{
    guint nth_column, n_columns;

    n_columns = garrow_record_batch_get_n_columns(record_batch);
    for (nth_column = 0; nth_column < n_columns; nth_column++) {
        GArrowArray *array;

        g_print("columns[%u](%s): ",
                nth_column,
                garrow_record_batch_get_column_name(record_batch, nth_column));
        array = garrow_record_batch_get_column_data(record_batch, nth_column);
        print_array(array);
        g_object_unref(array);
    }
}


/*
 * 1. create recordbatch with schema and metadata
 * 2. build one array for each column
 * 3. put them inside recordbatch
 * 4. try to print it
 */
void testarrowrecordbatch(){

    // 1.
    GList *fields = NULL;
    for(int i = 0; i < 3; i++){
        char *colname = "Test column";
        printf("Column name %d: %s \n", i, colname);
        GArrowField *nextField = garrow_field_new(colname, (GArrowDataType *) garrow_int32_data_type_new());
        fields = g_list_append(fields, nextField);
    }
    GArrowSchema *currentSchema = garrow_schema_new(fields);
    if(currentSchema == NULL){
        printf("Schema creation failed!\n");
    }else printf("Created Schema with %d columns:\n %s\n", garrow_schema_n_fields(currentSchema),garrow_schema_to_string(currentSchema));

    // 2.
    GList *cols = NULL;
    for(int i = 0; i < 3; i++){
        GArrowArray *colarr = buildArray();
        cols = g_list_append(cols, colarr);
    }
    GList *it = g_list_first(cols);
    printf("Trying to print list: \n");
    while(it != NULL){
        printf("Element\n");
        it = g_list_next(it);
    }

    // 3.
    GError *error = NULL;
    printf("Creating recordbatch...\n");
    GArrowRecordBatch *recordBatch = garrow_record_batch_new(currentSchema, 3, cols, &error);
    if(recordBatch == NULL){
        printf("Failed to create recordbatch! Error: %s\n", error->message);
        return;
    }

    // 4.
    printf("Created Record batch: \n");
    print_record_batch(recordBatch);


    // alternate version with record batch builder

    printf("Test alternative: \n");
    GArrowRecordBatchBuilder *builder = garrow_record_batch_builder_new(currentSchema, &error);
    garrow_record_batch_builder_set_initial_capacity(builder, 4);
    GArrowArrayBuilder **colBuilders = malloc(sizeof(GArrowArrayBuilder *)* garrow_record_batch_builder_get_n_columns(builder));
    for(int i=0; i < garrow_record_batch_builder_get_n_columns(builder); i++){
        colBuilders[i] = garrow_record_batch_builder_get_column_builder(builder, i);
    }
    for(int i=0; i<5;i++){
        for(int j=0; j<garrow_record_batch_builder_get_n_columns(builder);j++){
            garrow_int32_array_builder_append_value(colBuilders[j], i, &error);
        }
    }
    GArrowRecordBatch *anotherBatch = garrow_record_batch_builder_flush(builder, &error);
    print_record_batch(anotherBatch);
    GArrowBuffer *buffer = garrow_record_batch_serialize(anotherBatch, NULL, &error);
    printf("Serialized record batch size: %lu\n", garrow_buffer_get_size(buffer));

}

void testarrowdecimal(){
    char *decimalstring = "245.54";
    GError *error = NULL;
    GArrowDecimal128 *decimalarrow = garrow_decimal128_new_string(decimalstring, &error);
    GArrowDecimal128ArrayBuilder *builder = garrow_decimal128_array_builder_new(garrow_decimal128_data_type_new(20,2,&error));
    garrow_decimal128_array_builder_append_value(builder, decimalarrow, &error);
    GArrowArray *array = garrow_array_builder_finish(builder, &error);
    GArrowDecimal128 *arraydecimal = garrow_decimal128_array_get_value(array, 0);
    printf("Decimal as String: %s\n", decimalstring);
    printf("Decimal converted to arrow and back to string: %s\n", garrow_decimal128_to_string(decimalarrow));
    printf("Decimal converted to arrowarray and back to string: %s\n", garrow_decimal128_to_string(arraydecimal));
    printf("Decimal in Array: %s \n", garrow_array_to_string(array, &error));
    printf("Printed as %lu bytes: \n", sizeof(GArrowDecimal128));
    for(int i=0; i<sizeof(GArrowDecimal128); i++){
        print_binary(((unsigned char *) decimalarrow)[i]);
        printf(" | ");
    }
    printf("\n");
    printf("Printed as %lu bytes: \n", sizeof(GArrowDecimal128));
    for(int i=0; i<sizeof(GArrowDecimal128); i++){
        print_binary(((unsigned char *) arraydecimal)[i]);
        printf(" | ");
    }
    printf("\n");
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
    int numberCols = PQnfields(res);
    int numberTuples = PQntuples(res);
    GError *error = NULL;

    //hardcoded schema for numerictest
    /* create schema: read column names and data types and write them as fields into schema */
    GList *fields = NULL;
    for(int i = 0; i < numberCols; i++){
        char *colname = PQfname(res, i);
        if(i == 0){
            GArrowField *nextField = garrow_field_new(colname, garrow_int32_data_type_new());
            fields = g_list_append(fields, nextField);
        }
        else{
            GArrowField *nextField = garrow_field_new(colname, garrow_decimal128_data_type_new(20,2,&error));
            fields = g_list_append(fields, nextField);
        }
    }
    GArrowSchema *currentSchema = garrow_schema_new(fields);
    if(currentSchema == NULL){
        fprintf(stderr,"Schema creation failed!\n");
    }

    printf("Build schema..\n");
    // 1.
    GArrowRecordBatchBuilder *builder = garrow_record_batch_builder_new(currentSchema, &error);
    garrow_record_batch_builder_set_initial_capacity(builder, numberTuples);

    // 2.
    GArrowArrayBuilder **colBuilders = malloc(sizeof(GArrowArrayBuilder *)* numberCols);
    for(int i=0; i < numberCols; i++){
        colBuilders[i] = garrow_record_batch_builder_get_column_builder(builder, i);
    }

    printf("Build recordbatchbuilder..\n");
    char *nextVal = NULL;
    GArrowArrayBuilder *currentBuilder = NULL;
    // 3. //TODO better way of iterating rows and columns?
    for(int i=0; i<numberTuples; i++){
        for(int j=0; j<numberCols; j++){
            nextVal = PQgetvalue(res,i,j);
            currentBuilder = colBuilders[j];
            switch(garrow_array_builder_get_value_type(currentBuilder)){
                case(GARROW_TYPE_DECIMAL128):
                    garrow_decimal128_array_builder_append_value(currentBuilder, garrow_decimal128_new_string(getStrFromNumeric((u_int16_t *)nextVal), &error), &error);
                    break;
                case(GARROW_TYPE_STRING):
                    garrow_string_array_builder_append_string(currentBuilder, nextVal, &error);
                    break;
                case(GARROW_TYPE_INT32):
                    garrow_int32_array_builder_append_value(currentBuilder, ntohl(*((uint32_t *) nextVal)), &error);
                    break;
                case(GARROW_TYPE_INT64):
                    garrow_int64_array_builder_append_value(currentBuilder, ntohl(*((uint64_t *) nextVal)), &error);
                    break;
                case(GARROW_TYPE_DATE32):
                    garrow_date32_array_builder_append_value(currentBuilder, ntohl(*((uint32_t *) nextVal)), &error);
                    break;
                default:
                    garrow_array_builder_append_null(currentBuilder, &error);
                    fprintf(stderr, "Found unknown datatype while building recordbatch!\n");
                    break;
            }
        }
    }

    printf("filled with data..\n");

    // 4.
    resultBatch = garrow_record_batch_builder_flush(builder, &error);
    if(resultBatch == NULL){
        fprintf(stderr, "Failed to create RecordBatch! Error message: %s\n", error->message);
        g_error_free(error);
    }

    free(colBuilders);

    return resultBatch;
}

GArrowRecordBatch * testrecordbatchDecimal(PGconn *conn){
    PGresult *res = PQexecParams(conn, "SELECT * FROM numerictest LIMIT 5;", 0, NULL, NULL, NULL, NULL, 1);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "unable to begin transaction: %s", PQresultErrorMessage(res));
        PQclear(res);
        PQfinish(conn);
        exit(1);
    }
    printf("Got another pgresult with numerictest data.\n");
    GArrowRecordBatch *rb = buildRecordBatch(res);
    GError *error = NULL;
    printf("RecordBatch with decimals: \n%s\n", garrow_record_batch_to_string(rb, &error));
    return rb;
}

void testRecordbatchStream(GArrowRecordBatch *rb1, PGconn *conn){
    PGresult *res = PQexecParams(conn, "SELECT * FROM numerictest LIMIT 5;", 0, NULL, NULL, NULL, NULL, 1);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "unable to begin transaction: %s", PQresultErrorMessage(res));
        PQclear(res);
        PQfinish(conn);
        exit(1);
    }
    GArrowRecordBatch *rb2 = buildRecordBatch(res);

    GError *error = NULL;

    printf("Got 2 RecordBatches, each with this data:\n%s\n", garrow_record_batch_to_string(rb1, &error));

    GArrowResizableBuffer *buffer = garrow_resizable_buffer_new(4096, &error);
        if(buffer == NULL){
        fprintf(stderr, "Failed to initialize resizable buffer! Error message: %s\n", error->message);
        g_error_free(error);
    }

    GArrowBufferOutputStream *bufferStream = garrow_buffer_output_stream_new(buffer);
    GArrowSchema *schema = garrow_record_batch_get_schema(rb1);
    GArrowRecordBatchStreamWriter *sw = garrow_record_batch_stream_writer_new(GARROW_OUTPUT_STREAM(bufferStream), schema, &error);
    if(sw == NULL){
        fprintf(stderr, "Failed to create Record batch writer! Error message: %s\n", error->message);
        g_error_free(error);
    }

    g_object_unref(bufferStream);
    g_object_unref(schema);

    gboolean test = garrow_record_batch_writer_write_record_batch(GARROW_RECORD_BATCH_WRITER(sw), rb1, &error);
    if(!test){
        fprintf(stderr, "Failed to write Record batch! Error message: %s\n", error->message);
        g_error_free(error);
    }

    GBytes *data = garrow_buffer_get_data(GARROW_BUFFER(buffer));
    gint64 length = garrow_buffer_get_size(GARROW_BUFFER(buffer));

    gsize datasize;
    gconstpointer datap = g_bytes_get_data(data, &datasize);
    void *pointer = (void *)datap;
    size_t pointersize = datasize;


    GArrowBuffer *receivingBuffer = garrow_buffer_new(pointer, pointersize);

    GArrowBufferInputStream *inputStream = garrow_buffer_input_stream_new(GARROW_BUFFER(receivingBuffer));
    GArrowRecordBatchStreamReader *sr = garrow_record_batch_stream_reader_new(GARROW_INPUT_STREAM(inputStream), &error);
    if(sr == NULL){
        fprintf(stderr, "Failed to create stream reader! Error message: %s\n", error->message);
        g_error_free(error);
    }

    printf("Reading 2 RecordBatches:\n");
    GArrowRecordBatch *rb3 = garrow_record_batch_reader_read_next(GARROW_RECORD_BATCH_READER(sr), &error);
        if(rb3 == NULL){
            printf("Failed to create record batch from stream... Error message: %s\n", error->message);
            g_error_free(error);
        }else {
            printf("Recordbatch1:\n%s\n", garrow_record_batch_to_string(rb3, &error));
        }


    test = garrow_record_batch_writer_write_record_batch(GARROW_RECORD_BATCH_WRITER(sw), rb2, &error);
    if(!test){
        fprintf(stderr, "Failed to write Record batch! Error message: %s\n", error->message);
        g_error_free(error);
    }

    GArrowRecordBatch *rb4 = garrow_record_batch_reader_read_next(GARROW_RECORD_BATCH_READER(sr), &error);
    if(rb3 == NULL){
        printf("Failed to create record batch from stream... Error message: %s\n", error->message);
        g_error_free(error);
    }else {
        printf("Recordbatch2:\n%s\n", garrow_record_batch_to_string(rb4, &error));
    }

    if(garrow_record_batch_equal(rb1,rb3) || garrow_record_batch_equal(rb2,rb4)){
        printf("==== Result ====\nrb1 and rb3 are equal\nrb2 and rb4 are equal\n");
    } else printf("The RecordBatches are NOT equal!\n");



    g_bytes_unref(data);
    g_object_unref(sw);
    g_object_unref(inputStream);
    g_object_unref(rb2);
    g_object_unref(rb3);
    g_object_unref(rb4);
    g_object_unref(rb1);
    g_object_unref(sr);
    g_object_unref(buffer);
}


int main (int argc, char **argv){
    PGconn *conn;
    int status;
    const char *keys[20];
    const char *values[20];
    int			index = 0;

    keys[index] = "host";
    values[index] = "localhost";
    index++;

    keys[index] = "port";
    values[index] = "5432";
    index++;

    /*
    keys[index] = "dbname";
    values[index] = pgsql_database;
    index++;
    */

    keys[index] = "user";
    values[index] = "postgres";
    index++;


    keys[index] = "password";
    values[index] = "iu7trdu8OpOIfud6";
    index++;


    /* terminal */
    keys[index] = NULL;
    values[index] = NULL;

    printf("Connect to database...\n");
    conn = PQconnectdbParams(keys, values, 0);
    if (!conn)
        fprintf(stderr, "connection failed\n");
    status = PQstatus(conn);
    if (status != CONNECTION_OK)
        fprintf(stderr, "failed on PostgreSQL connection: %s\n",
                PQerrorMessage(conn));


    PGresult *res;
    printf("Query database in String mode...\n");
    res = PQexec(conn, "SELECT acctbal FROM numerictest LIMIT 5;");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "unable to begin transaction: %s", PQresultErrorMessage(res));
        PQclear(res);
        PQfinish(conn);
        exit(1);
    }

    int rownum = PQntuples(res);
    printf("Rownumbers: %d\n", rownum);
    for(int i = 0; i<rownum; i++){
        printf("Row number %d with value: %s\n", i, PQgetvalue(res, i, 0));
    }
    PQclear(res);

    printf("Query numeric precision and scale...\n");
    res = PQexecParams(conn, "SELECT (atttypmod - 4) >> 16 & 65535 AS precision,\n"
                             "       (atttypmod - 4) & 65535 AS scale\n"
                             "FROM pg_attribute\n"
                             "WHERE attrelid = 'numerictest'::regclass\n"
                             "  AND attname = 'acctbal';", 0, NULL, NULL, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "unable to begin transaction: %s", PQresultErrorMessage(res));
        PQclear(res);
        PQfinish(conn);
        exit(1);
    }
    rownum = PQntuples(res);
    printf("Precision: %s\n", PQgetvalue(res, 0, 0));
    printf("Scale: %s\n", PQgetvalue(res, 0, 1));
    PQclear(res);

    printf("Query database in binary mode...\n");
    res = PQexecParams(conn, "SELECT acctbal FROM numerictest LIMIT 5;", 0, NULL, NULL, NULL, NULL, 1);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "unable to begin transaction: %s", PQresultErrorMessage(res));
        PQclear(res);
        PQfinish(conn);
        exit(1);
    }

    rownum = PQntuples(res);
    printf("Rownumbers: %d\n", rownum);
    for(int i = 0; i<rownum; i++){
        printf("Row number %d with value: ", i);
        char *oneNumVal =PQgetvalue(res, i, 0);
        printf("%s\n", getStrFromNumeric((u_int16_t *)oneNumVal));
        int oneNumValLen = PQgetlength(res, i, 0);
        for (int j = 0; j < oneNumValLen; j++) {
            switch(j){
                case 0:
                    printf("ndigits | ");
                    break;
                case 2:
                    printf("weight | ");
                    break;
                case 4:
                    printf("sign | ");
                    break;
                case 6:
                    printf("dscale | ");
                    break;
                case 8:
                    printf("digits | ");
                default:
                    ;
            }
            print_binary(oneNumVal[j]);
            printf(" | ");
        }
        printf("\n");
    }
    printf("Freeing ressoures.\n");
    PQclear(res);

    printf("============================== Arrow Decimal Layout Test ===============================\n");

    testarrowdecimal();

    printf("===========  Test garrow freeing ressources..!  ==================\n");


    testarrowfreeing();


    printf("===========  Test garrow RecordBatch creation!  ==================\n");

    testarrowrecordbatch();

    printf("================= Test decimal record batch =======================\n");
    GArrowRecordBatch *rb = testrecordbatchDecimal(conn);


    printf("================= Test record batch stream input/output =======================\n");

    testRecordbatchStream(rb, conn);

    PQfinish(conn);
}
