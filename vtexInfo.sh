#!/bin/bash
source /root/emsdk_portable/emsdk_env.sh
rm vtexInfo.js
#emcc mdlInfo.cpp -O2 -o mdlInfo.js --embed-file testbin/axe.dx90.vtx
emcc vtexInfo.cpp engine/kvreader2.cpp -O2 -o vtexInfo.js -s TOTAL_MEMORY=128777216 --embed-file testbin/
node vtexInfo.js