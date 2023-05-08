import sys
import graph_tool as gt
import graph_tool.topology as topology


def read_graph(file_path):
    with open(file_path, 'r') as f:
        lines = f.readlines()

    vertices, edges = map(int, lines[0].split()[2:])
    adj_list = {i: [] for i in range(1, vertices + 1)}

    for line in lines[1:]:
        if line.strip() and line[0] != 'p':
            u, v = map(int, line.split())
            adj_list[u].append(v)
            adj_list[v].append(u)

    return vertices, edges, adj_list


def build_graph_tool_graph(vertices, edges, adj_list):
    g = gt.Graph(directed=False)
    g.add_vertex(vertices)
    edge_list = [(u - 1, v - 1) for u, neighbors in adj_list.items() for v in neighbors if u < v]
    g.add_edge_list(edge_list)

    return g


def graph_density(vertices, edges):
    return (2 * edges) / (vertices * (vertices - 1))


def degree_distribution(adj_list):
    degree_count = {}

    for vertex, neighbors in adj_list.items():
        degree = len(neighbors)
        degree_count[degree] = degree_count.get(degree, 0) + 1

    return degree_count


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python graph_info.py <graph_file>")
        sys.exit(1)

    graph_file = sys.argv[1]

    vertices, edges, adj_list = read_graph(graph_file)
    graph_tool_graph = build_graph_tool_graph(vertices, edges, adj_list)
    is_planar = topology.is_planar(graph_tool_graph)

    density = graph_density(vertices, edges)
    distribution = degree_distribution(adj_list)

    print(f"Graph Planarity: {is_planar}")
    print(f"Graph Density: {density}")
    print("Degree Distribution:")
    for degree, count in sorted(distribution.items()):
        print(f"Degree {degree}: {count} vertices")
