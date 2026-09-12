# Agents — Five Nights at Edward's (Native C/SDL2)

Canonical instructions for AI agents working in this repo.

## Stack

C11, CMake 3.16+, Ninja, SDL2 + SDL2_image. Canonical toolchain is
Windows 10/11 + MSYS2 UCRT64 (see README.md).

## Build (MSYS2 UCRT64)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/FNaE_Native.exe
```

Run from the repo root so `assets/` resolves. `cmake *` is pre-approved
in `opencode.json`; anything destructive still asks first.

## Rules

- Keep to C11 + SDL2 public API. Update `PORT_STATUS.md` / `docs/`
  when converting Fusion logic.
- Do not commit MSYS2 absolute paths (`C:/msys64...`); keep
  `CMakeLists.txt` portable.
- Host may be Win11 Home: no Sandbox/Hyper-V. Prefer CI for
  untrusted builds; do not run opencode with `--auto`.
