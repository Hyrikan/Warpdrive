# This script copies relevant files for compilation into the clickhouse docker folder.
#!/bin/bash

cp -r absl cityhash lz4 clickhouse CMakeLists.txt main.cpp main.h libclickhouse-cpp-lib.so liblz4-lib.a libcityhash-lib.a libabsl-lib.a ../../Docker/clickhouse/clickhouse-jdbc-bridge/warpServer
