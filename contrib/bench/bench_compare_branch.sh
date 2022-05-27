#!/usr/bin/env bash
# Check out, build and run benchmarks on two branches, and compare their results.
# If one branch is specified, it compares the currently checked out branch against that.
# If no branches are specified, it compares the currently checked out branch against master.
# Example usage: contrib/bench/bench_compare_branch.sh [--filter 'my_benchmark.*'] [--threshold 15] master my_branch

export LC_ALL=C

# check if we're in a ninja build dir
if [ ! -f "build.ninja" ]; then
  echo "Run this from your build directory, after calling cmake -GNinja" >&2
  exit 1
fi

reqs=("git" "ninja" "bc" "grep")
for req in "${reqs[@]}"; do
  if [ -z "$(which "$req")" ]; then
    echo "Missing dependency: \"$req\" - please install this and try again." >&2
    exit 1
  fi
done

current_branch=$(git branch | grep -F '*' | cut -d ' ' -f 2)
compare_base="master"

filter_arg=""
declare -a threshold_args
# parse commandline arguments, if any
while [ "${1:0:2}" = "--" ]; do
  if [ "$1" = "--filter" ]; then
    shift
    filter_arg="-filter=$1"
    shift
    echo "Passing filter argument $filter_arg" >&2
  fi
  if [ "$1" = "--threshold" ]; then
    shift
    threshold_args=("--threshold" "$1")
    shift
  fi
  if [ "$1" = "--colour" ] || [ "$1" = "--color" ]; then
    shift
    colour_args=("--colour" "$1")
    shift
  fi
done

if [ -z "$1" ]; then
  if [ "$current_branch" = "$compare_base" ]; then
    echo "The current branch and the default comparison target ($compare_base) are the same.  Please specify two different branches to compare." >&2
    echo "Usage: $0 branch1 branch2" >&2
    exit 1
  fi
  compare_branch="$current_branch"
elif [ -z "$2" ]; then
  if [ "$1" = "$current_branch" ]; then
    echo "The current branch and the branch given on the commandline are the same.  Please specify two different branches to compare." >&2
    echo "Usage: $0 branch1 branch2" >&2
  fi
  compare_base="$1"
  compare_branch="$current_branch"
else
  compare_base="$1"
  compare_branch="$2"
fi

# check out branches and run the benchmarks
echo "Comparing benchmark results for $compare_base and $compare_branch..."

result1=$(mktemp "bchn_bench_compare_tmp.XXXXXXXXXX")
result2=$(mktemp "bchn_bench_compare_tmp.XXXXXXXXXX")
function cleanup_tmp {
  rm -f "$result1" "$result2"
}
trap cleanup_tmp EXIT

if [ "$current_branch" = "$compare_base" ]; then
  echo "Benchmarking $compare_base..." >&2
  ninja bench_bitcoin && \
    src/bench/bench_bitcoin "$filter_arg" > "$result1" || exit 1
  echo "Benchmarking $compare_branch..." >&2
  git checkout "$compare_branch" >&2 && \
    ninja bench_bitcoin >&2 && \
    src/bench/bench_bitcoin "$filter_arg" > "$result2" || exit 1
elif [ "$current_branch" = "$compare_branch" ]; then
  echo "Benchmarking $compare_branch..." >&2
  ninja bench_bitcoin && \
    src/bench/bench_bitcoin "$filter_arg" > "$result2" || exit 1
  echo "Benchmarking $compare_base..." >&2
  git checkout "$compare_base" >&2 && \
    ninja bench_bitcoin >&2 && \
    src/bench/bench_bitcoin "$filter_arg" > "$result1" || exit 1
else
  echo "Benchmarking $compare_base..." >&2
  git checkout "$compare_base" >&2 && \
    ninja bench_bitcoin >&2 && \
    src/bench/bench_bitcoin "$filter_arg" > "$result1" || exit 1
  echo "Benchmarking $compare_branch..." >&2
  git checkout "$compare_branch" >&2 && \
    ninja bench_bitcoin >&2 && \
    src/bench/bench_bitcoin "$filter_arg" > "$result2" || exit 1
fi
# return to the previously checked out branch
git checkout "$current_branch" >&2

script_dir=$(dirname "$0")
echo
"$script_dir/bench_compare.sh" "${threshold_args[@]}" "${colour_args[@]}" "$result1" "$result2"
