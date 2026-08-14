# Builds the Navier-Stokes solver into an ES module for the SvelteKit app.
#
#   make          # optimised build -> ../src/lib/wasm/fluid.js
#   make clean
#
# Requires Emscripten. On Arch, `pacman -S emscripten` installs to
# /usr/lib/emscripten without putting it on PATH, so we fall back to that.

EMXX := $(shell command -v em++ 2>/dev/null || echo /usr/lib/emscripten/em++)

OUT := ../src/lib/wasm/fluid.js
SRC := fluid.cpp

# SINGLE_FILE embeds the wasm as base64 inside the .js, so the build produces
# exactly one committed artifact -- no runtime fetch, no asset path handling in
# Vite, and nothing to 404 on a static host.
FLAGS := \
	-O3 \
	-std=c++20 \
	-ffast-math \
	-sMODULARIZE=1 \
	-sEXPORT_ES6=1 \
	-sENVIRONMENT=web \
	-sSINGLE_FILE=1 \
	-sALLOW_MEMORY_GROWTH=0 \
	-sINITIAL_MEMORY=16MB \
	-sFILESYSTEM=0 \
	-sEXPORTED_RUNTIME_METHODS=HEAPU8 \
	-sEXPORTED_FUNCTIONS=_fluid_init,_fluid_reset,_fluid_set_params,_fluid_splat,_fluid_step,_fluid_render,_fluid_size

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC) Makefile
	@mkdir -p $(dir $(OUT))
	$(EMXX) $(FLAGS) $(SRC) -o $(OUT)
	@echo "built $(OUT) ($$(du -h $(OUT) | cut -f1))"

clean:
	rm -f $(OUT)
