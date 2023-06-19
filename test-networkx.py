import sys
import networkx as nx
import os
import matplotlib.pyplot as plt



def read_input_graph_from_lines(input_lines):
    graph = nx.Graph()
    first_line = input_lines[0].strip()
    num_vertices, num_edges = map(int, first_line.split()[2:])

    for line in input_lines[1:]:
        line = line.strip()
        if not line or line.startswith('c'):
            continue
        v_id, u_id = map(int, line.split())
        graph.add_edge(v_id, u_id)

    return graph


def read_input_graph_from_file(filename):
    with open(filename, 'r') as f:
        input_lines = f.readlines()
    return read_input_graph_from_lines(input_lines)


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

    g = nx.Graph()
    is_planar, P = nx.check_planarity(input_graph)
    faces = [tuple(sorted(P.traverse_face(v, u))) for (v, u) in P.edges()]
    unique_faces = list(set(faces))

    plt.figure(figsize=(10, 10))
    nx.draw_planar(P, with_labels=True, node_color='lightblue')
    plt.savefig('graph.png')