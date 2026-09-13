# Five Nights at Edward's — Native Port

This project is the native C/SDL2 port of **Five Nights at Edward's**.

The goal is to recreate the original Clickteam Fusion game in native code, with the eventual goal of supporting platforms including Windows and Wii U.

---

## Requirements

### Windows

The recommended development environment is:

- Windows 10/11
- MSYS2
- UCRT64 environment
- GCC
- CMake
- Ninja
- SDL2
- SDL2_image

---

# Setting Up MSYS2

Download and install MSYS2:

https://www.msys2.org/

After installing it, open:

**MSYS2 UCRT64**

Do not use the regular MSYS terminal for building this project.

---

## Install Dependencies

Inside the **MSYS2 UCRT64** terminal, run:

```bash
pacman -Syu
````

If MSYS2 asks you to close the terminal, close it, reopen **MSYS2 UCRT64**, and run:

```bash
pacman -Syu
```

again.

Then install the development tools and SDL libraries:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_image
```

And:

```bash
pacman -S mingw-w64-ucrt-x86_64-SDL2_mixer
```

This installs:

* GCC
* CMake
* Ninja
* SDL2
* SDL2_image
* SDL2_mixer

---

# Building the Project

Open **MSYS2 UCRT64** and navigate to the project folder.

For example, if the project is located at:

```text
C:\Five Nights at Edward's U\Five Nights at Edward's U
```

use:

```bash
cd "/c/Five Nights at Edward's U/Five Nights at Edward's U"
```

Check that you are in the correct folder:

```bash
ls
```

You should see files/folders such as:

```text
CMakeLists.txt
src
assets
docs
README.md
```

---

## Configure CMake

Run:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

Then build:

```bash
cmake --build build
```

If everything succeeds, the self-contained bundle will be located at:

```text
build/FNaE/
```

containing `FNaE_Native.exe` plus `assets/` and the required SDL DLLs.

---

# Running the Game

From the project root, run:

```bash
./build/FNaE/FNaE_Native.exe
```

Run it from the project directory so the game can find:

```text
assets/
```

Alternatively `cd` into `build/FNaE/` and run it there — the bundle
carries its own copy of `assets/` and the DLLs, so it works standalone.

---

# Rebuilding After Changes

After editing the C source files, simply run:

```bash
cmake --build build
```

You normally do **not** need to run CMake again.

If `CMakeLists.txt` was changed, run:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

For a completely clean build:

```bash
rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

---

# Headless Testing (screenshots without a display)

The game can run scripted sessions and save screenshots — useful for
verifying visuals without playing through:

```bash
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  ./build/FNaE/FNaE_Native.exe --headless --frames 60 \
  --script scripts/headless/title.txt
```

Scripts live in `scripts/headless/` (`example.txt` shows the format:
`<frame> <key|keyup|click|shot> <args>`). Screenshots land in
`screenshots/`; remove them with:

```bash
cmake --build build --target clean-screenshots
```

---

# Quick Build Commands

From **MSYS2 UCRT64**:

```bash
cd "/c/path/to/FNaE_Native"

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

cmake --build build

./build/FNaE/FNaE_Native.exe
```

That's all that is required to build the current Windows version.
