#!/bin/bash

# The directory with the instances. Replace this with your actual directory.
DIRECTORY="/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/tests/exact-public/"

# Path to your runner.py
RUNNER_SCRIPT="/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/twin_width/runner.py"

# Output file where the results will be saved.
OUTPUT_FILE="results.csv"

# Header of the CSV file
echo "Instance, Twin-width" > $OUTPUT_FILE

# Iterate over all .gr files in the given directory.
for instance in $DIRECTORY/*.gr
do
  # Run the runner.py script on the instance and capture the output.
  output=$(python3 $RUNNER_SCRIPT $instance)

  # Extract the twin-width from the last line of the output.
  twin_width=$(echo "$output" | tail -1 | awk -F'[()]' '{print $2}')

  # Get the instance name without the directory part for cleaner output
  instance_name=$(basename $instance)

  # Write the instance name and twin-width to the results file.
  echo "$instance_name, $twin_width" >> $OUTPUT_FILE
done

echo "Benchmark completed. Results saved in $OUTPUT_FILE"
