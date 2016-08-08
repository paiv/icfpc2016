#!/bin/bash


function genimg {

problemId=$1
printf -v PROB "%06d" $problemId

specFile="../../spec/problems/$PROB-spec.txt"

./api.py preproc "$specFile" > "temp-spec.txt" \
  && ./api.py draw "temp-spec.txt" > "images/$PROB.svg"
  # && convert -density 300 -transparent white -resize 600 "images/$PROB.svg" "images/$PROB.png"
}

for i in $(seq 1 101); do
  genimg $i
done
