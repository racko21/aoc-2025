#!/usr/bin/env python3
"""
AoC 2025 multi-language benchmark harness.

Solves Advent of Code days autonomously via the Anthropic API, one isolated
session per (language, day), capturing exact token/cost data for RQ2 and
runtime/memory/quality data for RQ1, and honest attempt/failure data for RQ3.

Design principles
-----------------
1. ISOLATION BY CONSTRUCTION. Each (language, day) session is given a working
   directory containing ONLY that language's files plus a read-only copy of the
   shared problem assets (PROBLEM.md, input.txt). No other language's solutions
   exist on disk in that session, so cross-language porting is impossible — not
   blocked by a hook, simply absent. The model also gets a fresh message history
   per day (no cross-day context leakage).

2. THE MODEL NEVER SEES THE ANSWER KEY. real_answers.txt is read by the HARNESS
   only, never placed in the session sandbox, never included in any prompt or
   tool result. Failure feedback tells the model "wrong answer" but never the
   expected value, so it cannot hardcode its way to a pass.

3. HONEST LOGGING. Attempts = number of solve iterations actually taken. A day
   that hits the turn cap is logged failure_category="incomplete", not silently
   dropped. Token/cost come straight from the API usage field, not transcribed.

4. LANGUAGE-AGNOSTIC. All language specifics live in languages/<lang>/ as
   LANG_SPEC.md (rules the model sees), build.sh and test.sh (how the harness
   compiles/runs a solution). Adding a language = new folder, zero runner edits.

Usage
-----
    export ANTHROPIC_API_KEY=sk-ant-...
    python runner.py --config config.yaml                 # all configured langs/days
    python runner.py --config config.yaml --lang python   # one language
    python runner.py --config config.yaml --lang python --days 1,2,3
    python runner.py --config config.yaml --dry-run        # validate setup, no API calls
"""

from __future__ import annotations

import argparse
import datetime as dt
import json
import os
import re
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Any, Optional

try:
    import yaml
except ImportError:
    sys.exit("Missing dependency: pip install pyyaml")

try:
    from anthropic import Anthropic
except ImportError:
    sys.exit("Missing dependency: pip install anthropic")


# --------------------------------------------------------------------------- #
# Configuration & data model
# --------------------------------------------------------------------------- #

ROOT = Path(__file__).resolve().parent


@dataclass
class Config:
    model: str
    max_turns_per_day: int
    languages: list[str]
    days: list[int]
    pricing: dict[str, dict[str, float]]  # model -> {input, output, cache_read, cache_write} USD per Mtok
    timeout_build_sec: int
    timeout_run_sec: int
    max_tool_output_chars: int
    max_cost_per_day_usd: Optional[float]

    @staticmethod
    def load(path: Path) -> "Config":
        raw = yaml.safe_load(path.read_text())
        return Config(
            model=raw["model"],
            max_turns_per_day=raw.get("max_turns_per_day", 30),
            languages=raw["languages"],
            days=raw.get("days") or list(range(1, 13)),
            pricing=raw["pricing"],
            timeout_build_sec=raw.get("timeout_build_sec", 60),
            timeout_run_sec=raw.get("timeout_run_sec", 60),
            max_tool_output_chars=raw.get("max_tool_output_chars", 20000),
            max_cost_per_day_usd=raw.get("max_cost_per_day_usd"),
        )


@dataclass
class Usage:
    input_tokens: int = 0
    output_tokens: int = 0
    cache_read_input_tokens: int = 0
    cache_creation_input_tokens: int = 0

    def add(self, u: Any) -> None:
        self.input_tokens += getattr(u, "input_tokens", 0) or 0
        self.output_tokens += getattr(u, "output_tokens", 0) or 0
        self.cache_read_input_tokens += getattr(u, "cache_read_input_tokens", 0) or 0
        self.cache_creation_input_tokens += getattr(u, "cache_creation_input_tokens", 0) or 0

    def cost_usd(self, model: str, pricing: dict) -> Optional[float]:
        p = pricing.get(model)
        if not p:
            return None
        return round(
            self.input_tokens / 1e6 * p.get("input", 0)
            + self.output_tokens / 1e6 * p.get("output", 0)
            + self.cache_read_input_tokens / 1e6 * p.get("cache_read", 0)
            + self.cache_creation_input_tokens / 1e6 * p.get("cache_write", 0),
            6,
        )


