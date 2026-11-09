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
    WASM_OPT_FLAGS := -O0 -g
else
    WASM_OPT_FLAGS := -O3 -DNDEBUG
endif

EM_COMPILE_FLAGS := -r -std=c++17 $(WASM_OPT_FLAGS) \
                    -I$(CUBE_BUILD_INCLUDE_DIR) \
                    -DCUBE_USE_SYMMETRY=0 \
                    -DCUBE_ENABLE_TRACE=1

EM_LINK_BASE     := -lembind $(WASM_OPT_FLAGS) \
                    -s EXPORT_NAME=$(EXPORT_NAME) \
                    -s EXPORT_ES6=1 \
                    -s FORCE_FILESYSTEM=1 \
                    -s ALLOW_MEMORY_GROWTH=1

# Output Targets
WASM_OBJ := $(DIST_DIR)/cube_wasm.o
WEB_ESM  := $(INTERNAL_DIR)/jscube.web.mjs
NODE_ESM := $(INTERNAL_DIR)/jscube.node.mjs

# ==============================================================================
# Build Rules
# ==============================================================================
.PHONY: all prepare clean

all: $(WEB_ESM) $(NODE_ESM)
	@rm -f $(WASM_OBJ)

prepare:
	@mkdir -p "$(INTERNAL_DIR)"

$(WASM_OBJ): $(WASM_SOURCES) | prepare
	$(EMCC) $(WASM_SOURCES) $(EM_COMPILE_FLAGS) -o $@

# for Web environment
$(WEB_ESM): $(WASM_OBJ)
	$(EMCC) $< $(EM_LINK_BASE) \
    -s ENVIRONMENT=web -s EXPORTED_RUNTIME_METHODS=['FS'] \
    -o $@

# for Node environment
$(NODE_ESM): $(WASM_OBJ)
	$(EMCC) $< $(EM_LINK_BASE) -lnodefs.js \
    -s ENVIRONMENT=node -s EXPORTED_RUNTIME_METHODS=['FS','NODEFS'] \
    -o $@

clean:
	rm -rf "$(DIST_DIR)"