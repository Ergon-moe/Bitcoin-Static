#!/usr/bin/env bash
# Compare two sets of benchmark results, highlighting the most significant
# differences, and filtering out any differences between a minimal threshold.
# To generate a set of benchmark results, simply save the result of several benchmark runs:
#   src/bench/bench_bitcoin [-filter='benchmarkname'] | tee my_bench_result.txt
# and call this script with
#   contrib/bench/bench_compare.sh my_brench_result1.txt my_bench_result2.txt

export LC_ALL=C

# default threshold for differences to report, in %
threshold=10;

colour_enabled=auto

# consume any arguments
while [ "$#" -gt 2 ]; do
  if [ "$1" = "--threshold" ]; then
    shift
    threshold="$1"
    shift
  elif [ "$1" = "--colour" ] || [ "$1" = "--color" ]; then
    shift
    if [ "${1,,}" = "on" ] || [ "${1,,}" = "always" ] || [ "${1,,}" = "true" ] || [ "${1,,}" = "yes" ]; then
      colour_enabled=true
    elif [ "${1,,}" = "off" ] || [ "${1,,}" = "never" ] || [ "${1,,}" = "false" ] || [ "${1,,}" = "no" ]; then
      colour_enabled=false
    elif [ "${1,,}" = "auto" ]; then
      colour_enabled=auto
    else
      echo "Unrecognised --colour argument $1 - use \"on\", \"off\" or \"auto\"." >&2
      exit 1
    fi
    shift
  else
    echo "Unrecognised argument $1" >&2
    exit 1
  fi
done

if [ "$colour_enabled" == "auto" ]; then
  # enable colour if stdout is an interactive terminal only
  if [ -t 1 ]; then
    colour_enabled=true
  else
    colour_enabled=false
  fi
fi

if [ -z "$2" ]; then echo "Usage: $0 [--threshold <reporting_threshold_in_percent>] first_bench_result.txt second_bench_result.txt" >&2; exit 1; fi
if [ ! -f "$1" ]; then echo "Cannot find file $1" >&2; exit 1; fi
if [ ! -f "$2" ]; then echo "Cannot find file $2" >&2; exit 1; fi

reqs=("bc" "grep")
for req in "${reqs[@]}"; do
  if [ -z "$(which "$req")" ]; then
    echo "Missing dependency: \"$req\" - please install this and try again." >&2
    exit 1
  fi
done

echo "Reporting differences greater than $threshold% threshold." >&2

colour_red="\033[0;31m"
colour_green="\033[0;32m"
colour_reset="\033[0m"

# read lists of names of both sets of benchmark results
benchmarks_lhs=$(
  grep -v '^#' "$1" \
    | cut -d ',' -f 1 \
    | sort \
    | uniq
)
benchmarks_rhs=$(
  grep -v '^#' "$2" \
    | cut -d ',' -f 1 \
    | sort \
    | uniq
)
all_benchmarks=($(sort <<< "$benchmarks_lhs"$'\n'"$benchmarks_rhs" | uniq))

# find the common set, and any benchmarks unique to one or the other set
declare -a common_benchmarks
declare -a unique_benchmarks_lhs
declare -a unique_benchmarks_rhs
for bench in "${all_benchmarks[@]}"; do
  if grep -q "^$bench$" <<< "$benchmarks_lhs"; then
    if grep -q "^$bench$" <<< "$benchmarks_rhs"; then
      common_benchmarks+=("$bench")
    else
      unique_benchmarks_lhs+=("$bench")
    fi
  else
    unique_benchmarks_rhs+=("$bench")
  fi
done

echo "Total benchmarks: ${#all_benchmarks[@]}, common: ${#common_benchmarks[@]}, unique to $1: ${#unique_benchmarks_lhs[@]}, unique to $2: ${#unique_benchmarks_rhs[@]}"
echo

# read both sets of benchmark results
result_lhs_content=$(grep -v '^#' "$1")
result_rhs_content=$(grep -v '^#' "$2")

headers=$(head -1 "$1" | grep '^#' | cut -sd ' ' -f 2-)
if [ -z "$headers" ]; then
  # top line is not a valid header - provide our own default
  headers="Benchmark, evals, iterations, total, min, max, median"
