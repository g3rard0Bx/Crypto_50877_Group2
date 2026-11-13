# Crypto_50877_Group2

Research harnesses for compiler-induced timing leakage in Libhydrogen and Monocypher.
Run commands from the repository root on Linux or WSL.

## Layout

- `third_party/`: vendored Libhydrogen, Monocypher, and dudect sources.
- `harnesses/correctness/`: ordinary library correctness checks.
- `harnesses/memcheck/`: library harnesses with secret annotations.
- `harnesses/timing/`: Monocypher encryption timing harness.
- `controls/`: synthetic leaky/fixed-work targets and their verification drivers.
- `support/`: shared checked randomness helper.
- `legacy/`: incomplete original prototypes, excluded from builds.
- `build/`: ignored binaries/objects, isolated by compiler version, target, and optimization.

Library sources are already vendored. Libhydrogen's `impl/` tree must stay beside
its `hydrogen.c`. Original upstream references:
[Libhydrogen](https://github.com/jedisct1/libhydrogen) and
[Monocypher source reference](https://github.com/snej/monocypher-cpp.git).

## Build and test

Install Make, C/C++ development tools, and Valgrind (Ubuntu: `sudo apt install build-essential valgrind`).
Select the intended compiler versions explicitly; the system defaults may differ.
The following example assumes GCC 16.2.0 was installed under `$HOME/opt`:

```bash
GCC_ROOT="$HOME/opt/gcc-16.2.0"  # Change to your installation directory.
export CC="$GCC_ROOT/bin/gcc"
export CXX="$GCC_ROOT/bin/g++"
"$CC" --version
"$CXX" --version
make -j2 OPT=O2
make test OPT=O2
make memcheck OPT=O2
make controls-memcheck OPT=O2
make smoke OPT=O2
```

For Clang, set `CC` and `CXX` to matching `clang` and `clang++` executables.
The build compiles library/target C as C and harness C++ as C++; optimization
applies to both. No LTO is enabled by default. Use `OPT=O0`, `O1`, `O2`, or `O3`.
`make` builds only; timing runs and tests are explicit targets. `make clean`
removes outputs for the selected configuration; older configuration directories
and historical pilot binaries are retained.

`make test` checks library round trips, Monocypher ciphertext/tag/wrong-key
rejection, and control results. `make memcheck` expects zero errors for ordinary
correctness checks. `make controls-memcheck` expects the positive controls to
report taint (exit 99) and fixed-work to report no errors.

## Individual harnesses

With the compiler variables above set:

```bash
BIN_DIR=$(make -s paths OPT=O2)
"$BIN_DIR/correctness-hydrogen"
"$BIN_DIR/correctness-monocypher"
valgrind --tool=memcheck --track-origins=yes --error-exitcode=99 "$BIN_DIR/memcheck-hydrogen"
valgrind --tool=memcheck --track-origins=yes --error-exitcode=99 "$BIN_DIR/memcheck-monocypher"
"$BIN_DIR/controls-test"
"$BIN_DIR/timing-monocypher" 1
"$BIN_DIR/controls-timing" leaky 1
"$BIN_DIR/controls-timing" fixed-work 1
```

The annotated library harnesses can report authentication-result branches;
these require interpretation before claiming compiler-induced leakage. Exit 99
means Memcheck reported errors, even if the round trip succeeded.

Timing arguments specify maximum batches, 500 generated samples per batch.
The first batch is discarded as warm-up; one batch is only a smoke check.
Run timing directly, never under Valgrind. Predeclare larger budgets and repeated
trials, and validate controls for each measurement configuration. Timing exit 3
means dudect reported a finding; exit 0 means the budget ended without a finding,
not proof of constant time. The current dudect timer is x86-specific; ARM64
requires a timer implementation and validation.