@dataclass
class DayResult:
    day: int
    language: str
    part1_correct: Optional[bool]
    part2_correct: Optional[bool]
    attempts: int
    duration_minutes: float
    duration_estimated: bool
    failure_category: str
    notes: str
    model: str
    lines_of_code: Optional[int]
    timestamp: str
    # RQ2
    input_tokens: int
    output_tokens: int
    cache_read_input_tokens: int
    cache_creation_input_tokens: int
    cost_usd: Optional[float]
    cost_calculation_note: str
    # RQ1
    runtime_ms_part1: Optional[float]
    runtime_ms_part2: Optional[float]
    peak_memory_kb_part1: Optional[int]
    peak_memory_kb_part2: Optional[int]
    compiler_warnings: Optional[int]
    optimization_flag: Optional[str]
    compiler: Optional[str]
    # RQ3 — iteration metrics (logged live, no backfill needed)
    turns_used: int
    submit_count: int
    bash_runs: int
    write_calls: int
    solution_rewrites: int
    read_calls: int


# --------------------------------------------------------------------------- #
# Tool definitions (what the model can do inside its sandbox)
# --------------------------------------------------------------------------- #

TOOLS = [
    {
        "name": "write_file",
        "description": (
            "Create or overwrite a file in your working directory. "
            "Paths are relative to your working directory. You cannot write "
            "outside it."
        ),
        "input_schema": {
            "type": "object",
            "properties": {
                "path": {"type": "string", "description": "Relative path, e.g. 'solution.py'"},
                "content": {"type": "string"},
            },
            "required": ["path", "content"],
        },
    },
    {
        "name": "read_file",
        "description": "Read a file from your working directory (relative path).",
        "input_schema": {
            "type": "object",
            "properties": {"path": {"type": "string"}},
            "required": ["path"],
        },
    },
    {
        "name": "run_bash",
        "description": (
            "Run a bash command from your working directory. Use this to compile "
            "and test your solution against your own extracted example. Network "
            "access is disabled. The real puzzle answer is NOT available to you; "
            "verify your logic against the example from the problem statement."
        ),
        "input_schema": {
            "type": "object",
            "properties": {"command": {"type": "string"}},
            "required": ["command"],
        },
    },
    {
        "name": "submit",
        "description": (
            "Submit when your solution is complete and you believe it is correct. "
            "The harness will run it against the real puzzle input and verify the "
            "answer. State which parts you are submitting."
        ),
        "input_schema": {
            "type": "object",
            "properties": {
                "parts_solved": {
                    "type": "array",
                    "items": {"type": "integer"},
                    "description": "e.g. [1, 2] or [1] if only part 1 exists/solved",
                }
            },
            "required": ["parts_solved"],
        },
    },
]


# --------------------------------------------------------------------------- #
# Sandbox: build a per-day isolated working directory
# --------------------------------------------------------------------------- #

