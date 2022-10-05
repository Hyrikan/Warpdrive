# this script copies relevant files for the standalone warp server compilation to the docker directory of postgres
#!/bin/bash

cp CMakeLists.txt main.c pg_warpserver.h ../../MyDockers/postgres/warpServer
