//
// Created by joel on 02.05.22.
//
#include "hello_world_util.h"
#include "warp_server.h"

static long test_string_length = 16;
static long iov_cnt            = 1;
static uint16_t server_port    = DEFAULT_PORT;
static sa_family_t ai_family   = AF_INET;



/**
 * Server's application context to be used in the user's connection request
 * callback.
 * It holds the server's listener and the handle to an incoming connection request.
 */
typedef struct ucx_server_ctx {
    volatile ucp_conn_request_h conn_request;
    ucp_listener_h              listener;
} ucx_server_ctx_t;


/**
 * Stream request context. Holds a value to indicate whether or not the
 * request is completed.
 */
typedef struct test_req {
    size_t byteswritten;
    int ready;
} test_req_t;

/**
 * Some more variables from me
 */
char *listen_addr = NULL;
int warp_stop_sign = 0;
pthread_t server;
int retval;
int initialized = 0;
warpserver_cb_query queryCallback;
warpserver_cb_data dataCallback;



/**
 * Parse the command line arguments.
 */
static int parse_cmd(char *listen_addr_local, int port, int useINet6)
{
    if(NULL != listen_addr_local) listen_addr = listen_addr_local;
    if((port >= 0) && (port <= UINT16_MAX)) server_port = port;
    if(useINet6) ai_family = AF_INET6;
    return 0;
}


/**
 * Create a ucp worker on the given ucp context.
 */
static int init_worker(ucp_context_h ucp_context, ucp_worker_h *ucp_worker)
{
    ucp_worker_params_t worker_params;
    ucs_status_t status;
    int ret = 0;

    memset(&worker_params, 0, sizeof(worker_params));

    worker_params.field_mask  = UCP_WORKER_PARAM_FIELD_THREAD_MODE;
    worker_params.thread_mode = UCS_THREAD_MODE_SINGLE;

    status = ucp_worker_create(ucp_context, &worker_params, ucp_worker);
    if (status != UCS_OK) {
        DLOG( "failed to ucp_worker_create (%s)\n", ucs_status_string(status));
        ret = -1;
    }

    return ret;
}

/**
 * Initialize the UCP context and worker.
 */
static int init_context(ucp_context_h *ucp_context, ucp_worker_h *ucp_worker)
{
    /* UCP objects */
    ucp_params_t ucp_params;
    ucs_status_t status;
    int ret;

    memset(&ucp_params, 0, sizeof(ucp_params));

    /* UCP initialization */
    ucp_params.field_mask = UCP_PARAM_FIELD_FEATURES;
    ucp_params.features = UCP_FEATURE_STREAM;

    status = ucp_init(&ucp_params, NULL, ucp_context);
    if (status != UCS_OK) {
        DLOG( "failed to ucp_init (%s)\n", ucs_status_string(status));
        ret = -1;
        return ret;
    }

    ret = init_worker(*ucp_context, ucp_worker);
    if (ret != 0) {
        ucp_cleanup(*ucp_context);
    }

    return ret;
}


/**
 * Set an address for the server to listen on - INADDR_ANY on a well known port.
 */
void set_sock_addr(const char *address_str, struct sockaddr_storage *saddr)
{
    struct sockaddr_in *sa_in;
    struct sockaddr_in6 *sa_in6;

    /* The server will listen on INADDR_ANY */
    memset(saddr, 0, sizeof(*saddr));

    switch (ai_family) {
        case AF_INET:
            sa_in = (struct sockaddr_in*)saddr;
            if (address_str != NULL) {
                inet_pton(AF_INET, address_str, &sa_in->sin_addr);
            } else {
                sa_in->sin_addr.s_addr = INADDR_ANY;
            }
            sa_in->sin_family = AF_INET;
            sa_in->sin_port   = htons(server_port);
            break;
        case AF_INET6:
            sa_in6 = (struct sockaddr_in6*)saddr;
            if (address_str != NULL) {
                inet_pton(AF_INET6, address_str, &sa_in6->sin6_addr);
            } else {
                sa_in6->sin6_addr = in6addr_any;
            }
            sa_in6->sin6_family = AF_INET6;
            sa_in6->sin6_port   = htons(server_port);
            break;
        default:
            DLOG( "Invalid address family");
            break;
    }
}


