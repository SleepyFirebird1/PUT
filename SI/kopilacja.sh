#!/usr/bin/env bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

python3 "$DIR/fast-downward/fast-downward.py" "$DIR/hanoi/domain.pddl" "$DIR/hanoi/problem.pddl" --search "astar(blind())"