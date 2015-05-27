#!/bin/bash
source /root/emsdk_portable/emsdk_env.sh
rm mdlInfo.js
emcc mdlInfo.cpp -O2 -o mdlInfo.js --embed-file testbin/axe.dx90.vtx
node mdlInfo.js