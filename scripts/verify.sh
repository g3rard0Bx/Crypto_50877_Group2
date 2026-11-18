#!/usr/bin/env bash
# Test script for basic checks of project
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
GCC_ROOT=${GCC_ROOT:-$HOME/opt/gcc-16.2.0}
CLANG_ROOT=${CLANG_ROOT:-$HOME/opt/llvm-23.1.0}
cpu=${CPU:-$(awk '/Cpus_allowed_list/ {split($2,a,/[-,]/); print a[1]}' /proc/self/status)}
failed=0

# Run a command and check its expected exit status, including positive controls.
expect() {
    local expected=$1 status=0
    shift
    "$@" || status=$?
    if [[ $status != "$expected" ]]; then
        echo "FAIL: expected exit $expected, got $status: $*" >&2
        failed=1
    fi
}

for family in gcc clang; do
    if [[ $family == gcc ]]; then
        cc=$GCC_ROOT/bin/gcc; cxx=$GCC_ROOT/bin/g++
    else
        cc=$CLANG_ROOT/bin/clang; cxx=$CLANG_ROOT/bin/clang++
    fi
    echo "=== $family: builds, correctness, Memcheck and timing ==="
    "$cc" --version
    args=(make "CC=$cc" "CXX=$cxx" OPT=O2)
    "${args[@]}" -B -j2 all
    "${args[@]}" test memcheck controls-memcheck smoke
    bin=$("${args[@]}" -s paths)
    for name in memcheck-hydrogen memcheck-monocypher; do
        expect 99 valgrind --tool=memcheck --track-origins=yes \
            --error-exitcode=99 "$bin/$name"
    done
    for rep in {1..5}; do
        echo "=== $family timing repetition $rep/5 (2000 batches) ==="
        expect 3 timeout 180s taskset -c "$cpu" "$bin/controls-timing" leaky 2000
        expect 0 timeout 180s taskset -c "$cpu" "$bin/controls-timing" fixed-work 2000
        expect 0 timeout 180s taskset -c "$cpu" "$bin/timing-monocypher" 2000
    done
done

echo 'No timing finding is not proof of constant time. Annotated authentication reports are expected.'
if [[ $failed == 0 ]]; then echo 'Verification passed.'; else echo 'Verification failed.'; fi
exit "$failed"