static char* sockaddr_get_ip_str(const struct sockaddr_storage *sock_addr,
                                 char *ip_str, size_t max_size)
{
    struct sockaddr_in  addr_in;
    struct sockaddr_in6 addr_in6;

    switch (sock_addr->ss_family) {
        case AF_INET:
            memcpy(&addr_in, sock_addr, sizeof(struct sockaddr_in));
            inet_ntop(AF_INET, &addr_in.sin_addr, ip_str, max_size);
            return ip_str;
        case AF_INET6:
            memcpy(&addr_in6, sock_addr, sizeof(struct sockaddr_in6));
            inet_ntop(AF_INET6, &addr_in6.sin6_addr, ip_str, max_size);
            return ip_str;
        default:
            return "Invalid address family";
    }
}

static char* sockaddr_get_port_str(const struct sockaddr_storage *sock_addr,
                                   char *port_str, size_t max_size)
{
    struct sockaddr_in  addr_in;
    struct sockaddr_in6 addr_in6;

    switch (sock_addr->ss_family) {
        case AF_INET:
            memcpy(&addr_in, sock_addr, sizeof(struct sockaddr_in));
            snprintf(port_str, max_size, "%d", ntohs(addr_in.sin_port));
            return port_str;
        case AF_INET6:
            memcpy(&addr_in6, sock_addr, sizeof(struct sockaddr_in6));
            snprintf(port_str, max_size, "%d", ntohs(addr_in6.sin6_port));
            return port_str;
        default:
            return "Invalid address family";
    }
}


/**
 * The callback on the server side which is invoked upon receiving a connection
 * request from the client.
 */
static void server_conn_handle_cb(ucp_conn_request_h conn_request, void *arg)
{
    ucx_server_ctx_t *context = arg;
    ucp_conn_request_attr_t attr;
    char ip_str[IP_STRING_LEN];
    char port_str[PORT_STRING_LEN];
    ucs_status_t status;

    attr.field_mask = UCP_CONN_REQUEST_ATTR_FIELD_CLIENT_ADDR;
    status = ucp_conn_request_query(conn_request, &attr);
    if (status == UCS_OK) {
       DLOG("Server received a connection request from client at address %s:%s\n",
               sockaddr_get_ip_str(&attr.client_address, ip_str, sizeof(ip_str)),
               sockaddr_get_port_str(&attr.client_address, port_str, sizeof(port_str)));
    } else if (status != UCS_ERR_UNSUPPORTED) {
        DLOG( "failed to query the connection request (%s)\n",
                ucs_status_string(status));
    }

    if (context->conn_request == NULL) {
        context->conn_request = conn_request;
    } else {
        /* The server is already handling a connection request from a client,
         * reject this new one */
       DLOG("Rejecting a connection request. "
               "Only one client at a time is supported.\n");
        status = ucp_listener_reject(context->listener, conn_request);
        if (status != UCS_OK) {
            DLOG( "server failed to reject a connection request: (%s)\n",
                    ucs_status_string(status));
        }
    }
}


/**
 * Initialize the server side. The server starts listening on the set address.
 */
static ucs_status_t
start_server(ucp_worker_h ucp_worker, ucx_server_ctx_t *context,
             ucp_listener_h *listener_p)
{
    struct sockaddr_storage addr_storage;
    ucp_listener_params_t params;
    ucp_listener_attr_t attr;
    ucs_status_t status;
    char ip_str[IP_STRING_LEN];
    char port_str[PORT_STRING_LEN];

    set_sock_addr(listen_addr, &addr_storage);

    params.field_mask         = UCP_LISTENER_PARAM_FIELD_SOCK_ADDR |
                                UCP_LISTENER_PARAM_FIELD_CONN_HANDLER;
    params.sockaddr.addr      = (const struct sockaddr*)&addr_storage;
    params.sockaddr.addrlen   = sizeof(addr_storage);
    params.conn_handler.cb    = server_conn_handle_cb;
    params.conn_handler.arg   = context;

    /* Create a listener on the server side to listen on the given address.*/
    status = ucp_listener_create(ucp_worker, &params, listener_p);
    if (status != UCS_OK) {
        DLOG( "failed to listen (%s)\n", ucs_status_string(status));
        goto out;
    }

    /* Query the created listener to get the port it is listening on. */
    attr.field_mask = UCP_LISTENER_ATTR_FIELD_SOCKADDR;
    status = ucp_listener_query(*listener_p, &attr);
    if (status != UCS_OK) {
        DLOG( "failed to query the listener (%s)\n",
                ucs_status_string(status));
        ucp_listener_destroy(*listener_p);
        goto out;
    }

    DLOG( "server is listening on IP %s port %s\n",
            sockaddr_get_ip_str(&attr.sockaddr, ip_str, IP_STRING_LEN),
            sockaddr_get_port_str(&attr.sockaddr, port_str, PORT_STRING_LEN));

   DLOG("Waiting for connection...\n");

    out:
    return status;
}




