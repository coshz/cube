ROOT := $(shell git rev-parse --show-toplevel)

# ==============================================================================
# Project Configurations
# ==============================================================================
PYTHON       ?= python3
PROJECT_NAME := cube
VERSION      := 0.4.0
LICENSE      := MIT

AMALG_DIR     := $(ROOT)/amalg
SCRIPT_AMALG  := $(ROOT)/dev/scripts/create_amalg.py
SCRIPT_HEADER := $(ROOT)/dev/scripts/update_header.py

# Input Sources & Includes
SOURCES := $(addprefix $(ROOT)/, \
               src/rubik.cpp \
               src/coord.cpp \
               src/table.cpp \
               src/twophase.cpp \
               src/cube.cpp \
               src/internal.cpp \
           )

INCLUDES := $(ROOT)/build/include

# Target Outputs
AMALG_CPP     := $(AMALG_DIR)/cube_amalg.cpp
AMALG_MIN_CPP := $(AMALG_DIR)/cube_amalg.min.cpp

# ==============================================================================
# Build Rules
# ==============================================================================
.PHONY: all prepare clean

# Entry target: Generate both cpp files and then update the header comments
all: $(AMALG_CPP) $(AMALG_MIN_CPP)
	$(PYTHON) $(SCRIPT_HEADER) $(AMALG_DIR) \
		--project-name $(PROJECT_NAME) \
		--version $(VERSION) \
		--license $(LICENSE)

prepare:
	@mkdir -p "$(AMALG_DIR)"

# 1. Generate full amalgam source
$(AMALG_CPP): $(SOURCES) $(SCRIPT_AMALG) | prepare
	$(PYTHON) $(SCRIPT_AMALG) $(SOURCES) -I $(INCLUDES) -o $@

# 2. Generate minified amalgam source (-c option)
$(AMALG_MIN_CPP): $(SOURCES) $(SCRIPT_AMALG) | prepare
	$(PYTHON) $(SCRIPT_AMALG) $(SOURCES) -I $(INCLUDES) -c -o $@

clean:
	rm -f $(AMALG_CPP) $(AMALG_MIN_CPP)
