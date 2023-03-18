#!/bin/bash

if [ -z "$1" ]; then
  TEST_NAME="test" # default test name
else
  TEST_NAME=$1
fi

if [ -z "$2" ]; then
  TIMEOUT=600 # default timeout of 600 seconds
else
  TIMEOUT=$2
fi

if [ -z "$3" ]; then
  MAX_FAILED=3 # default max failed instances of 3
else
  MAX_FAILED=$3
fi

OUT_DIR=./out # default output directory of ./out

if [ ! -d "$OUT_DIR" ]; then
  mkdir -p $OUT_DIR
fi

today=$(date +%Y-%m-%d-%H-%M-%S)
LOG="log-$today.txt"                            # specify the name of the log file
maxSec=432000                                   # overall allowed time for the whole script


JAVA_OUT_FILE=out.txt     # replace this with your program's output file name
JAVA_RUN_CMD="java -cp ../out/production/twin-width-solver Solver" # replace this with your program's run command

# create CSV header
echo "graph,timeout,failed,result,other_params" > $OUT_DIR/"results-$TEST_NAME-$today.csv"

for dir in ../tests/*; do
  if [ -d "$dir" ]; then
    failed=0
    for file in $dir/*.gr; do
      graph=$(basename $file)
      graph=${graph%.*}

      echo "Running $graph ..."

      timeout $TIMEOUT $JAVA_RUN_CMD > /dev/null
      exit_code=$?

      if [ $exit_code -eq 124 ]; then
        echo "$graph timed out"
        echo "$graph,$TIMEOUT,," >> $OUT_DIR/results.csv
      elif [ $exit_code -eq 0 ]; then
        result=$(tail -n 1 $JAVA_OUT_FILE)

        # modify this to output your other params as necessary
        echo "$graph,$TIMEOUT,,\"$result\"," >> $OUT_DIR/results.csv
      else
        echo "$graph failed with exit code $exit_code"
        failed=$((failed+1))

        if [ $failed -ge $MAX_FAILED ]; then
          echo "Max failed instances exceeded, exiting ..."
          break
        fi
      fi
    done
  fi
done