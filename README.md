# Crypto_50877_Group2
Run cpp script through WSL: Ubuntu

installing valgrind:
sudo apt update
sudo apt install -y valgrind

installing libhydrogen directly from github:
https://github.com/jedisct1/libhydrogen/wiki/Installation
-download hydrogen.c and hydrogen.h and /impl directory into the repository

If red squiggly still appears in hydrogen.h, hydrogen.c then ensure C/C++ Edit Configurations (JSON) includePath is set to the below:
```
"includePath": [
                "${workspaceFolder}/**"
            ],
```

Monocypher (git repo):
https://github.com/snej/monocypher-cpp.git
-monocypher.c
-monocypher.h

## (dev) Monocypher tests

Run from the repository root on Linux or WSL with GCC/G++ and Valgrind installed.
Use the intended compiler versions for research measurements, I built gcc 16.2 and Clang 23.1.0 from the release tags.
Set `GCC_ROOT` below to your GCC 16.2.0 installation directory if it differs.

```bash
GCC_ROOT="$HOME/opt/gcc-16.2.0"
CC="$GCC_ROOT/bin/gcc"
CXX="$GCC_ROOT/bin/g++"
"$CC" --version
"$CXX" --version
mkdir -p build/gcc-O2
"$CC" -O2 -g -Wall -Wextra -c monocypher.c -o build/gcc-O2/monocypher.o
"$CXX" -O2 -g -Wall -Wextra testMonocypher.cpp build/gcc-O2/monocypher.o -o build/gcc-O2/testMonocypher
"$CXX" -O2 -g -Wall -Wextra cryptoMonocypher.cpp build/gcc-O2/monocypher.o -o build/gcc-O2/cryptoMonocypher
"$CXX" -O2 -g -Wall -Wextra cryptoMonoDudect.cpp build/gcc-O2/monocypher.o -lm -o build/gcc-O2/cryptoMonoDudect

# Correctness checks: expect four PASS lines and zero Memcheck errors.
./build/gcc-O2/testMonocypher
valgrind --tool=memcheck --track-origins=yes --error-exitcode=99 ./build/gcc-O2/testMonocypher

# Secret-key annotations: authentication-result reports require interpretation.
valgrind --tool=memcheck --track-origins=yes --error-exitcode=99 ./build/gcc-O2/cryptoMonocypher

# Timing smoke check: run directly, not under Valgrind.
./build/gcc-O2/cryptoMonoDudect 1
```

For Clang, set `CC`/`CXX` to your `clang`/`clang++` executables and use a separate
`build/clang-O2` directory. Compile the library as C and the harnesses as C++.
