#!/bin/bash
source /root/emsdk_portable/emsdk_env.sh
rm mdlInfo.js
emcc mdlInfo.cpp -o mdlInfo.js --embed-file testbin/axe.mdl
node mdlInfo.js