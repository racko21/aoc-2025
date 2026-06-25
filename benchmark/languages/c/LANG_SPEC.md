# C language rules

- Language: C17, compiled with `gcc` (or `cc`) using
  `-std=c17 -Wall -Wextra -Wpedantic`. Measurement builds add `-O2`.
- C standard library ONLY. No third-party libraries (no GLib, no external
  parsers). The math library (`-lm`) is available if you need it — if your
  solution uses math functions, note that it must link with `-lm` by putting
  any required functions to use; the build will link `-lm` automatically.
- Put your ENTIRE solution in a single file named `main.c` in your working
  directory. Do not split across multiple translation units. (You do not have
  a shared utils library here — write any helpers you need inside main.c.)
- `main.c` must read the puzzle input from the file `input.txt` in the same
  directory.
- Part selection via command-line argument:
      ./main 1     -> solve and print ONLY part 1
      ./main 2     -> solve and print ONLY part 2
  When run with no argument, solve and print both parts.
- Print results to stdout in exactly this format, one per line:
      Part 1: <answer>
      Part 2: <answer>
  Use `%lld` for 64-bit integer answers (AoC answers often exceed 32 bits).
  Print only `Part 1:` if the problem has no part 2.
- Do not print anything else to stdout (diagnostics go to stderr if needed).
- Manage memory correctly: every malloc/calloc should be freed. Avoid leaks
  and out-of-bounds access — the code is compiled with warnings as signal.
- Your solution must run to completion within the time limit on the REAL
  input, which is larger than the example. Choose an algorithm that scales.
- You may create scratch files (e.g. an `example.txt` you extract from the
  problem) to test your logic. Only `main.c` is measured for lines of code
  and correctness.
