#!/bin/bash
source /root/emsdk_portable/emsdk_env.sh
rm vtexInfo.js
#emcc mdlInfo.cpp -O2 -o mdlInfo.js --embed-file testbin/axe.dx90.vtx
emcc vtexInfo.cpp -o vtexInfo.js --embed-file testbin/
node vtexInfo.js