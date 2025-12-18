TARGET_WEB=jscube.web.mjs
TARGET_NODE_CJS=jscube.node.cjs
TARGET_NODE_WRAPPER=jscube.node.mjs

DIST_DIR=dist
MODULE_NAME=createModule

CUBE_INCLUDE=../../include
CUBE_SOURCE=../../src

CXX_FLAGS += -std=c++17 -DNDEBUG -O2 -I$(CUBE_INCLUDE) -lembind

EMCC=emcc
EM_FLAGS_WEB = -s EXPORT_NAME=$(MODULE_NAME) \
	-s EXPORT_ES6=1\
	-s ENVIRONMENT=web
EM_FLAGS_NODE = -s EXPORT_NAME=$(MODULE_NAME) \
	-s MODULARIZE=1\
	-s ENVIRONMENT=node\
	-s WASM_ASYNC_COMPILATION=0

cpp_sources = \
	cube_wasm.cpp \
	$(shell find $(CUBE_SOURCE) -name '*.cpp')

.PHONY: prepare all clean web node 

prepare:
	@mkdir -p $(DIST_DIR)
	@command -v $(EMCC) > /dev/null 2>&1 || { echo \
'Error: the script exits since "emcc" is not found in your system.'; \
	exit 1; }

all: web node 

web: | prepare
	$(EMCC) $(cpp_sources) $(CXX_FLAGS) $(EM_FLAGS_WEB) -o $(DIST_DIR)/$(TARGET_WEB)

node: | prepare 
# it seems emscripten cannot generate usable esm for node environment;
# so I use cjs as the intermediate file. 
	$(EMCC) $(cpp_sources) $(CXX_FLAGS) $(EM_FLAGS_NODE) -o $(DIST_DIR)/$(TARGET_NODE_CJS)
	$(MAKE) $(DIST_DIR)/$(TARGET_NODE_WRAPPER)

$(DIST_DIR)/$(TARGET_NODE_WRAPPER): | prepare
	@echo 'import { createRequire } from "module";' > $@
	@echo 'const require = createRequire(import.meta.url);' >> $@
	@echo 'export default require("./$(TARGET_NODE_CJS)");' >> $@

clean: 
	rm -rf $(DIST_DIR)

