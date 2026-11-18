#!/usr/bin/env bash
# Validate the remaining x86-64 optimization levels; terminal output only.
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
GCC_ROOT=${GCC_ROOT:-$HOME/opt/gcc-16.2.0}
CLANG_ROOT=${CLANG_ROOT:-$HOME/opt/llvm-23.1.0}
cpu=${CPU:-$(awk '/Cpus_allowed_list/ {split($2,a,/[-,]/); print a[1]}' /proc/self/status)}
taskset -c "$cpu" true
failed=0
declare -A bins findings maxima

for family in gcc clang; do
    if [[ $family == gcc ]]; then
        cc=$GCC_ROOT/bin/gcc; cxx=$GCC_ROOT/bin/g++
    else
        cc=$CLANG_ROOT/bin/clang; cxx=$CLANG_ROOT/bin/clang++
    fi
    "$cc" --version
    for opt in O0 O1 O3; do
        args=(make "CC=$cc" "CXX=$cxx" "OPT=$opt")
        bin=$("${args[@]}" -s paths)
        bins[$family,$opt]=$bin
        "${args[@]}" -B -j2 "$bin/controls-test" "$bin/controls-timing"
        "$bin/controls-test"
        for mode in leaky fixed-work; do
            status=0
            valgrind --tool=memcheck --track-origins=yes --error-exitcode=99 \
                "$bin/controls-test" "$mode" || status=$?
            expected=0; [[ $mode != leaky ]] || expected=99
            [[ $status == "$expected" ]] || failed=1
            findings[$family,$opt,$mode]=0
            maxima[$family,$opt,$mode]=0
        done
    done
done

# Shuffle all 60 trials once; never replace failed trials.
while read -r family opt mode rep; do
    status=0
    output=$(timeout 180s taskset -c "$cpu" \
        "${bins[$family,$opt]}/controls-timing" "$mode" 2000 2>&1) || status=$?
    peak=$(awk '/max t:/ {for(i=1;i<NF;i++) if($i=="t:") {v=$(i+1)+0; if(v<0)v=-v; if(v>m)m=v}} END {print m+0}' <<< "$output")
    counts=$(awk '/Completed batches=/' <<< "$output")
    key=$family,$opt,$mode
    maxima[$key]=$(awk -v a="${maxima[$key]}" -v b="$peak" 'BEGIN {print (a>b?a:b)}')
    if [[ $status == 3 ]]; then findings[$key]=$(( ${findings[$key]} + 1 )); fi
    result=PASS
    if [[ $mode == leaky ]]; then
        [[ $status == 3 ]] || result=FAIL
    elif [[ $status != 0 ]] || ! awk '/Completed batches=/ {
        split($2,b,"="); split($4,g,"="); split(g[2],v,"/");
        split($7,c,"="); split(c[2],n,"/");
        if(b[2]+0==2000 && v[1]+v[2]==1000000 && n[1]+n[2]==977511) ok=1
    } END {exit !ok}' <<< "$output"; then result=FAIL; fi
    printf '%s %s %s repetition=%s exit=%s max|t|=%s %s\n%s\n' \
        "$family" "$opt" "$mode" "$rep" "$status" "$peak" "$result" "$counts"
    if [[ $result == FAIL ]]; then failed=1; printf '%s\n' "$output"; fi
done < <(
    for family in gcc clang; do
        for opt in O0 O1 O3; do
            for mode in leaky fixed-work; do
                for rep in {1..5}; do echo "$family $opt $mode $rep"; done
            done
        done
    done | shuf
)

echo 'Compiler optimization mode findings/5 largest printed |t|'
for family in gcc clang; do
    for opt in O0 O1 O3; do
        for mode in leaky fixed-work; do
            key=$family,$opt,$mode
            echo "$family $opt $mode ${findings[$key]}/5 ${maxima[$key]}"
        done
    done
done
echo 'Assembly inspection is a separate manual check. No finding is not proof of constant time.'
exit "$failed"
