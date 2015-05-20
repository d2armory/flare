all: flare

flare: main.bc esUtil.bc glm
	emcc main.bc es/esUtil.bc -o /usr/share/nginx/html/app/raw.html

main.bc: main.cpp
	emcc main.cpp -o main.bc
	
esUtil.bc: es/esUtil.h es/esUtil.c
	emcc es/esUtil.c -o es/esUtil.bc
	
glm: glm/common.hpp
	echo GLM needs no armor
	
test: test.cpp
	emcc test.cpp -o test.js

clean:
	rm *.bc