class Sandbox:
    """An isolated working dir for one (language, day). Only this language's
    assets plus read-only shared problem files exist inside it."""

    def __init__(self, lang: str, day: int, cfg: Config):
        self.lang = lang
        self.day = day
        self.cfg = cfg
        self.day_str = f"day{day:02d}"
        self.work = ROOT / "languages" / lang / "work" / self.day_str
        self.shared = ROOT / "shared" / self.day_str
        self.lang_dir = ROOT / "languages" / lang

    def setup(self) -> None:
        # Fresh working dir every run
        if self.work.exists():
            shutil.rmtree(self.work)
        self.work.mkdir(parents=True, exist_ok=True)
        # Copy ONLY problem statement + input into the sandbox. Note: NOT
        # real_answers.txt — the model must never see the answer key.
        shutil.copy2(self.shared / "PROBLEM.md", self.work / "PROBLEM.md")
        shutil.copy2(self.shared / "input.txt", self.work / "input.txt")
        # Go isolation: the sandbox lives inside the repo tree, and the repo
        # root has a go.mod. Go tooling walks UP to find the nearest go.mod, so
        # without a local one a sandboxed `go build` would treat the sandbox as
        # part of the repo module and could resolve against the existing
        # dayNN/main.go solutions. Writing a standalone go.mod here stops that
        # upward walk and isolates the sandbox as its own module. (build.sh also
        # creates one defensively; doing it here guarantees it before any model
        # action.)
        if self.lang == "go":
            (self.work / "go.mod").write_text("module aocsolution\n\ngo 1.21\n")

    def real_answers(self) -> list[str]:
        """Read the answer key — HARNESS ONLY, never exposed to the model."""
        txt = (self.shared / "real_answers.txt").read_text().strip()
        return [ln.strip() for ln in txt.splitlines() if ln.strip()]

    # --- safe path resolution: confine all model file ops to the sandbox ---
    def _resolve(self, rel: str) -> Path:
        p = (self.work / rel).resolve()
        if not str(p).startswith(str(self.work.resolve())):
            raise ValueError(f"path escapes sandbox: {rel}")
        return p


# --------------------------------------------------------------------------- #
# Tool execution
# --------------------------------------------------------------------------- #

def run_subprocess(cmd: list[str] | str, cwd: Path, timeout: int, shell: bool = False) -> tuple[int, str, str]:
    try:
        proc = subprocess.run(
            cmd,
            cwd=str(cwd),
            shell=shell,
            capture_output=True,
            text=True,
            timeout=timeout,
            env={**os.environ, "PATH": os.environ.get("PATH", "")},
        )
        return proc.returncode, proc.stdout, proc.stderr
    except subprocess.TimeoutExpired:
        return 124, "", f"TIMEOUT after {timeout}s"


def execute_tool(name: str, inp: dict, sb: Sandbox, audit) -> tuple[str, bool]:
    """Returns (result_text, is_submit)."""
    cap = sb.cfg.max_tool_output_chars

    if name == "write_file":
        try:
            path = inp.get("path")
            content = inp.get("content")
            if path is None or content is None:
                return ("Error: write_file requires both 'path' and 'content'. "
                        "Resend the tool call with both fields."), False
            p = sb._resolve(path)
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(content)
            audit(f"write_file {path} ({len(content)} bytes)")
            return f"Wrote {path} ({len(content)} bytes).", False
        except Exception as e:
            audit(f"write_file DENIED {inp.get('path')}: {e}")
            return f"Error: {e}", False

    if name == "read_file":
        try:
            path = inp.get("path")
            if path is None:
                return "Error: read_file requires a 'path'. Resend with it.", False
            p = sb._resolve(path)
            if not p.exists():
                return f"Error: {path} does not exist.", False
            txt = p.read_text()
            audit(f"read_file {path}")
            if len(txt) > cap:
                txt = txt[:cap] + f"\n...[truncated, {len(txt)} chars total]"
            return txt, False
        except Exception as e:
            audit(f"read_file DENIED {inp.get('path')}: {e}")
            return f"Error: {e}", False

    if name == "run_bash":
        try:
            cmd = inp.get("command")
            if cmd is None:
                return ("Error: run_bash requires a 'command' string. "
                        "Resend the tool call with a 'command' field."), False
            audit(f"run_bash: {cmd}")
            rc, out, err = run_subprocess(cmd, sb.work, sb.cfg.timeout_run_sec, shell=True)
            body = f"exit={rc}\n--- stdout ---\n{out}\n--- stderr ---\n{err}"
            if len(body) > cap:
                body = body[:cap] + f"\n...[truncated]"
            return body, False
        except Exception as e:
            audit(f"run_bash ERROR: {e}")
            return f"Error running command: {e}", False

    if name == "submit":
        return json.dumps(inp), True

    return f"Unknown tool: {name}", False


