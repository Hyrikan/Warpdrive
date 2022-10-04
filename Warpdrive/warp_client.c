//
// Created by joel on 02.05.22.
//
#include "hello_world_util.h"
#include "warp_client.h"


static uint16_t server_port    = DEFAULT_PORT;
static sa_family_t ai_family   = AF_INET;

// some more variables
char *server_addr = NULL;
ucp_context_h ucp_context;
ucp_worker_h  ucp_worker;
ucp_ep_h     client_ep;
int communicationInProgress = 0;
ucp_dt_iov_t *recvbuffer;
ucp_request_param_t recvparam;


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
 * Initialize the UCP context and worker.
 */
static int init_context_and_worker()
{
    /* UCP objects: ucp_params for context params
     *              ucp_worker_params for worker params.
     *              status for function error code checks. */
    ucp_params_t ucp_params;
    ucp_worker_params_t worker_params;
    ucs_status_t status;
    int ret = 0;
    memset(&worker_params, 0, sizeof(worker_params));
    memset(&ucp_params, 0, sizeof(ucp_params));

    /* UCP initialization params*/
    ucp_params.field_mask = UCP_PARAM_FIELD_FEATURES; // mask selects used params.
    ucp_params.features = UCP_FEATURE_STREAM; // actual param telling the context to optimize for stream (in contrast to am or tag)

    /* real UCP initialization */
    DLOG("context...\n");
    status = ucp_init(&ucp_params, NULL, &ucp_context);
    if (status != UCS_OK) {
        DLOG("failed to ucp_init (%s)\n", ucs_status_string(status));
        ret = -1;
        goto err;
    }

    /* UCP worker initialization params */
    worker_params.field_mask  = UCP_WORKER_PARAM_FIELD_THREAD_MODE; // mask for selecting used params.
    worker_params.thread_mode = UCS_THREAD_MODE_SINGLE; // only param telling to use single thread mode (only one worker).

    /* real UCP worker creation */
    DLOG("worker...\n");
    status = ucp_worker_create(ucp_context, &worker_params, &ucp_worker);
    if (status != UCS_OK) {
        DLOG("failed to ucp_worker_create (%s)\n", ucs_status_string(status));
        ret = -1;
        goto err_cleanup;
    }

    return ret;

err_cleanup:
    ucp_cleanup(ucp_context);
err:
    return ret;
}


/**
 * Set an address for the server to listen on - INADDR_ANY on a well known port.
 */
void set_sock_addr(const char *address_str, struct sockaddr_storage *saddr)
{
    /* Preparing objects */
    struct sockaddr_in *sa_in;
    struct sockaddr_in6 *sa_in6;
    memset(saddr, 0, sizeof(*saddr));

    /* ip6 or ip4 address? */
    switch (ai_family) {
        case AF_INET:
            sa_in = (struct sockaddr_in*)saddr;
            /* Use the given address or use any address(INADDR_ANY) on the given port, if no address given. */
            if (address_str != NULL) {
                /* Translate written address from address_str to binary address and put it into &sa_in->sin_addr */
                inet_pton(AF_INET, address_str, &sa_in->sin_addr);
            } else {
                /* Just use all addresses on port (INADDR_ANY) */
                sa_in->sin_addr.s_addr = INADDR_ANY;
            }
            sa_in->sin_family = AF_INET;
            sa_in->sin_port   = htons(server_port);
            break;
        // same for ip6
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
            DLOG("Invalid address family");
            break;
    }
}


