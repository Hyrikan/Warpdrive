/**
 * @file warp_client.h
 * @brief Delivers functions for connecting and requesting data from a warpServer.
 *
 * The warpClient delivers functions for connecting and requesting data from
 * a warpServer. The workflow starts by connecting to the server via the function
 * warpclient_queryServer(), which sends a query to the server. Subsequently,
 * calls to warpclient_getData() are filling buffers until a NULL is returned,
 * which indicates the end of the data transfer. Lastly, a call to warpclient_cleanup()
 * releases all allocated buffers and structs.
 *
 * Created by joel Ziegler on 23.05.22.
 */

#ifndef TEST_FIELD_UCP_WARP_CLIENT_H
#define TEST_FIELD_UCP_WARP_CLIENT_H

#include <ucp/api/ucp.h>
#include <string.h>    /* memset */
#include <arpa/inet.h> /* inet_addr */
#include <unistd.h>    /* getopt */
#include <stdlib.h>    /* atoi */
#include <stdbool.h>

#define DEFAULT_PORT           13337
#define IP_STRING_LEN          50
#define PORT_STRING_LEN        8
#define TAG                    0xCAFE
#define COMM_TYPE_DEFAULT      "STREAM"
#define PRINT_INTERVAL         2000
#define TEST_AM_ID             0
#define DEFAULT_BUFFER_SIZE    4096
#define DEFAULT_BUFFER_COUNT   5



/** Initializes a connection to a warpserver and sends the specified query.
 * @param server_addr_local - IP address string. Defaults to "localhost" if NULL.
 * @param port - Port as Integer between 0 and 65535. Defaults to 13337 if out of range.
 * @param useINet6 - Flag to use IPv6 address String.
 * @param transferFormat - Determines the format in which data is received. True for row, False for column format.
 * @param query - SQL query as String.
 * @return Returns error codes:
 *              -3: Failed to initialize UCP context or worker.
 *              -2: Failed to start client.
 *              -1: Failed to connect to server.
 */
int warpclient_queryServer(char *server_addr_local, int port, int useINet6, bool transferFormat,char *query);

/**
 * Returns a buffer with data, length is in the variable, or NULL if all data received.
 * Currently, each call to warpclient_getData() unvalidates the previous buffer!
 * @param written - contains length of the returned buffer.
 * @return Returns a void * buffer with result data, if data left. Otherwise returns NULL.
 */
void *warpclient_getData(size_t *written);

/**
 * Releases ressources from the data transfer.
 */
int warpclient_cleanup();


#endif //TEST_FIELD_UCP_WARP_CLIENT_H
