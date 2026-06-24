# Advent of Code 2025 — C arm build system
#
# Usage:
#   make day=01 run      Build (debug, -O0 -g) and run day 01
#   make day=01 test     Build and run day 01 tests
#   make day=01 release  Build (-O2) and run day 01  <-- use for measurements
#   make day=01 debug    Build with -fanalyzer and run day 01
#   make day=01 clean    Remove build artifacts for day 01
#   make all             Build all days in release mode
#   make help            Show this message
#
# IMPORTANT: All RQ1 measurements must use 'release' target (-O2).
#            Never mix -O0 and -O2 results across days.

CC      ?= gcc
CFLAGS   = -std=c17 -Wall -Wextra -Wpedantic
DFLAGS   = -O0 -g
RFLAGS   = -O2
UTILS    = utils/utils.c
DAYS     = day01 day02 day03 day04 day05 day06 day07 day08 day09 day10 day11 day12

.PHONY: run test release debug clean all help

# Guard macro — emit an error if day= is not set
define need_day
	@test -n "$(day)" || { echo "Error: set day=NN, e.g. make day=01 $@"; exit 1; }
endef

run: ; $(need_day)
	$(CC) $(CFLAGS) $(DFLAGS) -I. -o day$(day)/main day$(day)/main.c $(UTILS)
	./day$(day)/main

test: ; $(need_day)
	$(CC) $(CFLAGS) $(DFLAGS) -DUNIT_TEST -I. -o day$(day)/test_main day$(day)/test_main.c day$(day)/main.c $(UTILS)
	./day$(day)/test_main

# release: -O2, no debug info — use this target for all RQ1 measurements
release: ; $(need_day)
	$(CC) $(CFLAGS) $(RFLAGS) -I. -o day$(day)/main day$(day)/main.c $(UTILS)
	./day$(day)/main

debug: ; $(need_day)
	$(CC) $(CFLAGS) $(DFLAGS) -fanalyzer -I. -o day$(day)/main day$(day)/main.c $(UTILS)
	./day$(day)/main

clean: ; $(need_day)
	rm -f day$(day)/main day$(day)/test_main massif.out.*

all:
	@for d in $(DAYS); do \
		if [ -f $$d/main.c ]; then \
			echo "Building $$d..."; \
			$(CC) $(CFLAGS) $(RFLAGS) -I. -o $$d/main $$d/main.c $(UTILS) \
				&& echo "  OK:   $$d" || echo "  FAIL: $$d"; \
		else \
			echo "  SKIP: $$d (no main.c yet)"; \
		fi; \
	done

help:
	@echo "Usage:"
	@echo "  make day=NN run      Build (debug) and run day NN"
	@echo "  make day=NN test     Build and run day NN tests"
	@echo "  make day=NN release  Build (-O2) and run  <-- use for measurements"
	@echo "  make day=NN debug    Build (-fanalyzer) and run day NN"
	@echo "  make day=NN clean    Remove artifacts for day NN"
	@echo "  make all             Build all days in release mode"