# --------------------------------------------------------------------------- #
# Verification & measurement (harness-side, post-submit)
# --------------------------------------------------------------------------- #

ANSWER_RE = re.compile(r"Part\s*([12])\s*:\s*(.+?)\s*$", re.MULTILINE)


def parse_answers(stdout: str) -> dict[int, str]:
    """Extract 'Part N: value' lines from solution stdout."""
    out: dict[int, str] = {}
    for m in ANSWER_RE.finditer(stdout):
        out[int(m.group(1))] = m.group(2).strip()
    return out


def verify(sb: Sandbox, lang_dir: Path, cfg: Config) -> dict:
    """Build, then run against the REAL input, compare to the answer key.
    Returns a dict with correctness + RQ1 measurements. The model is not
    involved here and never sees the key."""
    result: dict[str, Any] = {
        "build_ok": False, "build_warnings": None, "build_stderr": "",
        "part1_correct": None, "part2_correct": None,
        "runtime_ms_part1": None, "runtime_ms_part2": None,
        "peak_memory_kb_part1": None, "peak_memory_kb_part2": None,
        "compiler": None, "optimization_flag": None,
        "produced": {},
    }
    key = sb.real_answers()  # [part1, part2?]

    # 1. Build (compile). build.sh receives the work dir as $1.
    rc, out, err = run_subprocess(
        ["bash", str(lang_dir / "build.sh"), str(sb.work)],
        cwd=sb.work, timeout=cfg.timeout_build_sec,
    )
    result["build_stderr"] = err
    result["build_ok"] = (rc == 0)
    # warnings: build.sh prints "WARNINGS=<n>" and "COMPILER=<...>" / "OPTFLAG=<...>"
    for line in (out + err).splitlines():
        if line.startswith("WARNINGS="):
            try: result["build_warnings"] = int(line.split("=", 1)[1])
            except ValueError: pass
        elif line.startswith("COMPILER="):
            result["compiler"] = line.split("=", 1)[1].strip()
        elif line.startswith("OPTFLAG="):
            result["optimization_flag"] = line.split("=", 1)[1].strip()
    if not result["build_ok"]:
        return result

    # 2. Run each part against the real input, measuring time + peak RSS.
    #    test.sh <workdir> <part> prints the solution stdout, and on stderr:
    #      runtime_ms: <float>
    #      peak_kb: <int>
    for part in (1, 2):
        if part == 2 and len(key) < 2:
            continue  # no part 2 for this day
        rc, out, err = run_subprocess(
            ["bash", str(lang_dir / "test.sh"), str(sb.work), str(part)],
            cwd=sb.work, timeout=cfg.timeout_run_sec,
        )
        produced = parse_answers(out).get(part)
        result["produced"][part] = produced
        expected = key[part - 1] if part - 1 < len(key) else None
        if expected is not None and produced is not None:
            result[f"part{part}_correct"] = (str(produced).strip() == str(expected).strip())
        # measurements from stderr
        rt = re.search(r"runtime_ms:\s*([\d.]+)", err)
        mem = re.search(r"peak_kb:\s*(\d+)", err)
        if rt: result[f"runtime_ms_part{part}"] = float(rt.group(1))
        if mem: result[f"peak_memory_kb_part{part}"] = int(mem.group(1))

    return result


def count_loc(sb: Sandbox, lang_dir: Path) -> Optional[int]:
    """LOC of the primary solution file, via loc.sh if present (lang-specific)."""
    loc_script = lang_dir / "loc.sh"
    if loc_script.exists():
        rc, out, err = run_subprocess(["bash", str(loc_script), str(sb.work)],
                                      cwd=sb.work, timeout=15)
        m = re.search(r"LOC=(\d+)", out)
        if m: return int(m.group(1))
    return None


# --------------------------------------------------------------------------- #
# The per-day agent loop
# --------------------------------------------------------------------------- #