/**
 * Error handling callback.
 */
static void err_cb(void *arg, ucp_ep_h ep, ucs_status_t status)
{
   DLOG("error handling callback was invoked with status %d (%s)\n",
           status, ucs_status_string(status));
}


static ucs_status_t server_create_ep(ucp_worker_h data_worker,
                                     ucp_conn_request_h conn_request,
                                     ucp_ep_h *server_ep)
{
    ucp_ep_params_t ep_params;
    ucs_status_t    status;

    /* Server creates an ep to the client on the data worker.
     * This is not the worker the listener was created on.
     * The client side should have initiated the connection, leading
     * to this ep's creation */
    ep_params.field_mask      = UCP_EP_PARAM_FIELD_ERR_HANDLER |
                                UCP_EP_PARAM_FIELD_ERR_HANDLING_MODE |
                                UCP_EP_PARAM_FIELD_CONN_REQUEST;
    ep_params.err_mode        = UCP_ERR_HANDLING_MODE_PEER;
    ep_params.conn_request    = conn_request;
    ep_params.err_handler.cb  = err_cb;
    ep_params.err_handler.arg = NULL;

    status = ucp_ep_create(data_worker, &ep_params, server_ep);
    if (status != UCS_OK) {
        DLOG( "failed to create an endpoint on the server: (%s)\n",
                ucs_status_string(status));
    }

    return status;
}


void buffer_free(ucp_dt_iov_t *iov)
{
    size_t idx;

    for (idx = 0; idx < iov_cnt; idx++) {
        mem_type_free(iov[idx].buffer);
    }
}

int buffer_malloc(ucp_dt_iov_t *iov)
{
    size_t idx;

    for (idx = 0; idx < iov_cnt; idx++) {
        iov[idx].length = test_string_length;
        iov[idx].buffer = mem_type_malloc(iov[idx].length);
        if (iov[idx].buffer == NULL) {
            buffer_free(iov);
            return -1;
        }
    }

    return 0;
}


static void common_cb(void *user_data, const char *type_str, size_t length)
{
    test_req_t *ctx;

    if (user_data == NULL) {
        DLOG( "user_data passed to %s mustn't be NULL\n", type_str);
        return;
    }

    ctx           = user_data;
    ctx->byteswritten = length;
    ctx->ready = 1;
}

/**
 * The callback on the receiving side, which is invoked upon receiving the
 * stream message.
 */
static void stream_recv_cb(void *request, ucs_status_t status, size_t length,
                           void *user_data)
{
    common_cb(user_data, "stream_recv_cb", length);
}

/**
 * The callback on the sending side, which is invoked after finishing sending
 * the message.
 */
static void send_cb(void *request, ucs_status_t status, void *user_data)
{
    common_cb(user_data, "send_cb", 0);
}

/**
 * Progress the request until it completes.
 */
static ucs_status_t request_wait(ucp_worker_h ucp_worker, void *request,
                                 test_req_t *ctx)
{
    ucs_status_t status;

    /* if operation was completed immediately */
    if (request == NULL) {
        return UCS_OK;
    }

    if (UCS_PTR_IS_ERR(request)) {
       DLOG("Read was error.\n");
        return UCS_PTR_STATUS(request);
    }

    while (ctx->ready == 0) {
        ucp_worker_progress(ucp_worker);
    }
    status = ucp_request_check_status(request);

    ucp_request_free(request);
    return status;
}



