#!/bin/bash

# Change directory to the script's location
cd ..

# Get the optional name parameter and current date/time
test_run_name="${1:-}"
subfolder="${2:-}"
current_datetime=$(date "+%Y-%m-%d-%H-%M-%S")

# Create the results and logs files
results_file="results-${test_run_name}-${current_datetime}.csv"
logs_file="logs-${test_run_name}-${current_datetime}.csv"
echo "Test,Time,Vertices,Edges,Solution" > scripts/out/$results_file
echo "Test,Output" > scripts/out/$logs_file

# Prepare the find command based on the presence of a subfolder argument
if [ -z "$subfolder" ]; then
    find_command="find tests -type f -name \"*.gr\""
else
    find_command="find tests/$subfolder -type f -name \"*.gr\""
fi

printf "%-30s %-10s %-10s %-10s %-10s\n" "Test" "Time" "Vertices" "Edges" "Solution"

# Iterate through all .gr files in tests' subfolders
eval "$find_command" | sort | while read -r test_file; do
    # Get the subfolder and test name
    test_name="${test_file#tests/}"

    # Extract the vertices and edges from the .gr file
    vertices_edges=$(head -n 1 "$test_file" | awk '{print $3 " " $4}')
    vertices=$(echo "$vertices_edges" | cut -d ' ' -f 1)
    edges=$(echo "$vertices_edges" | cut -d ' ' -f 2)


    # Run the Java solver with the test input and measure the time using gtime
    gtime_output=$(gtime --format="%e" --quiet -o temp_time.txt java -cp out/production/twin-width-solver Solver < "$test_file" 2>&1 > temp_java_output.txt)
    java_output=$(cat temp_java_output.txt)
    elapsed_time=$(cat temp_time.txt)

    # Extract the solution from the Java output
    java_solution=$(echo "$java_output" | perl -nle 'print $1 if /c twin width: (\d+)/')

    verifier_output=$(python3 scripts/verifier.py "$test_file" <(printf "%s" "$java_output"))
    verifier_solution=$(echo "$verifier_output" | perl -nle 'print $1 if /Width: (\d+)/')

    if [ "$java_solution" == "$verifier_solution" ]; then
        solution="$java_solution"
    else
        solution="FAILED: Verifier: $verifier_solution != Java: $java_solution"
    fi

    # Save the result and log
    echo "$test_name,$elapsed_time,$vertices,$edges,$solution" >> scripts/out/$results_file
    echo "$test_name,\"$java_output\"" >> scripts/out/$logs_file

    # Print the elapsed time
    printf "%-30s %-10s %-10s %-10s %-10s\n" "$test_name" "${elapsed_time}s" "$vertices" "$edges" "$solution"

    # Clean up the temporary files
    rm temp_java_output.txt temp_time.txt
done
