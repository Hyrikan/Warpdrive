//
// Created by joel on 23.08.22.
//

#ifndef CLICK_WARPSERVER_CPP_MAIN_H
#define CLICK_WARPSERVER_CPP_MAIN_H

#define DEBUG

#include <iostream>
#include "clickhouse/client.h"
#include <queue>

extern "C"{
#include <warp_server.h>
};

#ifdef DEBUG
#define DLOG(x) do { std::cerr << x << std::endl; } while (0)
#else
#define DLOG(x)
#endif

#endif //CLICK_WARPSERVER_CPP_MAIN_H