/* Sends a message */
int warpserver_sendBuffer(ucp_worker_h ucp_worker, ucp_ep_h ep, ucp_dt_iov_t *buffer){

    ucp_request_param_t param;
    ucs_status_t *request;
    ucs_status_t status;
    test_req_t ctx;

    /* prepare send parametres. */
    ctx.ready       = 0;
    ctx.byteswritten =  0;
    param.op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK |  // use a callback function
                         UCP_OP_ATTR_FIELD_DATATYPE | // used datatype is the default. Just set here, to have the option to change it.
                         UCP_OP_ATTR_FIELD_USER_DATA; // lets us give userdata to the callback function.
    param.datatype     = ucp_dt_make_contig(1);
    param.cb.send      = send_cb;

    param.user_data = &ctx;//only first field of callback data is used, because we only use one buffer, not

    // wait for the query
    request              = ucp_stream_send_nbx(ep, buffer->buffer, buffer->length, &param);
    status               = request_wait(ucp_worker, request, &ctx);
    if(status != UCS_OK){
        DLOG( "Something off? Status error message: %s\n", ucs_status_string(status)); // if client ends connection (with flush) an error at the server happens and the last batch does not get read!
        return -1;
    }
    return 0;
}

/*
 * Sends dummy tuples for postgresql
 */
static void sendDummyData(ucp_dt_iov_t *buffer, test_req_t ctx, ucp_worker_h ucp_worker, ucp_ep_h ep){
    int j = 0;
    /*int32_t *dummyfield1 = (int32_t) malloc(5 * sizeof(int32_t));
    for(int i = 1; i < 6; i++){
        dummyfield1[i] = i;
    }*/
    char *dummyfield2[] = {"Joel", "Lara", "Charly", "Hari", "Karl Heinz"};
    //char *dummyfield3[] = {"Ziegler","Jurisch", "Vogler", "Haralompololo", "August"};

    while(j<5){
        buffer->length = strlen(dummyfield2[j])+1;
        memcpy(buffer->buffer, dummyfield2[j], buffer->length);
        warpserver_sendBuffer(ucp_worker, ep, buffer);
        DLOG("Send some dummydata...\n");
        ctx.ready       = 0;
        ctx.byteswritten = 0;
        j++;
    }
    DLOG("Finished Sending stuff.\n");
}

