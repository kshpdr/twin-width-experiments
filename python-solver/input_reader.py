from graph import Graph

def read_input_graph_from_lines(input_lines):
    graph = Graph()
    first_line = input_lines[0].strip()
    num_vertices, num_edges = map(int, first_line.split()[2:])

    graph.add_vertices(num_vertices)
    density = 2.0 * num_edges / (num_vertices * (num_vertices - 1))

    if density > 0.5:
        for i in range(num_vertices):
            for j in range(i + 1, num_vertices):
                graph.add_edge(i, j)

    for line in input_lines[1:]:
        line = line.strip()
        if not line or line.startswith('c'):
            continue
        v_id, u_id = map(int, line.split())

        if density > 0.5:
            graph.remove_edge(v_id - 1, u_id - 1)
        else:
            graph.add_edge(v_id - 1, u_id - 1)

    return graph


def read_input_graph_from_file(filename):
    with open(filename, 'r') as f:
        input_lines = f.readlines()
    return read_input_graph_from_lines(input_lines)
