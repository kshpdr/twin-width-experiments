#!/bin/bash

elapsed_time_counter() {
    local start=$SECONDS
    local elapsed_seconds
    while true; do
        elapsed_seconds=$(( SECONDS - start ))
        printf "\r%-30s %2ds" "Elapsed time:" "$elapsed_seconds"
        sleep 1
    done
}

# Change directory to the script's location
cd ..

# Get the optional name parameter and current date/time
subfolder="${1:-}"
current_date=$(date "+%Y-%m-%d")
current_time=$(date "+%H-%M-%S")

# Extract the function name from the twinwidth.py file
function_name=$(grep -o 'find_[^()]*' python-solver/main.py | head -n 1 | sed 's/(.*//')

# Create the output directories if they don't exist
mkdir -p "scripts/out/$current_date/results"
mkdir -p "scripts/out/$current_date/logs"

# Create the results and logs files
results_file="${current_time}-results-python-${subfolder}-${function_name}.csv"
logs_file="${current_time}-logs-python-${subfolder}-${function_name}.csv"
echo "Test,Time,Vertices,Edges,Solution" > "scripts/out/$current_date/results/$results_file"
echo "Test,Output" > "scripts/out/$current_date/logs/$logs_file"

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

    elapsed_time_counter & elapsed_time_pid=$!
    (gtime --format="%e" --quiet -o temp_time.txt python3 python-solver/main.py < "$test_file" 2>/dev/null > temp_python_output.txt)
    kill $elapsed_time_pid 2>/dev/null
    disown $elapsed_time_pid
    printf "\r"

    python_output=$(cat temp_python_output.txt)
    elapsed_time=$(cat temp_time.txt)

    # Extract the solution from the Python output
    python_solution=$(echo "$python_output" | perl -nle 'print $1 if /c twin width: (\d+)/')

    verifier_output=$(python3 scripts/verifier.py "$test_file" <(printf "%s" "$python_output") 2>&1)
    verifier_solution=$(echo "$verifier_output" | perl -nle 'print $1 if /Width: (\d+)/')

    if [ "$python_solution" == "$verifier_solution" ]; then
        solution="$python_solution"
    else
        verifier_error=$(echo "$verifier_output" | grep -v 'Width:' | head -n 1)
        solution="FAILED: Verifier: $verifier_solution != Python: $python_solution, Error: $verifier_error"
    fi

    # Save the result and log
    echo "$test_name,$elapsed_time,$vertices,$edges,$solution" >> "scripts/out/$current_date/results/$results_file"
    echo "$test_name,\"$python_output\"" >> "scripts/out/$current_date/logs/$logs_file"

    # Print the elapsed time
    printf "%-30s %-10s %-10s %-10s %-10s\n" "$test_name" "${elapsed_time}s" "$vertices" "$edges" "$solution"

    # Clean up the temporary files
    rm temp_python_output.txt temp_time.txt
done
