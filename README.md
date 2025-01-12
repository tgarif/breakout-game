# Breakout Game

> [!WARNING]
> This is still under activate development. Anything can change at any moment without any notice!

Breakout game using C and OpenGL, a popular API for GPU rendering

## Overview

The game is a rewrite of Breakout game from https://learnopengl.com/In-Practice/2D-Game/Breakout.
In the series, the game is build using C++ but here, we use C. All the assets are taken from the series directly.
Only the app icon is created by me.

## Getting Started

#### Prerequisites

- [Make](https://www.gnu.org/software/make/)
- [C Compiler](https://gcc.gnu.org/) (e.g., GCC or Clang)
- [MinGW](https://www.mingw-w64.org/) for cross-compilation to Windows on Linux

#### Setup

1. Clone the repository:

```bash
git clone https://github.com/tgarif/breakout-game.git
cd breakout-game
```

2. Build the project for your platform:

- Linux:

  ```bash
  make linux
  ```

- Windows:

  ```bash
  make windows
  ```

Or build for both platforms:

```bash
make
```

3. Run the game:

- On Linux, you can simply run the game inside the `./build_linux` directory.

```bash
./build_linux/breaker # On Linux
```

- On Windows, you need to copy the dll files inside `./raylib/lib_windows/*.dll`, copy the executable inside `./build_windows`,
  and copy `./shaders`, `./textures`, `./audio`, `./levels`, `./icons`, `./fonts` into your game directory in windows before running it.

```bash
<windows-game-dir>/breaker.exe # On Windows
```

## License

This project is licensed under the MIT License. See [LICENSE](./LICENSE) for details.