/* Handles the heavy lifting of communicating with the client */
static int warp_server_do_work(ucp_worker_h ucp_worker, ucp_ep_h ep)
{

    /*
     * buffer for receiving data
     * param for recv call
     * request and status to check communication status
     * ctx is a callback variable to signal completion of communication operation on buffers
     */
    ucp_dt_iov_t *buffer;
    ucp_request_param_t param;
    ucs_status_t *request;
    ucs_status_t status;
    test_req_t ctx;

    // allocate buffer and open file handle
    CHKERR_ACTION( NULL == (buffer = calloc(1, sizeof(ucp_dt_iov_t))),"allocate buffer struct", return -2);
    CHKERR_ACTION( NULL == (buffer->buffer = calloc(1, DEFAULT_BUFFER_SIZE)),"allocate intern buffer", free(buffer); return -2);
    buffer->length = DEFAULT_BUFFER_SIZE;

    //communication params params
    ctx.ready       = 0;
    ctx.byteswritten =  0;
    param.op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK |
                         UCP_OP_ATTR_FIELD_DATATYPE |
                         UCP_OP_ATTR_FIELD_USER_DATA;
    param.datatype     = ucp_dt_make_contig(1);
    param.user_data    = &ctx;
    param.cb.recv_stream = stream_recv_cb;
    //param.op_attr_mask  |= UCP_OP_ATTR_FIELD_FLAGS;
    //param.flags          = UCP_STREAM_RECV_FLAG_WAITALL; // always take a full buffer of data from recv... other solutions may be better in the future

    DLOG("Waiting for the queryheader (4 bytes)...\n");
    request              = ucp_stream_recv_nbx(ep, buffer->buffer, 4, &ctx.byteswritten, &param);
    status               = request_wait(ucp_worker, request, &ctx);
    if(status != UCS_OK){
        DLOG( "Client ended connection? Status error message: %s\n", ucs_status_string(status)); // if client ends connection (with flush) an error at the server happens and the last batch does not get read!
        // Close buffers
        free(buffer->buffer);
        free(buffer);
        return -1;
    }
    DLOG("Received the query header.\n");

    // Read transfer format bit
    unsigned char transferFormatBit = ((unsigned char *) buffer->buffer)[0];
    DLOG("First byte in hex: %02x\n", transferFormatBit);
    bool transferFormat = false;
    if(transferFormatBit&TRANSFERFORMAT_BITMASK) transferFormat = true;
    DLOG("Transfer format: %s\n", transferFormat ? "some row format" : "arrow columnar format");

    //Read Query length
    u_int16_t queryLength = *(u_int16_t *) (buffer->buffer+2);
    DLOG("Query length: %u\n", queryLength);

    DLOG("Waiting for the query...\n");
    request              = ucp_stream_recv_nbx(ep, buffer->buffer, queryLength, &ctx.byteswritten, &param);
    status               = request_wait(ucp_worker, request, &ctx);
    if(status != UCS_OK){
        DLOG( "Client ended connection? Status error message: %s\n", ucs_status_string(status)); // if client ends connection (with flush) an error at the server happens and the last batch does not get read!
        // Close buffers
        free(buffer->buffer);
        free(buffer);
        return -1;
    }
    DLOG("Received the query.\n");

    //Read query
    char * query = (char *) malloc(queryLength);
    memcpy(query, buffer->buffer, queryLength);
    DLOG("Query: %s\n", query);

    //Dummydata requested?
    if (0 == strcmp("SELECT * FROM *;", query)) {
        DLOG("Sending dummydata.\n");
        sendDummyData(buffer, ctx, ucp_worker, ep);
        free(buffer->buffer);
        free(buffer);
        free(query);

        return 0;
    }

    queryCallback(transferFormat, query);


    // Schicke jeweils DEFAULT_BUFFER_SIZE packete ab, bis data pointer abgeschickt, frage einen weiteren an, bis alle daten abgeschickt sind.
    DLOG("Reading and sending data...\n");
    void *data = NULL;
    size_t length;

    int check = 0;
    dataCallback(&data, &length);
    size_t dataSend = 0;
    while(data != NULL){
        DLOG("Got %lu bytes in data pointer\n", length);
        while(dataSend < length) {
            size_t remaining = length - dataSend;
            size_t dataCopyAmount = (remaining > DEFAULT_BUFFER_SIZE) ? DEFAULT_BUFFER_SIZE : remaining;
            memcpy(buffer->buffer, ((unsigned char *) data) + dataSend, dataCopyAmount);
            buffer->length = dataCopyAmount;
            check = warpserver_sendBuffer(ucp_worker, ep, buffer);
            if(check != 0) break;
            dataSend += dataCopyAmount;
        }
        if(check != 0) break;
        dataSend = 0;
        dataCallback(&data, &length);
    }



    // Close buffers
    free(buffer->buffer);
    free(buffer);

    return 0;
}


/**
 * Close the given endpoint.
 * Currently closing the endpoint with UCP_EP_CLOSE_MODE_FORCE since we currently
 * cannot rely on the client side to be present during the server's endpoint
 * closing process.
 */
static void ep_close(ucp_worker_h ucp_worker, ucp_ep_h ep)
{
    ucs_status_t status;
    void *close_req;

    close_req          = ucp_ep_close_nb(ep, UCP_EP_CLOSE_MODE_FLUSH);
    if (UCS_PTR_IS_PTR(close_req)) {
        do {
            ucp_worker_progress(ucp_worker);
            status = ucp_request_check_status(close_req);
        } while (status == UCS_INPROGRESS);

        ucp_request_free(close_req);
    } else if (UCS_PTR_STATUS(close_req) != UCS_OK) {
        DLOG( "failed to close ep. Status: %s\n", ucs_status_string(UCS_PTR_STATUS(close_req)));
    }
}

