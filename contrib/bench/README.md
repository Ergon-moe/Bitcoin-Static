# Contents

This directory contains tools for working with benchmarks and processing benchmark results.

## `bench_compare.sh`

Compare results of two benchmark runs, and report the biggest changes in total time by percent, sorted by the change amount.

Options:

- `--threshold` - By default, reports only changes greater than 10%. This is configurable with the `--threshold x` option.
- `--colour` (or `--color` depending on your preferred dialect) - enable or disable colour output.  Enabled by default in interactive terminals, disabled otherwise.  Accepts "on", "off" (and common synonyms), and "auto" (default).

Dependencies:

- `bc` - GNU Basic Calculator (`apt install bc`) - needed for floating point arithmetic.
- `grep`

Example usage:

    contrib/bench/bench_compare.sh --threshold 15 --colour on temp1.bench temp2.bench

## `bench_compare_branch.sh`

Check out, build and run benchmarks on two branches, and compare their results.

- If one branch is specified, it compares the currently checked out branch against that.
- If no branches are specified, it compares the currently checked out branch against master.
- If a `--filter` argument is specified, it's passed to `bench_bitcoin`'s `-filter`.
- If `--threshold` or `--colour` arguments are specified, they are treated as above.
- If one of the branches is checked out already, it builds and benchmarks that one first, to make best use of any cached build artefacts.

Dependencies:
- `git` to check out the project.
- `ninja` for building the project.
- All dependencies of `bench_compare.sh`.

Example usage (basic / advanced):

    ../contrib/bench/bench_compare_branch.sh
    ../contrib/bench/bench_compare_branch.sh --filter 'Base58.*' --threshold 15 master my_branch

## `benchmark_diff.py`

Compares bench_bitcoin outputs and displays percentage changes.

Changes are computed between a 'before_file' and 'after_file' which are
the captured standard output of the `bench_bitcoin` program.

Benchmarks that do not appear in both files are listed in a notice preceding
the per-benchmark output unless `--quiet` option is used.

All other benchmarks are compared and output in a combined table row which
lists benchmark names, the before/after values and percentage change of
median.

Before/after averages are also always computed but output only if the
`--average` (`-a`) option is used.

Output is plain text (CSV) by default, but can be switched to Markdown
table format using `--markdown` (`-m`).

The `--color` (`-c`) option makes colored output using ANSI escape sequences.
Percentage change values are colored green for improvement, red for
degradations, and not colored unless their absolute value deviates more than
an configurable percentage between before and after medians OR averages.
Coloring is disabled for Markdown.

The "highlighting" percentage cutoff is configurable using
`--percentage` (`-p`).
