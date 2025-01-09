# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude -Iopengl/include

# Linux-specific settings
LDFLAGS_LINUX = -L./opengl/lib_linux -Wl,-rpath,./opengl/lib_linux -lglfw3 -lGLEW -ldl -lm -lGL -lassimp
BUILDDIR_LINUX = build_linux
TARGET_LINUX = $(BUILDDIR_LINUX)/breakout

# Windows-specific settings
LDFLAGS_WINDOWS = -L./opengl/lib_windows -lglfw3 -lglew32 -lopengl32 -lgdi32 -luser32 -lkernel32 -lassimp
BUILDDIR_WINDOWS = build_windows
TARGET_WINDOWS = $(BUILDDIR_WINDOWS)/breakout.exe

# Source files
SRCDIR = src
SRC = $(wildcard $(SRCDIR)/*.c)

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

$(TARGET_WINDOWS): $(SRC)
	x86_64-w64-mingw32-gcc $(CFLAGS) $^ -o $@ $(LDFLAGS_WINDOWS) -mwindows

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