fi

max_change="0.0"

# compare every result pair
for bench in "${common_benchmarks[@]}"; do
  result_lhs=$(
    grep "^$bench," <<< "$result_lhs_content" \
      | head -1 \
      | tr -d ' '
  )
  result_rhs=$(
    grep "^$bench," <<< "$result_rhs_content" \
      | head -1 \
      | tr -d ' '
  )
  evals_lhs=$(cut -d ',' -f 2 <<< "$result_lhs")
  evals_rhs=$(cut -d ',' -f 2 <<< "$result_rhs")
  if [ "$evals_lhs" != "$evals_rhs" ]; then
    echo "Cannot compare $bench - different number of evaluations ($evals_lhs in $1, $evals_rhs in $2)" >&2
    continue
  fi

  iters_lhs=$(cut -d ',' -f 3 <<< "$result_lhs")
  iters_rhs=$(cut -d ',' -f 3 <<< "$result_rhs")
  if [ "$iters_lhs" != "$iters_rhs" ]; then
    echo "Cannot compare $bench - different number of iterations ($iters_lhs in $1, $iters_rhs in $2)" >&2
    continue
  fi

  total_lhs=$(cut -d ',' -f 4 <<< "$result_lhs")
  total_rhs=$(cut -d ',' -f 4 <<< "$result_rhs")
  increase_speed=$(bc <<< "scale=8; -($total_rhs - $total_lhs) / $total_rhs * 100")
  increase_time=$(bc <<< "scale=8; ($total_rhs - $total_lhs) / $total_lhs * 100")

  # keep track of greatest increase
  if [ "$(bc <<< "define abs(x) {if (x<0) {return -x}; return x;}; abs($increase_speed) > abs($max_change)")" == 1 ]; then
    max_change="$increase_speed"
  fi

  increase_over_threshold=$(bc <<< "define abs(x) {if (x<0) {return -x}; return x;}; abs($increase_speed) >= $threshold || abs($increase_time) >= $threshold")
  if [ "$increase_over_threshold" = 0 ]; then
    # don't report this, as it's below the change threshold
    continue
  fi
  increase_speed_short=$(bc <<< "scale=2; $increase_speed / 1")
  increase_time_short=$(bc <<< "scale=2; $increase_time / 1")
  change_report="$change_report"$'\n'"$increase_speed_short,$increase_time_short,$bench"
done

change_report=$(sort -rn <<< "$change_report")
for change in $change_report; do
  increase_speed=$(cut -d ',' -f 1 <<< "$change")
  increase_time=$(cut -d ',' -f 2 <<< "$change")
  bench=$(cut -d ',' -f 3 <<< "$change")

  colour=""
  $colour_enabled && colour_end="$colour_reset" || colour_end=""
  if [ "${increase_speed:0:1}" = "-" ]; then
    verb_speed="decreased"
    verb_time="increased"
    $colour_enabled && colour="$colour_red"
    increase_speed=${increase_speed:1}
  else
    verb_speed="increased"
    verb_time="decreased"
    $colour_enabled && colour="$colour_green"
    increase_time=${increase_time:1}
  fi

  echo -e "$bench total time $verb_time $colour$increase_time%$colour_reset, speed $verb_speed $colour$increase_speed%$colour_end:"
  (
    echo "  File $headers"
    echo "  $1 $(grep "^$bench," "$1")"
    echo "  $2 $(grep "^$bench," "$2")"
  ) | tr -d ',' | column -t -s ' '
  echo
done

if [ -z "$change_report" ]; then
  echo "No changes to report within the $threshold% threshold.  Greatest change was $max_change%."
  echo
fi

# report benchmarks unique to one or the other set
if [ "${#unique_benchmarks_lhs[@]}" != 0 ]; then
  echo "The following ${#unique_benchmarks_lhs[@]} benchmarks appear only in $1: ${unique_benchmarks_lhs[*]}"
  echo
fi
if [ "${#unique_benchmarks_rhs[@]}" != 0 ]; then
  echo "The following ${#unique_benchmarks_rhs[@]} benchmarks appear only in $2: ${unique_benchmarks_rhs[*]}"
  echo
fi
