ROOT      := $(shell git rev-parse --show-toplevel)
WASM_DIR  := $(ROOT)/bindings/wasm

# ==============================================================================
# Compiler and Configuration Flags
# ==============================================================================
EMCC                   ?= em++
BUILD_TYPE             ?= Release
DIST_DIR               ?= $(WASM_DIR)/dist
INTERNAL_DIR           := $(DIST_DIR)/internal
CUBE_BUILD_INCLUDE_DIR ?= $(ROOT)/build/include
CPP_SOURCES            ?= $(ROOT)/amalg/cube_amalg.min.cpp

EXPORT_NAME  := createModule
WASM_SOURCES := $(WASM_DIR)/native/cube_wasm.cpp $(CPP_SOURCES)

ifeq ($(BUILD_TYPE),Debug)
    WASM_OPT_FLAGS := -O0
else
    WASM_OPT_FLAGS := -O2 -DNDEBUG
endif

EM_COMPILE_FLAGS := -r -std=c++17 -I$(CUBE_BUILD_INCLUDE_DIR) $(WASM_OPT_FLAGS)
EM_LINK_FLAGS    := -lembind -s EXPORT_NAME=$(EXPORT_NAME) $(WASM_OPT_FLAGS)

# Output Targets
WASM_OBJ := $(DIST_DIR)/cube_wasm.o
WEB_MJS  := $(INTERNAL_DIR)/jscube.web.mjs
NODE_CJS := $(INTERNAL_DIR)/jscube.node.cjs
NODE_MJS := $(INTERNAL_DIR)/jscube.node.mjs

# ==============================================================================
# Build Rules
# ==============================================================================
.PHONY: all prepare clean

all: $(WEB_MJS) $(NODE_CJS) $(NODE_MJS)
	@rm -f $(WASM_OBJ)
	
prepare:
	@mkdir -p "$(INTERNAL_DIR)"

# 1. Compile C++ sources into an intermediate object file
$(WASM_OBJ): $(WASM_SOURCES) | prepare
	$(EMCC) $(WASM_SOURCES) $(EM_COMPILE_FLAGS) -o $@

# 2. Link for Web environment (ES Module)
$(WEB_MJS): $(WASM_OBJ)
	$(EMCC) $< $(EM_LINK_FLAGS) -s ENVIRONMENT=web -s EXPORT_ES6=1 -o $@

# 3. Link for Node environment (CommonJS)
$(NODE_CJS): $(WASM_OBJ)
	$(EMCC) $< $(EM_LINK_FLAGS) -s ENVIRONMENT=node -s MODULARIZE=1 -s WASM_ASYNC_COMPILATION=0 -o $@

# 4. Generate Node ES Module wrapper for CJS output
$(NODE_MJS): | prepare
	@printf 'import { createRequire } from "module";\nconst require = createRequire(import.meta.url);\nexport default require("./jscube.node.cjs");\n' > $@

clean:
	rm -rf "$(DIST_DIR)"
