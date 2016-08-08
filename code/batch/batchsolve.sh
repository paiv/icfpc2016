#!/bin/bash

SOLVER="${1:-./solve}"
PROBLEMS="../../spec/problems"
SOLUTIONS="results"
FAILED="failed"
logFile="tempbatch.log"
errorFile="tempbatch-err.log"
tempSpec="tempbatch-spec.log"

if [ ! -x "$SOLVER" ]; then
  echo "usage: batchsolve [solver]"
  exit 1
fi

function trysolve {
  specFile="$1"

  x="${specFile##*/}"
  x="${x%-*}"
  problemId=$((10#$x))
  printf -v PROB "%06d" $problemId

  mkdir -p "$SOLUTIONS"

  tempFile="$FAILED/$PROB-failed.txt"
  OUTFILE="$SOLUTIONS/$PROB-solved.txt"

  if [ ! -s "$OUTFILE" ]; then
    if [ -s "$tempFile" ]; then
      # echo "skipping $tempFile"
      return
    fi

    echo "$specFile"

    [ -f "$logFile" ] && rm "$logFile"
    [ -f "$errorFile" ] && rm "$errorFile"
    [ -f "$tempSpec" ] && rm "$tempSpec"
    [ -f "$tempFile.solv" ] && rm "$tempFile.solv"

    ./api.py preproc "$specFile" > "$tempSpec"

    timeout 5 "$SOLVER" -f "$tempSpec" > "$tempFile.solv"

    if [ -s "$tempFile.solv" ]; then
      ./api.py postproc "$specFile" "$tempFile.solv" | tee "$tempFile" \
        && sleep 1 \
        && ./api.py submit "$problemId" "$tempFile" 1> "$logFile" 2> "$errorFile" \
        && mv "$tempFile" "$OUTFILE"

      if [ -s "$logFile" -a -s "$OUTFILE" ]; then
        cat "$logFile" >> "$OUTFILE"
        tail -n 1 "$logFile"
      fi
    else
      echo "failed" >> "$tempFile"
    fi

    if [ -s "$errorFile" ]; then
      cat "$errorFile" >> "$tempFile"
      tail -n 1 "$errorFile"
    fi

    [ -f "$logFile" ] && rm "$logFile"
    [ -f "$errorFile" ] && rm "$errorFile"
    [ -f "$tempSpec" ] && rm "$tempSpec"
    [ -f "$tempFile.solv" ] && rm "$tempFile.solv"

  fi
}

for f in "$PROBLEMS/"*-spec.txt ; do
  trysolve "$f"
done
