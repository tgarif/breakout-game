# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude -Iopengl/include
WINDRES = x86_64-w64-mingw32-windres

# Linux-specific settings
LDFLAGS_LINUX = -L./opengl/lib_linux -Wl,-rpath,./opengl/lib_linux -lglfw3 -lGLEW -ldl -lm -lGL -lassimp -lfreetype
BUILDDIR_LINUX = build_linux
TARGET_LINUX = $(BUILDDIR_LINUX)/breakout

# Windows-specific settings
LDFLAGS_WINDOWS = -L./opengl/lib_windows -lglfw3 -lglew32 -lopengl32 -lgdi32 -luser32 -lkernel32 -lassimp -lfreetype
BUILDDIR_WINDOWS = build_windows
TARGET_WINDOWS = $(BUILDDIR_WINDOWS)/breakout.exe

# Source files
SRCDIR = src
SRC = $(wildcard $(SRCDIR)/*.c)
TARGET_PNG = icons/breaker.png
ICO_FILE = icons/breaker.ico

# Ico resource
ICON_RC = icons/breaker.rc
ICON_RES = build_windows/breaker.res

# Build rules
all: linux windows

linux: $(BUILDDIR_LINUX) $(TARGET_LINUX)

$(BUILDDIR_LINUX):
	mkdir -p $(BUILDDIR_LINUX)

$(TARGET_LINUX): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS_LINUX)

windows: $(BUILDDIR_WINDOWS) $(TARGET_WINDOWS)

$(BUILDDIR_WINDOWS):
	mkdir -p $(BUILDDIR_WINDOWS)

$(TARGET_WINDOWS): $(SRC) $(ICON_RES)
	x86_64-w64-mingw32-gcc $(CFLAGS) $(SRC) $(ICON_RES) -o $(TARGET_WINDOWS) $(LDFLAGS_WINDOWS) -mwindows

create-ico:
	@convert $(TARGET_PNG) -resize 16x16 icons/tmp_breaker_16.png
	@convert $(TARGET_PNG) -resize 32x32 icons/tmp_breaker_32.png
	@convert $(TARGET_PNG) -resize 48x48 icons/tmp_breaker_48.png
	@icotool -c -o $(ICO_FILE) icons/tmp_breaker_16.png icons/tmp_breaker_32.png icons/tmp_breaker_48.png $(TARGET_PNG)
	@rm -f icons/tmp_breaker_*.png

$(ICON_RES): $(ICON_RC)
	$(WINDRES) $(ICON_RC) -O coff -o $(ICON_RES)

run_linux:
	./$(TARGET_LINUX)

run_linux_debug:
	gdb ./$(TARGET_LINUX)

run_windows:
	wine ./$(TARGET_WINDOWS)

# Clean rule
clean:
	rm -rf $(BUILDDIR_LINUX) $(BUILDDIR_WINDOWS)

rebuild: clean all

.PHONY: all clean rebuild linux windows run_linux run_windows run_linux_debug
