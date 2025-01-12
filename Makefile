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

# Source and object files
SRCDIR = src
OBJDIR_LINUX = $(BUILDDIR_LINUX)/obj
OBJDIR_WINDOWS = $(BUILDDIR_WINDOWS)/obj
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ_LINUX = $(SRC:$(SRCDIR)/%.c=$(OBJDIR_LINUX)/%.o)
OBJ_WINDOWS = $(SRC:$(SRCDIR)/%.c=$(OBJDIR_WINDOWS)/%.o)

# Icon files
TARGET_PNG = icons/breaker.png
ICO_FILE = icons/breaker.ico
ICON_RC = icons/breaker.rc
ICON_RES = $(BUILDDIR_WINDOWS)/breaker.res

# Build rules
all: linux windows

# Linux build
linux: $(BUILDDIR_LINUX) $(TARGET_LINUX)

$(BUILDDIR_LINUX):
	mkdir -p $(BUILDDIR_LINUX) $(OBJDIR_LINUX)

$(OBJDIR_LINUX)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_LINUX): $(OBJ_LINUX)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS_LINUX)

# Windows build
windows: $(BUILDDIR_WINDOWS) $(TARGET_WINDOWS)

$(BUILDDIR_WINDOWS):
	mkdir -p $(BUILDDIR_WINDOWS) $(OBJDIR_WINDOWS)

$(OBJDIR_WINDOWS)/%.o: $(SRCDIR)/%.c
	x86_64-w64-mingw32-gcc $(CFLAGS) -c $< -o $@

$(TARGET_WINDOWS): $(OBJ_WINDOWS) $(ICON_RES)
	x86_64-w64-mingw32-gcc $(CFLAGS) $^ -o $@ $(LDFLAGS_WINDOWS) -mwindows

# Icon generation
create-ico:
	@convert $(TARGET_PNG) -resize 16x16 icons/tmp_breaker_16.png
	@convert $(TARGET_PNG) -resize 32x32 icons/tmp_breaker_32.png
	@convert $(TARGET_PNG) -resize 48x48 icons/tmp_breaker_48.png
	@icotool -c -o $(ICO_FILE) icons/tmp_breaker_16.png icons/tmp_breaker_32.png icons/tmp_breaker_48.png $(TARGET_PNG)
	@rm -f icons/tmp_breaker_*.png

$(ICON_RES): $(ICON_RC)
	$(WINDRES) $(ICON_RC) -O coff -o $@

# Run rules
run_linux:
	./$(TARGET_LINUX)

run_linux_debug:
	gdb ./$(TARGET_LINUX)

run_windows:
	wine ./$(TARGET_WINDOWS)

# Clean rules
clean:
	rm -rf $(BUILDDIR_LINUX) $(BUILDDIR_WINDOWS)

rebuild: clean all

.PHONY: all clean rebuild linux windows run_linux run_windows run_linux_debug
