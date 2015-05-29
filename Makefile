SHELL := /bin/bash
PATH  := /root/emsdk_portable:/root/emsdk_portable/clang/fastcomp/build_master_64/bin:/root/emsdk_portable/emscripten/master:$(PATH)

COMPILER = emcc
#OPTIMIZE = -s DEMANGLE_SUPPORT=1 -Werror -s ASSERTIONS=2
OPTIMIZE = -Werror

all: flare

flare: main.bc es/esUtil.bc glm.bc assets.bc engine.bc squish.bc
	$(COMPILER) $(OPTIMIZE) main.bc es/esUtil.bc engine.bc squish.bc -o /usr/share/nginx/html/app/raw.html --preload-file assets
	sed -i 's/div\.emscripten_border { border: 1px solid black;/div\.emscripten_border { border: 1px solid black; background: url(http:\/\/104\.236\.208\.106\/app\/Black_Texture___Ray_by_Ethenyl\.jpg) center bottom;/' /usr/share/nginx/html/app/raw.html
	sed -i 's/alpha:\s*false/alpha:true/g' /usr/share/nginx/html/app/raw.js
	sed -i 's/backgroundColor\s*=\s*"black"/backgroundColor="rgba(0,0,0,0)"/g' /usr/share/nginx/html/app/raw.js

main.bc: main.cpp
	$(COMPILER) $(OPTIMIZE) main.cpp -o main.bc
	
es/esUtil.bc: es/esUtil.h es/esUtil.c
	$(COMPILER) $(OPTIMIZE) es/esUtil.c -o es/esUtil.bc
	
engine.bc: engine/fileLoader.cpp engine/fileLoader.hpp engine/material.cpp engine/material.hpp engine/model.cpp engine/model.hpp engine/texture.cpp engine/texture.hpp engine/manager.cpp engine/manager.hpp engine/shader.cpp engine/shader.hpp engine/heroshader.cpp engine/heroshader.hpp engine/shadowshader.cpp engine/shadowshader.hpp engine/kvreader.hpp engine/kvreader.cpp engine/scene.hpp engine/scene.cpp
	$(COMPILER) $(OPTIMIZE) engine/manager.cpp -o engine/manager.bc
	$(COMPILER) $(OPTIMIZE) engine/texture.cpp -o engine/texture.bc
	$(COMPILER) $(OPTIMIZE) engine/model.cpp -o engine/model.bc
	$(COMPILER) $(OPTIMIZE) engine/material.cpp -o engine/material.bc
	$(COMPILER) $(OPTIMIZE) engine/fileLoader.cpp -o engine/fileLoader.bc
	$(COMPILER) $(OPTIMIZE) engine/shader.cpp -o engine/shader.bc
	$(COMPILER) $(OPTIMIZE) engine/heroshader.cpp -o engine/heroshader.bc
	$(COMPILER) $(OPTIMIZE) engine/shadowshader.cpp -o engine/shadowshader.bc
	$(COMPILER) $(OPTIMIZE) engine/kvreader.cpp -o engine/kvreader.bc
	$(COMPILER) $(OPTIMIZE) engine/scene.cpp -o engine/scene.bc
	$(COMPILER) $(OPTIMIZE) engine/fileLoader.bc engine/material.bc engine/model.bc engine/texture.bc engine/manager.bc engine/shader.bc engine/heroshader.bc engine/shadowshader.bc engine/kvreader.bc engine/scene.bc -o engine.bc
		
squish.bc: squish/squish.h
	$(COMPILER) $(OPTIMIZE) squish/alpha.cpp -o squish/alpha.bc
	$(COMPILER) $(OPTIMIZE) squish/clusterfit.cpp -o squish/clusterfit.bc
	$(COMPILER) $(OPTIMIZE) squish/colourblock.cpp -o squish/colourblock.bc
	$(COMPILER) $(OPTIMIZE) squish/colourfit.cpp -o squish/colourfit.bc
	$(COMPILER) $(OPTIMIZE) squish/colourset.cpp -o squish/colourset.bc
	$(COMPILER) $(OPTIMIZE) squish/maths.cpp -o squish/maths.bc
	$(COMPILER) $(OPTIMIZE) squish/rangefit.cpp -o squish/rangefit.bc
	$(COMPILER) $(OPTIMIZE) squish/singlecolourfit.cpp -o squish/singlecolourfit.bc
	$(COMPILER) $(OPTIMIZE) squish/squish.cpp -o squish/squish.bc
	$(COMPILER) $(OPTIMIZE) squish/alpha.bc squish/clusterfit.bc squish/colourblock.bc squish/colourfit.bc squish/colourset.bc squish/maths.bc squish/rangefit.bc squish/singlecolourfit.bc squish/squish.bc -o squish.bc
	
glm.bc: glm/common.hpp
	echo GLM needs no armor > /dev/null
	touch glm.bc
	
assets.bc: assets/shader.frag assets/shader.vert
	echo Assets also need no armors > /dev/null
	touch assets.bc
	
test: test.cpp
	$(COMPILER) test.cpp -o test.js

clean:
	find . -name "*.bc" -type f -delete