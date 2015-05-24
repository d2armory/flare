SHELL := /bin/bash
PATH  := /root/emsdk_portable:/root/emsdk_portable/clang/fastcomp/build_master_64/bin:/root/emsdk_portable/emscripten/master:$(PATH)

COMPILER = emcc
OPTIMIZE = 

all: flare

flare: main.bc es/esUtil.bc glm.bc assets.bc engine.bc
	$(COMPILER) $(OPTIMIZE) main.bc es/esUtil.bc engine.bc -o /usr/share/nginx/html/app/raw.html --preload-file assets

main.bc: main.cpp
	$(COMPILER) $(OPTIMIZE) main.cpp -o main.bc
	
es/esUtil.bc: es/esUtil.h es/esUtil.c
	$(COMPILER) $(OPTIMIZE) es/esUtil.c -o es/esUtil.bc
	
engine.bc: engine/fileLoader.bc engine/material.bc engine/model.bc engine/texture.bc engine/manager.bc
	$(COMPILER) $(OPTIMIZE) engine/fileLoader.bc engine/material.bc engine/model.bc engine/texture.bc engine/manager.bc -o engine.bc
		
engine/fileLoader.bc: engine/fileLoader.cpp engine/fileLoader.hpp
	$(COMPILER) $(OPTIMIZE) engine/fileLoader.cpp -o engine/fileLoader.bc

engine/material.bc: engine/material.cpp engine/material.hpp
	$(COMPILER) $(OPTIMIZE) engine/material.cpp -o engine/material.bc
	
engine/model.bc: engine/model.cpp engine/model.hpp
	$(COMPILER) $(OPTIMIZE) engine/model.cpp -o engine/model.bc
	
engine/texture.bc: engine/texture.cpp engine/texture.hpp
	$(COMPILER) $(OPTIMIZE) engine/texture.cpp -o engine/texture.bc
	
engine/manager.bc: engine/manager.cpp engine/manager.hpp
	$(COMPILER) $(OPTIMIZE) engine/manager.cpp -o engine/manager.bc
	
glm.bc: glm/common.hpp
	echo GLM needs no armor
	touch glm.bc
	
assets.bc: assets/shader.frag assets/shader.vert
	echo Assets also need no armors
	touch assets.bc
	
test: test.cpp
	$(COMPILER) test.cpp -o test.js

clean:
	find . -name "*.bc" -type f -delete