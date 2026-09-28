PKG_CONFIG  ?= pkg-config

.PHONY: all clean 
all: out 
out: eg.c
	gcc $(shell pkg-config --cflags cube) $^ -o $@ \
		$(shell pkg-config --libs cube) \
		-Wl,-rpath,$(shell pkg-config --variable=libdir cube)

# --- Static linking ---
# Static linking embeds libcube.a into the executable, but the C++ runtime (
# libstdc++ / libc++) is still linked dynamically. 
# We link with g++ to avoid passing -lstdc++ / -lc++ explicitly to gcc. 
# 
# 	gcc $(shell $(PKG_CONFIG) --static --cflags cube) \
# 		$(shell $(PKG_CONFIG) --variable=libdir cube)/libcube.a  \
# 		$< -lc++ -o $@
# OR:
# 	gcc -c $(shell $(PKG_CONFIG) --static --cflags cube) $< -o eg.o
# 	g++ eg.o $(shell $(PKG_CONFIG) --variable=libdir cube)/libcube.a -o $@

clean:
	rm -f eg.o out