SYSTEM_TEMPLATE = """\
You are solving an Advent of Code 2025 puzzle in {language}. This is a research
benchmark measuring how you solve algorithmic problems from scratch.

You are working in an isolated directory. It contains:
  - PROBLEM.md : the full puzzle statement (both parts if part 2 is unlocked)
  - input.txt  : your real puzzle input

{lang_spec}

Rules:
- Solve the problem independently and from first principles. You may have seen
  AoC problems before; set aside any recalled solution and derive your own.
- Use only the {language} standard library (plus anything explicitly allowed in
  the language rules above). No third-party packages.
- Your solution must read from input.txt and print results in EXACTLY this
  format, one per line:
      Part 1: <answer>
      Part 2: <answer>
  (Print only Part 1 if the problem has no part 2.)
- Extract the example from PROBLEM.md yourself and use it to check your logic.
  The real answer is NOT available to you — verify against the example you
  extract from the problem text.
- When confident, call submit. The harness will run your solution against the
  real input and tell you only whether each part is correct — not the value.
- If told an answer is wrong, debug and resubmit. Be rigorous, not lucky.

Work iteratively: write code, run it against your example, fix issues, submit.
"""


def solve_day(client: Anthropic, cfg: Config, lang: str, day: int,
              log_dir: Path, audit_log) -> DayResult:
    sb = Sandbox(lang, day, cfg)
    sb.setup()
    lang_dir = ROOT / "languages" / lang
    lang_spec = (lang_dir / "LANG_SPEC.md").read_text()

    problem = (sb.work / "PROBLEM.md").read_text()
    system = SYSTEM_TEMPLATE.format(language=lang, lang_spec=lang_spec)

    run_tag = f"{lang}_{sb.day_str}_{dt.datetime.now():%Y%m%dT%H%M%S}"
    run_path = log_dir / run_tag
    run_path.mkdir(parents=True, exist_ok=True)

    def audit(msg: str):
        audit_log(f"{dt.datetime.now(dt.timezone.utc).isoformat()} {lang} {sb.day_str} {msg}")

    messages: list[dict] = [{
        "role": "user",
        "content": f"Here is the puzzle:\n\n{problem}\n\nSolve it. Begin by extracting the example and planning your approach.",
    }]

    usage = Usage()
    attempts = 0           # counts submit attempts
    start = time.time()
    transcript: list[dict] = []
    final_verify: dict = {}
    notes_bits: list[str] = []
    failure_category = "incomplete"

    # RQ3 iteration counters — tallied live so no post-hoc backfill is needed.
    turns_used = 0
    bash_runs = 0
    write_calls = 0
    read_calls = 0
    solution_rewrites = 0
    # The canonical solution filename per language (writes to it = a rewrite).
    SOLUTION_FILE = {"python": "solution.py", "c": "main.c", "go": "main.go"}
    sol_name = SOLUTION_FILE.get(lang)

    for turn in range(cfg.max_turns_per_day):
        turns_used += 1
        # Budget guard: abort the day if its accumulated cost crosses the
        # configured ceiling, so a single runaway day can't drain the budget.
        if cfg.max_cost_per_day_usd is not None:
            running_cost = usage.cost_usd(cfg.model, cfg.pricing) or 0.0
            if running_cost >= cfg.max_cost_per_day_usd:
                notes_bits.append(
                    f"Aborted: per-day cost ceiling ${cfg.max_cost_per_day_usd} "
                    f"reached at turn {turns_used} (spent ${running_cost:.2f}).")
                failure_category = "incomplete"
                break
        resp = client.messages.create(
            model=cfg.model,
            max_tokens=8000,
            system=system,
            tools=TOOLS,
            messages=messages,
        )
        usage.add(resp.usage)
        transcript.append({"role": "assistant", "content": [_block_to_dict(b) for b in resp.content]})

        # gather tool calls
        tool_uses = [b for b in resp.content if b.type == "tool_use"]
        if not tool_uses:
            # model ended its turn with text and no action; nudge once
            messages.append({"role": "assistant", "content": resp.content})
            messages.append({"role": "user", "content": "Continue. Use the tools to write, test, and submit your solution."})
            continue

        messages.append({"role": "assistant", "content": resp.content})
        tool_results = []
        did_submit = False
        for tu in tool_uses:
            # tally iteration metrics live
            if tu.name == "run_bash":
                bash_runs += 1
            elif tu.name == "read_file":
                read_calls += 1
            elif tu.name == "write_file":
                write_calls += 1
                if sol_name and (tu.input or {}).get("path") == sol_name:
                    solution_rewrites += 1
            res_text, is_submit = execute_tool(tu.name, tu.input, sb, audit)
            if is_submit:
                did_submit = True
                attempts += 1
                # run real verification
                final_verify = verify(sb, lang_dir, cfg)
                fb, done = _submission_feedback(final_verify, sb)
                tool_results.append({"type": "tool_result", "tool_use_id": tu.id, "content": fb})
                if done:
                    failure_category = "correct"
                    notes_bits.append(f"Solved on submit attempt {attempts}.")
                    messages.append({"role": "user", "content": tool_results})
                    transcript.append({"role": "user", "content": tool_results})
                    break
            else:
                tool_results.append({"type": "tool_result", "tool_use_id": tu.id, "content": res_text})

        transcript.append({"role": "user", "content": tool_results})
        if did_submit and failure_category == "correct":
            break
        messages.append({"role": "user", "content": tool_results})

    duration_min = round((time.time() - start) / 60, 2)

    # measurements
    loc = count_loc(sb, lang_dir) if final_verify.get("build_ok") else None
    p1 = final_verify.get("part1_correct")
    p2 = final_verify.get("part2_correct")
    if failure_category != "correct":
        if not final_verify:
            notes_bits.append("Model never submitted within the turn cap.")
        elif not final_verify.get("build_ok"):
            failure_category = "build_error"
            notes_bits.append(f"Final build failed: {final_verify.get('build_stderr','')[:300]}")
        elif p1 is False or p2 is False:
            failure_category = "wrong_answer"
            notes_bits.append(f"Submitted but incorrect (p1={p1}, p2={p2}).")

    cost = usage.cost_usd(cfg.model, cfg.pricing)

    result = DayResult(
        day=day, language=lang,
        part1_correct=p1, part2_correct=p2,
        attempts=attempts if attempts else 1,
        duration_minutes=duration_min, duration_estimated=False,
        failure_category=failure_category,
        notes=" ".join(notes_bits) or "See transcript.",
        model=cfg.model, lines_of_code=loc,
        timestamp=dt.datetime.now(dt.timezone.utc).isoformat(),
        input_tokens=usage.input_tokens, output_tokens=usage.output_tokens,
        cache_read_input_tokens=usage.cache_read_input_tokens,
        cache_creation_input_tokens=usage.cache_creation_input_tokens,
        cost_usd=cost,
        cost_calculation_note="computed from API usage field" if cost is not None else "no pricing for model",
        runtime_ms_part1=final_verify.get("runtime_ms_part1"),
        runtime_ms_part2=final_verify.get("runtime_ms_part2"),
        peak_memory_kb_part1=final_verify.get("peak_memory_kb_part1"),
        peak_memory_kb_part2=final_verify.get("peak_memory_kb_part2"),
        compiler_warnings=final_verify.get("build_warnings"),
        optimization_flag=final_verify.get("optimization_flag"),
        compiler=final_verify.get("compiler"),
        turns_used=turns_used,
        submit_count=attempts,
        bash_runs=bash_runs,
        write_calls=write_calls,
        solution_rewrites=solution_rewrites,
        read_calls=read_calls,
    )

    # persist transcript + usage for audit / RQ2 ground truth
    (run_path / "transcript.json").write_text(json.dumps(transcript, indent=2, default=str))
    (run_path / "usage.json").write_text(json.dumps({
        "usage": asdict(usage), "cost_usd": cost, "model": cfg.model,
        "attempts": attempts, "duration_minutes": duration_min,
    }, indent=2))
    return result