/* Main Server loop */
static int warp_run_server(ucp_context_h ucp_context, ucp_worker_h ucp_worker)
{
    ucx_server_ctx_t context;
    ucp_worker_h     ucp_data_worker;
    ucp_ep_h         server_ep;
    ucs_status_t     status;
    int              ret;

    /* Create a data worker (to be used for data exchange between the server
     * and the client after the connection between them was established) */
    CHKERR_ACTION(0 != (ret = init_worker(ucp_context, &ucp_data_worker)), "Worker initialization failed! Aborting...\n", return ret);

    /* Initialize the server's context. */
    context.conn_request = NULL;

    /* Create a listener on the worker created at first. The 'connection
     * worker' - used for connection establishment between client and server.
     * This listener will stay open for listening to incoming connection
     * requests from the client */
    CHKERR_ACTION(UCS_OK != (status = start_server(ucp_worker, &context, &context.listener)), "Failed to start the server! Aborting...\n",
                    ucp_worker_destroy(ucp_data_worker);
                    return status);

    /* Server is always up listening */
    while (1) {
        /* Wait for the server to receive a connection request from the client.
         * If there are multiple clients for which the server's connection request
         * callback is invoked, i.e. several clients are trying to connect in
         * parallel, the server will handle only the first one and reject the rest - for now*/
        while (context.conn_request == NULL) {
            ucp_worker_progress(ucp_worker);
            usleep(100000); //only block 10 times a second. Not as much as it can per second!
            // thread break
            if(warp_stop_sign == 1){
                break;
            }
        }
        //thread break
        if(1 == warp_stop_sign){
            DLOG("Server received stop signal! Exiting...\n");
            ucp_listener_destroy(context.listener);
            ucp_worker_destroy(ucp_data_worker);
            return ret;
        }

        /* Server creates an ep to the client on the data worker.
         * This is not the worker the listener was created on.
         * The client side should have initiated the connection, leading
         * to this ep's creation */
        DLOG("Got a connection request!\n");
        CHKERR_ACTION(UCS_OK != (status = server_create_ep(ucp_data_worker, context.conn_request,
                                                           &server_ep)), "Failed to create data worker! Aborting...\n",
                      ucp_listener_destroy(context.listener);
                      ucp_worker_destroy(ucp_data_worker);
                      return status;);

        /* The real server communication work with the connected client.
         * The server waits for all the iterations to complete before moving on
         * to the next client */
        ret = warp_server_do_work(ucp_data_worker, server_ep);
        if (ret != 0) break;

        /* Close the endpoint to the client */
        DLOG("Close endpoint to client.\n");
        ep_close(ucp_data_worker, server_ep);

        /* Reinitialize the server's context to be used for the next client */
        context.conn_request = NULL;

        DLOG("Waiting for new client connection...\n");
    }

    /* Cleanup */
    ep_close(ucp_data_worker, server_ep);
    ucp_listener_destroy(context.listener);
    ucp_worker_destroy(ucp_data_worker);
    return ret;
}

/*
 * Entrypoint for the warp_server
 * handles initialization of ucp objects, then starts the server
 */
void *start_warp_drive(void *unused)
{
    /* UCP objects */
    ucp_context_h ucp_context;
    ucp_worker_h  ucp_worker;

    /* Initialize the UCX required objects */
    CHKERR_ACTION(0 != (retval = init_context(&ucp_context, &ucp_worker)), "Initialization of UCP objects failed! Aborting...\n",
                  pthread_exit(&retval));

    /* Server initialization/start */
    CHKERR_ACTION(0 != (retval = warp_run_server(ucp_context, ucp_worker)), "Running the server failed! Exiting...\n", );

    /* Cleanup */
    ucp_worker_destroy(ucp_worker);
    ucp_cleanup(ucp_context);

    pthread_exit(&retval);
}


/*
 * Initializes the server with parametres and callback functions. But does not start the server.
 */
int warpserver_initialize(char *listen_addr_local, int port, int useINet6, warpserver_cb_query querycb, warpserver_cb_data datacb){
    int ret;
    CHKERR_ACTION(0 != (ret = parse_cmd(listen_addr_local, port, useINet6)) , "Parsing command failed! Exiting...\n", return ret);
    queryCallback = querycb;
    dataCallback = datacb;
    initialized = 1;
    return ret;
}

/*
 * Temporary thread start function
 * first parses commands, then starts a separate thread for the server, so that the server does not block the database.
 */
int warpserver_start(){
    int ret;
    if(initialized) ret = pthread_create(&server, NULL, start_warp_drive, NULL);
    else ret = -1;
    return ret;
}

/*
 * Temporary thread warp_stop_sign function
 * Sets a variable to signal the server thread to warp_stop_sign and then waits for the thread to warp_stop_sign.
 */
int warpserver_stop(){
    DLOG("Trying to stop server!\n");
    warp_stop_sign = 1;
    void *status;
    pthread_join(server, &status);
    DLOG("Server stopped!\n");
    return 0;
}