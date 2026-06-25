# Go language rules

- Language: Go (use whatever `go` resolves to on this system; Go 1.26+).
- Standard library ONLY. No third-party modules, no `go get`. Do not create
  a go.mod that requires external dependencies. If you reach for a third-party
  package, implement the needed functionality yourself instead.
- Put your ENTIRE solution in a single file named `main.go` in your working
  directory, in `package main`.
- `main.go` must read the puzzle input from the file `input.txt` in the same
  directory.
- Part selection via command-line argument:
      ./main 1     -> solve and print ONLY part 1
      ./main 2     -> solve and print ONLY part 2
  When run with no argument, solve and print both parts. (Read os.Args[1].)
- Print results to stdout in exactly this format, one per line:
      Part 1: <answer>
      Part 2: <answer>
  Print only `Part 1:` if the problem has no part 2.
- Do not print anything else to stdout (diagnostics go to stderr if needed).
- Your solution is built with `go build` and run as `./main <part>`. It must
  run to completion within the time limit on the REAL input, which is larger
  than the example. Choose an algorithm that scales.
- Use int / int64 appropriately — AoC answers often exceed 32 bits; prefer
  `int` (64-bit on modern platforms) or explicit `int64` for large sums.
- You may create scratch files (e.g. an `example.txt` you extract from the
  problem) to test your logic. Only `main.go` is measured for lines of code
  and correctness.
