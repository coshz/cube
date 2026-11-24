ROOT := $(shell git rev-parse --show-toplevel)

# ==============================================================================
# Project Configurations
# ==============================================================================
PYTHON		?= python3
PROJECT_NAME 	:= cube
VERSION		:= $(strip $(shell cat $(ROOT)/VERSION))
LICENSE		:= MIT

AMALG_DIR	:= $(ROOT)/amalg
SCRIPT_AMALG	:= $(ROOT)/dev/scripts/py/create_amalg.py
SCRIPT_HEADER	:= $(ROOT)/dev/scripts/py/update_header.py

# Input Sources & Includes
INCLUDE := $(ROOT)/build/include
SOURCES := $(wildcard $(ROOT)/src/*.cpp)
HEADERS := $(wildcard $(ROOT)/src/*.hh) $(wildcard $(ROOT)/src/*.hpp) \
	   $(wildcard $(INCLUDE)/cube/*.h) $(wildcard $(INCLUDE)/cube/*.hh)

# Target Outputs
AMALG_CPP	:= $(AMALG_DIR)/cube_amalg.cpp
AMALG_MIN_CPP	:= $(AMALG_DIR)/cube_amalg.min.cpp

# ==============================================================================
# Build Rules
# ==============================================================================
.PHONY: all prepare clean

all: $(AMALG_CPP) $(AMALG_MIN_CPP)

prepare:
	@mkdir -p "$(AMALG_DIR)"

# 1. Generate full amalgam source & Add header
$(AMALG_CPP): $(SOURCES) $(HEADERS) $(SCRIPT_AMALG) $(SCRIPT_HEADER) | prepare
	$(PYTHON) $(SCRIPT_AMALG) $(SOURCES) -I $(INCLUDE) -o $@
	$(PYTHON) $(SCRIPT_HEADER) $@ \
	    --project-name $(PROJECT_NAME) \
	    --version $(VERSION) \
	    --license $(LICENSE)

# 2. Generate minified amalgam source & Add header
$(AMALG_MIN_CPP): $(SOURCES) $(HEADERS) $(SCRIPT_AMALG) $(SCRIPT_HEADER) | prepare
	$(PYTHON) $(SCRIPT_AMALG) $(SOURCES) -I $(INCLUDE) -c -o $@
	$(PYTHON) $(SCRIPT_HEADER) $@ \
	    --project-name $(PROJECT_NAME) \
	    --version $(VERSION) \
	    --license $(LICENSE)

clean:
	rm -f $(AMALG_CPP) $(AMALG_MIN_CPP)
