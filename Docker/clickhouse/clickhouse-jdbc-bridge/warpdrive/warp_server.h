//
// Created by joel on 23.05.22.
//

#ifndef TEST_FIELD_UCP_WARP_SERVER_H
#define TEST_FIELD_UCP_WARP_SERVER_H

#include <ucp/api/ucp.h>
#include <string.h>    /* memset */
#include <arpa/inet.h> /* inet_addr */
#include <unistd.h>    /* getopt */
#include <stdlib.h>    /* atoi */
#include <pthread.h>
#include <stdbool.h>


#define DEFAULT_PORT           13337
#define IP_STRING_LEN          50
#define PORT_STRING_LEN        8
#define DEFAULT_BUFFER_SIZE    4096
#define DEFAULT_BUFFER_COUNT   5
#define TRANSFERFORMAT_BITMASK 0x80


/**
 * Callback function for querying the local database.
 *
 * The server calls this function with the querystring and subsequently calls the data callback function for the results of the query.
 * May return error codes.
 */
typedef int (*warpserver_cb_query)(bool transferFormat, char *querystring);

/**
 * Callback function for accessing the results of a previous query call.
 *
 * The server expects the function to store the address for data in void *data and the size of the data at size_t length.
 * It then populates it's own send buffer with the data and potentially data from subsequent calls to this callback.
 * It expects a NULL pointer at data if all data got consumed. In this case, the callback also releases data.
 */
typedef void (*warpserver_cb_data)(void **data, size_t *length);

/**
 * char *listen_addr_local specifies listen address.
 * If not specified, server uses INADDR_ANY.
 *
 * int port specifies local listen port.
 * If not specified, server uses DEFAULT_PORT, as specified above.
 *
 * int useINet6 specifies, whether an ipv6 address shall be used.
 */
int warpserver_initialize(char *listen_addr_local, int port, int useINet6, warpserver_cb_query querycb, warpserver_cb_data datacb);

int warpserver_start();
int warpserver_stop();






#endif //TEST_FIELD_UCP_WARP_SERVER_H
