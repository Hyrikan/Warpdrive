#!/bin/bash
# this script copies the libraries into the ubuntu standart directories.

cp -r warpdrive /usr/lib/
cp warp_client.h warp_server.h /usr/include/
cp -r ucx/include/* /usr/include
cp -r ucx/lib /usr/lib/ucx
