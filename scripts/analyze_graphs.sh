#!/bin/bash

# Change directory to the script's location
cd ..

# Get the optional subfolder parameter
subfolder="${1:-}"

# Prepare the find command based on the presence of a subfolder argument
if [ -z "$subfolder" ]; then
    find_command="find tests -type f -name \"*.gr\""
else
    find_command="find tests/$subfolder -type f -name \"*.gr\""
fi

# Print the table header
printf "%-30s %-20s %-10s %-10s\n" "Graph" "Density" "Planar" "Bipartite"

# Iterate through all .gr files in the specified subfolder or all subfolders, sorted by their names
eval "$find_command" | sort | while read -r test_file; do
    # Get the subfolder and test name
    test_name="${test_file#tests/}"

    # Run the graph_info.py script and extract the density and planarity values
    graph_info_output=$(python3 scripts/analyze_graph.py "$test_file")
    density=$(echo "$graph_info_output" | awk '/Graph Density:/ {print $3}')
    planar=$(echo "$graph_info_output" | awk '/Graph Planarity:/ {print $3}')
    bipartite=$(echo "$graph_info_output" | awk '/Graph Bipartite:/ {print $3}')

    # Print the graph name, density, and planarity in a table format
    printf "%-30s %-20s %-10s %-10s\n" "$test_name" "$density" "$planar" "$bipartite"
done
