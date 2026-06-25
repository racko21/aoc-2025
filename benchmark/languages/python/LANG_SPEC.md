# Python language rules

- Language: Python 3 (use whatever `python3` resolves to on this system).
- Standard library ONLY. No pip installs, no third-party packages
  (no numpy, networkx, sympy, etc.). If you reach for a third-party library,
  implement the needed functionality yourself instead.
- Put your entire solution in a single file named `solution.py` in your
  working directory.
- `solution.py` must read the puzzle input from the file `input.txt` in the
  same directory (not from stdin, not a hardcoded path elsewhere).
- Print results to stdout in exactly this format, one per line:
      Part 1: <answer>
      Part 2: <answer>
  Print only the `Part 1:` line if the problem has no part 2.
- Do not print anything else to stdout (debug prints go to stderr if needed).
- Your code will be run as `python3 solution.py`. It must run to completion
  within the time limit on the real input, which is larger than the example.
- You may create scratch files (e.g. an `example.txt` you extract from the
  problem) to test your logic. Only `solution.py` is measured for lines of
  code and correctness.
