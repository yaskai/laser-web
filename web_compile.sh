emcc src/main.c \
    /home/kai/raylib/src/libraylib.web.a \
    -I/home/kai/raylib/src \
    -O2 \
    -DPLATFORM_WEB \
    -s USE_GLFW=3 \
    -s FULL_ES2=1 \
    -s ASYNCIFY \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s FORCE_FILESYSTEM=1 \
	-s INITIAL_MEMORY=256mb \
    -s EXPORTED_FUNCTIONS="['_main']" \
    -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
	--preload-file resources  \
	--shell-file minshell.html \
    -o index.html
