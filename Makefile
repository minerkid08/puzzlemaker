src := puzzlemaker/src
build := build
libDir := ../libs
puzBuild := $(build)/puzzlemaker

inc := -Ipuzzlemaker/src \
			-I../libs/libzip \
			-I../libs/lua/export/lua \
			-I../libs/lua/export \
			-I../libs/cglm/src \
			-I../libs/glad/src \
			-I../libs/clib/src \
			-I../libs/cjson/src \
			-I../libs/stb \
			-I../libs/cimgui/src \
			-I../libs/glfw/export

libs := $(libDir)/cimgui/cimgui.lib \
				$(libDir)/glfw/glfw3.lib \
				$(libDir)/libzip/libzip.lib \
				-ldbghelp

srcFiles = $(shell find $(src) -name "*.c")
luaFiles = $(shell find $(libDir)/lua/src -name "*.c")
gladFiles = $(shell find $(libDir)/glad/src/glad -name "*.c")
cjsonFiles = $(shell find $(libDir)/cjson/src -name "*.c")
clibFiles = $(shell find $(libDir)/clib/src -name "*.c")

objFiles = $(patsubst $(src)/%.c, $(puzBuild)/%.o, $(srcFiles))
luaObjFiles = $(patsubst $(libDir)/%.c, $(build)/%.o, $(luaFiles))
gladObjFiles = $(patsubst $(libDir)/%.c, $(build)/%.o, $(gladFiles))
cjsonObjFiles = $(patsubst $(libDir)/%.c, $(build)/%.o, $(cjsonFiles))
clibObjFiles = $(patsubst $(libDir)/%.c, $(build)/%.o, $(clibFiles))

dFiles = $(patsubst $(src)/%.c, $(puzBuild)/%.d, $(srcFiles))
luaDFiles = $(patsubst $(libDir)/%.c, $(build)/%.d, $(luaFiles))
gladDFiles = $(patsubst $(libDir)/%.c, $(build)/%.d, $(gladFiles))
cjsonDFiles = $(patsubst $(libDir)/%.c, $(build)/%.d, $(cjsonFiles))
clibDFiles = $(patsubst $(libDir)/%.c, $(build)/%.d, $(clibFiles))

all: $(build)/puzzlemaker.exe

$(build)/puzzlemaker.exe: $(objFiles) $(luaObjFiles) $(gladObjFiles) $(cjsonObjFiles) $(clibObjFiles)
	clang --target=x86_64-windows-msvc -fuse-ld=lld -g $^ $(libs) -o $(build)/puzzlemaker.exe

$(puzBuild)/%.o: $(src)/%.c | $(build)
	clang --target=x86_64-windows-msvc $(inc) -g -c $< -o $@ -Wno-deprecated-declarations -MMD -MP

$(build)/%.o: $(libDir)/%.c | $(build)
	clang --target=x86_64-windows-msvc $(inc) -g -c $< -o $@ -Wno-deprecated-declarations -MMD -MP

$(build):
	mkdir -p $(build)/puzzlemaker
	mkdir -p $(build)/puzzlemaker/antline
	mkdir -p $(build)/puzzlemaker/export
	mkdir -p $(build)/puzzlemaker/item
	mkdir -p $(build)/puzzlemaker/packages
	mkdir -p $(build)/puzzlemaker/compile
	mkdir -p $(build)/puzzlemaker/ui
	mkdir -p $(build)/puzzlemaker/voxel
	mkdir -p $(build)/puzzlemaker/renderer
	mkdir -p $(build)/puzzlemaker/export/script
	mkdir -p $(build)/lua/src
	mkdir -p $(build)/glad/src/glad
	mkdir -p $(build)/clib/src/math
	mkdir -p $(build)/cjson/src

clean:
	rm -r $(build)

-include $(dFiles) $(luaDFiles) $(gladDFiles) $(cjsonDFiles) $(libDFiles)
