from graph_tool import GraphView

def simulate(input_graph):
    current_sequence = ""
    pairs = """2 3
    2 4
    10 11
    10 8
    10 9
    6 7
    2 1
    2 5
    2 6
    2 10"""

    for p in pairs.split("\n"):
        p = p.strip().split()
        v, u = input_graph.get_vertex_by_id(int(p[0])), input_graph.get_vertex_by_id(int(p[1]))
        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        # print(f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n")
        input_graph.merge_vertices(v, u)

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def get_graph_view(input_graph, v, u):
    def filter_vertex(vertex):
        return vertex != u

    simulated_graph = GraphView(input_graph.graph, vfilt=filter_vertex)    # Now you can perform operations on the simulated_graph as if 'u' was not present

    # Return the simulated graph
    return simulated_graph
