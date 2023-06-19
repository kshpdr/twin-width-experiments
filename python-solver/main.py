import os
import sys
from heuristics import *
from input_reader import read_input_graph_from_lines, read_input_graph_from_file


if __name__ == '__main__':
    input_lines = None
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
        if os.path.isfile(input_file):
            input_graph = read_input_graph_from_file(input_file)
        else:
            print("File not found. Please provide a valid file name or input the graph directly.")
            sys.exit(1)
    else:
        input_lines = sys.stdin.readlines()
        input_graph = read_input_graph_from_lines(input_lines)

    print(find_from_cliques_degree_contraction_sequence(input_graph))

    # best_sequence = simulate(input_graph)
