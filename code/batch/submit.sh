#!/bin/bash
set -e

if [ "$1" = "" ]; then
  echo "usage: submit <problem id>"
  exit 1
fi

SOLVER="../build/solve"
PROBLEMS="../../spec/problems/rounded"
SOLUTIONS="results"

problemId="$1"
printf -v PROB "%06d" $problemId

mkdir -p "$SOLUTIONS"

TEMPFILE="$PROB-failed.txt"
OUTFILE="$SOLUTIONS/$PROB-solved.txt"

"$SOLVER" -f "$PROBLEMS/$PROB-rounded.txt" | tee "$TEMPFILE"

./api.py submit "$problemId" "$TEMPFILE" && mv "$TEMPFILE" "$OUTFILE"