def _submission_feedback(v: dict, sb: Sandbox) -> tuple[str, bool]:
    """Build feedback for the model after a submit — never reveals the answer."""
    if not v.get("build_ok"):
        return (f"Your solution failed to build:\n{v.get('build_stderr','')[:1500]}\n"
                "Fix the build and resubmit.", False)
    key = sb.real_answers()
    parts = [1] + ([2] if len(key) >= 2 else [])
    lines = []
    all_ok = True
    for p in parts:
        c = v.get(f"part{p}_correct")
        if c is True:
            lines.append(f"Part {p}: CORRECT")
        elif c is False:
            lines.append(f"Part {p}: INCORRECT")
            all_ok = False
        else:
            lines.append(f"Part {p}: no answer detected in output (check your output format)")
            all_ok = False
    msg = "\n".join(lines)
    if all_ok:
        return (msg + "\n\nAll parts correct. You are done.", True)
    return (msg + "\n\nReview your logic and resubmit. (The correct values are not provided.)", False)


def _block_to_dict(b) -> dict:
    d = {"type": b.type}
    if b.type == "text":
        d["text"] = b.text
    elif b.type == "tool_use":
        d.update({"id": b.id, "name": b.name, "input": b.input})
    return d


# --------------------------------------------------------------------------- #
# Main
# --------------------------------------------------------------------------- #

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--config", default="config.yaml")
    ap.add_argument("--lang", help="single language (default: all in config)")
    ap.add_argument("--days", help="comma-separated day numbers (default: all in config)")
    ap.add_argument("--dry-run", action="store_true", help="validate setup, no API calls")
    args = ap.parse_args()

    cfg = Config.load(ROOT / args.config)
    languages = [args.lang] if args.lang else cfg.languages
    days = [int(x) for x in args.days.split(",")] if args.days else cfg.days

    # --- preflight checks ---
    problems = []
    for d in days:
        sd = ROOT / "shared" / f"day{d:02d}"
        for fn in ("PROBLEM.md", "input.txt", "real_answers.txt"):
            if not (sd / fn).exists():
                problems.append(f"missing shared/day{d:02d}/{fn}")
    for lang in languages:
        ld = ROOT / "languages" / lang
        for fn in ("LANG_SPEC.md", "build.sh", "test.sh"):
            if not (ld / fn).exists():
                problems.append(f"missing languages/{lang}/{fn}")
    if problems:
        print("PREFLIGHT FAILED:")
        for p in problems:
            print("  -", p)
        sys.exit(1)
    if not os.environ.get("ANTHROPIC_API_KEY") and not args.dry_run:
        sys.exit("ANTHROPIC_API_KEY not set.")
    print(f"Preflight OK. Languages={languages} Days={days} Model={cfg.model}")
    if args.dry_run:
        print("Dry run — exiting before any API calls.")
        return

    client = Anthropic()
    log_path = ROOT / "results" / "thesis-log.jsonl"
    audit_path = ROOT / "results" / "isolation-audit.log"
    runs_dir = ROOT / "runs"
    runs_dir.mkdir(exist_ok=True)

    def audit_log(line: str):
        with audit_path.open("a") as f:
            f.write(line + "\n")

    for lang in languages:
        for d in days:
            print(f"\n=== {lang} day {d:02d} ===")
            try:
                res = solve_day(client, cfg, lang, d, runs_dir, audit_log)
            except Exception as e:
                import traceback
                tb = traceback.format_exc()
                print(f"  ERROR: {e}")
                print(tb)
                audit_log(f"{dt.datetime.now(dt.timezone.utc).isoformat()} {lang} day{d:02d} FATAL {e}\n{tb}")
                continue
            with log_path.open("a") as f:
                f.write(json.dumps(asdict(res)) + "\n")
            status = res.failure_category
            print(f"  -> {status} | p1={res.part1_correct} p2={res.part2_correct} "
                  f"| attempts={res.attempts} | {res.duration_minutes}min "
                  f"| {res.input_tokens}+{res.output_tokens} tok "
                  f"| ${res.cost_usd}")

    print("\nDone. Results appended to results/thesis-log.jsonl")


if __name__ == "__main__":
    main()
