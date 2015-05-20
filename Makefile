SHELL := /bin/bash
PATH  := /root/emsdk_portable:/root/emsdk_portable/clang/fastcomp/build_master_64/bin:/root/emsdk_portable/emscripten/master:$(PATH)

COMPILER = emcc
OPTIMIZE = -O2

all: flare

flare: main.bc es/esUtil.bc glm.bc
	$(COMPILER) $(OPTIMIZE) main.bc es/esUtil.bc -o /usr/share/nginx/html/app/raw.html

main.bc: main.cpp
	$(COMPILER) $(OPTIMIZE) main.cpp -o main.bc
	
es/esUtil.bc: es/esUtil.h es/esUtil.c
	$(COMPILER) $(OPTIMIZE) es/esUtil.c -o es/esUtil.bc
	
glm.bc: glm/common.hpp
	echo GLM needs no armor
	touch glm.bc
	
test: test.cpp
	$(COMPILER) test.cpp -o test.js

clean:
	find . -name "*.bc" -type f -delete