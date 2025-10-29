JS_TARGET="jscube.mjs"
MODULE_NAME="createCubeModule"

CXX_FLAGS += -std=c++17 -DNDEBUG -O2
INCLUDE += -I../../include
EM_FLAGS += -s MODULARIZE=1 \
			-s EXPORT_NAME=$(MODULE_NAME)


libcube_sources = $(shell find ../../src -name '*.cpp')

.PHONY: all clean

all: jscube 

jscube: cube_wasm.cpp $(libcube_sources) 
	emcc $^ $(CXX_FLAGS) $(EM_FLAGS) $(INCLUDE) -lembind -o $(JS_TARGET)

clean: 
	rm -f *.o $(JS_TARGET)
