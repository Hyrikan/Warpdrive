/*
 * pg_warpserver.h
 *  warpserver for postgres. Uses libpq to query a local postgres server to answer incoming warpclient calls.
 *  only returns data as either plain postgres data ("row oriented") or as clickhouse data ("column oriented").
 * Created by joel on 21.06.22.
 */
/* Debug mode flag */
//#define DEBUG

/* Macro to make conditional DEBUG more terse
 * Usage: elog(String); output can be found in console*/
#ifdef DEBUG
#define elog_debug(...) fprintf(stderr, __VA_ARGS__)
#else
#define elog_debug(...) ((void) 0)
#endif


#ifndef PG_WARPFDW_PG_WARPSERVER_H
#define PG_WARPFDW_PG_WARPSERVER_H

#include <warp_server.h>
#include <libpq-fe.h>
#include <stdio.h>
#include <arrow-glib/arrow-glib.h>

#include <stdbool.h>

/* for ntohl/htonl */
#include <netinet/in.h>
#include <arpa/inet.h>

/* pg2arrow*/
//#include "pg2arrow.c"

#endif //PG_WARPFDW_PG_WARPSERVER_H