static void common_cb(void *user_data, const char *type_str, size_t length)
{
    test_req_t *ctx;

    if (user_data == NULL) {
        DLOG("user_data passed to %s mustn't be NULL\n", type_str);
        return;
    }

    ctx           = user_data;
    ctx->byteswritten = length;
    ctx->ready = 1;
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
 * The callback on the receiving side, which is invoked upon receiving the
 * stream message.
 */
static void stream_recv_cb(void *request, ucs_status_t status, size_t length,
                           void *user_data)
{
    common_cb(user_data, "stream_recv_cb", length);
}


/**
 * Progress the request until it completes.
 */
static ucs_status_t request_wait(void *request,
                                 test_req_t *ctx)
{
    ucs_status_t status;

    /* if operation was completed immediately */
    if (request == NULL) {
        return UCS_OK;
    }

    if (UCS_PTR_IS_ERR(request)) {
        return UCS_PTR_STATUS(request);
    }

    time_t timeout, now;
    time(&timeout);
    while (ctx->ready == 0) {
        ucp_worker_progress(ucp_worker);
        time(&now);
        if(1000<(now-timeout)) {
            ucp_request_free(request);
            return UCS_ERR_TIMED_OUT;
        }
    }
    status = ucp_request_check_status(request);

    ucp_request_free(request);

    return status;
}

/**
 * Sends a message to the server
 * @return
 */
int warpclient_sendMsg(ucp_dt_iov_t *buffer){
    ucp_request_param_t param;
    ucs_status_t *request;
    ucs_status_t status;
    test_req_t ctx;

    /* prepare send parametres. */
    ctx.ready       = 0;
    param.op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK |  // use a callback function
                         UCP_OP_ATTR_FIELD_DATATYPE | // used datatype is the default. Just set here, to have the option to change it.
                         UCP_OP_ATTR_FIELD_USER_DATA; // lets us give userdata to the callback function.
    param.datatype     = ucp_dt_make_contig(1);
    param.cb.send      = send_cb;

    param.user_data = &ctx;//only first field of callback data is used, because we only use one buffer, not

    // wait for the query
    request              = ucp_stream_send_nbx(client_ep, buffer->buffer, buffer->length, &param);
    status               = request_wait(request, &ctx);
    if(status != UCS_OK){
        DLOG("Something off? Status error message: %s\n", ucs_status_string(status)); // if client ends connection (with flush) an error at the server happens and the last batch does not get read!
        return -1;
    }
    return 0;
}

/**
 * Close the given endpoint.
 * Currently closing the endpoint with UCP_EP_CLOSE_MODE_FORCE since we currently
 * cannot rely on the client side to be present during the server's endpoint
 * closing process.
 */
static void ep_close()
{
    ucs_status_t status;
    void *close_req;

    close_req          = ucp_ep_close_nb(client_ep, UCP_EP_CLOSE_MODE_FORCE);
    if (UCS_PTR_IS_PTR(close_req)) {
        do {
            ucp_worker_progress(ucp_worker);
            status = ucp_request_check_status(close_req);
        } while (status == UCS_INPROGRESS);

        ucp_request_free(close_req);
    } else if (UCS_PTR_STATUS(close_req) != UCS_OK) {
        DLOG("failed to close ep. Status: %s\n", ucs_status_string(UCS_PTR_STATUS(close_req)));
    }
}


/**
 * Error handling callback. Just prints the error status in stdout.
 */
static void err_cb(void *arg, ucp_ep_h ep, ucs_status_t status)
{
   DLOG("error handling callback was invoked with status %d (%s)\n",
           status, ucs_status_string(status));
}

/**
 * Create an endpoint from the client side to be
 * connected to the remote server (to the given IP).
 */
static ucs_status_t create_client_endpoint()
{
    /*
     * params for local endpoint
     * address for bootstrap connection
     * status for function error code check
     */
    ucp_ep_params_t ep_params;
    struct sockaddr_storage connect_addr;
    ucs_status_t status;

    /* Convert given address into binary address */
    set_sock_addr(server_addr, &connect_addr);

    /*
     * Endpoint field mask bits:
     * UCP_EP_PARAM_FIELD_FLAGS             - Use the value of the 'flags' field.
     * UCP_EP_PARAM_FIELD_SOCK_ADDR         - Use a remote sockaddr to connect
     *                                        to the remote peer.
     * UCP_EP_PARAM_FIELD_ERR_HANDLING_MODE - Error handling mode - this flag
     *                                        is temporarily required since the
     *                                        endpoint will be closed with
     *                                        UCP_EP_CLOSE_MODE_FORCE which
     *                                        requires this mode.
     *                                        Once UCP_EP_CLOSE_MODE_FORCE is
     *                                        removed, the error handling mode
     *                                        will be removed.
     */
    ep_params.field_mask       = UCP_EP_PARAM_FIELD_FLAGS       |
                                 UCP_EP_PARAM_FIELD_SOCK_ADDR   |
                                 UCP_EP_PARAM_FIELD_ERR_HANDLER |
                                 UCP_EP_PARAM_FIELD_ERR_HANDLING_MODE;
    ep_params.err_mode         = UCP_ERR_HANDLING_MODE_PEER;    // alternative is NONE instead of PEER. NONE is faster but unreliable.
    ep_params.err_handler.cb   = err_cb;                        // this callback just prints error status in stdout.
    ep_params.err_handler.arg  = NULL;
    ep_params.flags            = UCP_EP_PARAMS_FLAGS_CLIENT_SERVER;  // connection gets established via sockets. Needs an address.
    ep_params.sockaddr.addr    = (struct sockaddr*)&connect_addr;    // the address
    ep_params.sockaddr.addrlen = sizeof(connect_addr);

    /* native UCP ep creation call */
    status = ucp_ep_create(ucp_worker, &ep_params, &client_ep);
    if (status != UCS_OK) {
        DLOG("failed to connect to %s (%s)\n", server_addr,
                ucs_status_string(status));
    }

    return status;
}

/**
 * Sends the sql query to the server and handles communication before data transfer.
 * @return
 */
int send_query(bool transferFormat, char *query){

    int ret = 0;
    ucp_dt_iov_t *buffer;
    uint16_t queryLength;


    /* allocate buffer.*/
    queryLength = strlen(query)+1;
    CHKERR_ACTION( NULL == (buffer = calloc(1, sizeof(ucp_dt_iov_t))),"allocate buffer struct", return -2);
    buffer->length = queryLength + 4; // query + header
    CHKERR_ACTION( NULL == (buffer->buffer = calloc(1, buffer->length)),"allocate intern buffer", free(buffer); return -2);

    DLOG("Preparing query...\n");
    //First byte 1000..  for transferFormat true or 0000.... for false.
    unsigned char byte = transferFormat ? 0x80 : 0x00;
    *(unsigned char *)buffer->buffer = byte;
    //Second byte is zeroed.
    byte = 0x00;
    *(unsigned char *) (buffer->buffer+1) = byte;
    //third and fourth byte as query length.
    *(u_int16_t *)(buffer->buffer+2) = queryLength;
    //query message fifth byte onwards.
    memcpy(buffer->buffer+4, query, queryLength);
    DLOG("Sending query...\n");
    ret = warpclient_sendMsg(buffer);
    if(ret!=0){
        DLOG("Failed to send msg.\n");
    }else {
        DLOG("Query send.\n");
    }

    free(buffer->buffer);
    free(buffer);
    return ret;
}

/**
 * returns a buffer with data. As soon as the next buffer gets requested, the old one is considered obsolete and not safe
 * to use anymore! Returns NULL, if there is no more data and the request is finished. Also releases ressources then.
 * @return
 */
void *warpclient_getData(size_t *written){
    /* ctx gets written by sending callback as soon as the send completed. */
    test_req_t ctx;

    /* Helping variables */
    ucs_status_t *request;
    ucs_status_t status;

    /* Check whether I already allocated buffer for stream recv operations. */
    if(!communicationInProgress){

        /* allocate buffer and open file handle.*/
        DLOG("First call to getData(). Allocating buffers..\n");
        CHKERR_ACTION( NULL == (recvbuffer = calloc(1, sizeof(ucp_dt_iov_t))),"allocate buffer struct", return NULL);
        CHKERR_ACTION( NULL == (recvbuffer->buffer = calloc(1, DEFAULT_BUFFER_SIZE)),"allocate intern buffer", free(recvbuffer); return NULL);
        recvbuffer->length = DEFAULT_BUFFER_SIZE;

        // communication params params
        recvparam.op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK |
                                 UCP_OP_ATTR_FIELD_DATATYPE |
                                 UCP_OP_ATTR_FIELD_USER_DATA;
        recvparam.datatype     = ucp_dt_make_contig(1);
        recvparam.user_data    = &ctx;
        recvparam.cb.recv_stream = stream_recv_cb;
        communicationInProgress = 1;
    }

    DLOG("receiving data...\n");
    ctx.ready=0;
    request              = ucp_stream_recv_nbx(client_ep, recvbuffer->buffer, DEFAULT_BUFFER_SIZE, &ctx.byteswritten, &recvparam);
    status               = request_wait(request, &ctx);
    if(status != UCS_OK){
        DLOG("Error. Status message: %s\n", ucs_status_string(status));
        return NULL;
    }
    *written = ctx.byteswritten;
    return recvbuffer->buffer;
}

int warpclient_cleanup(){
    int ret = 0;

    //free buffers
    if(communicationInProgress) {
        free(recvbuffer->buffer);
        free(recvbuffer);
    }

    /* Close the endpoint to the server */
    DLOG("Close endpoint.\n");
    ep_close();

    /* releasing UCX ressources */
    ucp_worker_destroy(ucp_worker);
    ucp_cleanup(ucp_context);
    communicationInProgress = 0;

    return ret;
}

int warpclient_queryServer(char *server_addr_local, int port, int useINet6, bool transferFormat, char *query){
    /*
     * Initialize important connection variables
     */
    DLOG("Initializing connection variables...\n");
    if(NULL != server_addr_local) server_addr = server_addr_local;
    if((port >= 0) && (port <= UINT16_MAX)) server_port = port;
    if(useINet6) ai_family = AF_INET6;

    int ret;

    /* Initialize the UCX required objects worker and context*/
    DLOG("Initializing context and worker...\n");
    ret = init_context_and_worker();
    if (ret != 0) {
        DLOG("Initializing worker or context failed! Exiting..\n");
        return -3;
    }

    /*
     * UCP objects: client_ep as communication endpoint for the worker.
     *              status for function error code check.
     */
    ucs_status_t status;

    /* ep initialization and exchange with server over sockets */
    DLOG("Creating Client endpoint.\n");
    status = create_client_endpoint();
    if (status != UCS_OK) {
        DLOG("failed to start client (%s)\n", ucs_status_string(status));
        return -2;
    }

    ret = send_query(transferFormat, query);
    if(ret!=0){
        DLOG("Failed to connect to Server.\n");
        return -1;
    }

    return ret;
}



