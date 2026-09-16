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

# Building for Wii U (RPX + WUHB)

With devkitPPC + wut + the wiiu SDL2 portlibs installed (devkitPro pacman,
`wiiu-dev` group), configure with the Wii U toolchain wrapper and build:

```bash
powerpc-eabi-cmake -S . -B build-wiiu
powerpc-eabi-cmake --build build-wiiu
```

Run these from an MSYS2 shell (not PowerShell — `powerpc-eabi-cmake` and
`make` must resolve to the MSYS2/devkitPro ones). The build-tree path must
not contain spaces or apostrophes: devkitPPC's Windows-native gcc chokes on
MSYS path conversion, so a checkout at e.g. `Five Nights at Edward's U`
cannot build Wii U in-tree — copy the tree to a clean path (CI uses `/x`)
and build there, then copy the `.rpx`/`.wuhb`/`FNaE/` outputs back.

This produces `build-wiiu/FNaE_Native.rpx` and the Aroma bundle
`build-wiiu/FNaE_Native.wuhb` (game + `assets/` in one file, installed to
`sd:/wiiu/apps/`). A homebrew-folder bundle also lands in `build-wiiu/FNaE/` with only
`code/` (`FNaE_Native.rpx`, `app.xml`, `cos.xml`), `content/` (`assets/`), and `meta/`
(`meta.xml`, `iconTex.tga`, `bootTvTex.tga`, `bootDrcTex.tga`) — nothing loose at the root. Point Cemu's
File > Load at `code/FNaE_Native.rpx` (or copy the folder to
`sd:/wiiu/apps/FNaE/` for the Homebrew Launcher); the game probes
`fs:/vol/content` for its assets, so it boots from any of these layouts. On hardware the office runs on
the TV while the camera feeds run on the GamePad screen — see `KEYS.txt`
for the GamePad controls.

### Wii U saves (NAND/USB)

Pack `build-wiiu/FNaE/` (`code/`, `content/`, `meta/`) with NUSPacker and
install it with WUP Installer GX2 to NAND or USB: progress is kept in the
title's own save dir (`fs:/vol/save/common/Edward`, 128 KiB common save
declared in `assets/wiiu/meta.xml`), shared across accounts, in the same
INI format as the desktop save — back it up or inject an old file with
SaveMii. HBL / `.wuhb` runs (no installed title save) fall back to the SD
copy `sd:/wiiu/apps/FNaE/Edward`, then the working directory.

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
`<frame> <key|keyup|click|mouse|shot|ai|pad> <args>`; `doors.txt` covers
the office door controls). Screenshots land in
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
