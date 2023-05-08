import os
import graph_tool.all as gt
import random
import sys


class Graph:
    def __init__(self):

        self.graph = gt.Graph(directed=False)
        self.graph.properties[("v", "id")] = self.graph.new_vertex_property('int', val=False)
        self.graph.properties[("v", "red_edges")] = self.graph.new_vertex_property('int', val=False)
        self.graph.properties[("e", "red_edges")] = self.graph.new_edge_property('bool', val=False)
        self.twin_width = 0

    def get_vertex(self, i):
        return self.graph.vertex(i)

    def get_edge(self, v, u):
        return self.graph.edge(v, u)

    def get_vertices(self):
        return self.graph.vertices()

    def get_edges(self):
        return self.graph.edges()

    def get_red_edges(self):
        return self.graph.properties[("e", "red_edges")]

    def get_red_edges_pro_vertex(self):
        return self.graph.properties[("v", "red_edges")]

    def get_vertices_id(self):
        return self.graph.properties[("v", "id")]

    def add_vertices(self, n):
        self.graph.add_vertex(n)
        for i in range(n):
            self.graph.properties[("v", "id")][i] = i + 1

    def add_edge(self, v, u):
        self.graph.add_edge(v, u)

    def mark_edge_red(self, v, u):
        if self.get_red_edges()[self.get_edge(v, u)] == False:
            self.get_red_edges()[self.get_edge(v, u)] = True
            self.get_red_edges_pro_vertex()[v] += 1
            self.get_red_edges_pro_vertex()[u] += 1

    def mark_vertex_neighbors_red(self, source, neighbors):
        for neighbor in neighbors:
            self.mark_edge_red(source, neighbor)

    # Rewrite so that red_edges for each v updated
    def remove_vertex(self, v):
        self.graph.remove_vertex(v)

    def remove_edge(self, v, u):
        edge = self.graph.edge(v, u)
        if edge:
            self.graph.remove_edge(edge)
            if self.get_red_edges()[edge] == 1:
                self.get_red_edges_pro_vertex()[v] -= 1
                self.get_red_edges_pro_vertex()[u] -= 1

    def get_vertex_id(self, v):
        return self.graph.properties[("v", "id")][v]

    def get_twin_width(self):
        return self.twin_width

    def get_neighbors(self, v):
        return self.graph.get_out_neighbors(v)

    def get_score(self, v, u):
        common_neighbors = set(self.get_neighbors(v)).intersection(self.get_neighbors(u))
        all_neighbors = set(self.get_neighbors(v)).union(self.get_neighbors(u)).difference((v, u))
        return len(all_neighbors) - len(common_neighbors)

    # def get_red_edges_amount(self):
    #     red_edge_count = {}
    #     for v in self.get_vertices():
    #         red_edge_count[v] = 0
    #         for e in v.out_edges():
    #             if self.get_red_edges()[e]:
    #                 red_edge_count[v] += 1
    #
    #     return max(red_edge_count.values())

    def get_red_edges_amount(self):
        return self.get_red_edges_pro_vertex().a.max()

    def transfer_edges(self, src, dest, common_neighbors):
        for v in common_neighbors:
            e = input_graph.get_edge(src, v)
            if self.get_red_edges()[e] == 1 and self.get_red_edges()[input_graph.get_edge(dest, v)] == 0:
                self.mark_edge_red(dest, v)

    def merge_vertices(self, source, twin):
        print([f"{input_graph.get_vertex_id(e.source())} {input_graph.get_vertex_id(e.target())}: {input_graph.get_red_edges()[e]}" for e in input_graph.get_edges()])
        print([f"{input_graph.get_vertex_id(v)}: {input_graph.get_red_edges_pro_vertex()[v]}" for v in self.get_vertices()])
        source_neighbors = self.get_neighbors(source)
        twin_neighbors = self.get_neighbors(twin)

        # any edge (black or red) between x and y gets deleted
        self.remove_edge(source, twin)

        common_neighbors = set(source_neighbors).intersection(set(twin_neighbors))
        self.transfer_edges(twin, source, common_neighbors)

        #  x retains all black edges to its neighbors that are adjacent to y
        #  all edges from x to vertices that are not adjacent to y become red
        source_unique_neighbors = set(source_neighbors) - set(twin_neighbors).union({twin})
        self.mark_vertex_neighbors_red(source, source_unique_neighbors)

        # x is connected with a red edge to all vertices that are connected to y but not to x
        twin_unique_neighbors = set(twin_neighbors) - set(source_neighbors).union({source})
        new_edges = [(source, self.get_vertex(twin_neighbor)) for twin_neighbor in twin_unique_neighbors]
        self.graph.add_edge_list(new_edges)
        self.mark_vertex_neighbors_red(source, twin_unique_neighbors)

        # remove merged vertex
        self.remove_vertex(twin)
        print([f"{input_graph.get_vertex_id(e.source())} {input_graph.get_vertex_id(e.target())}: {input_graph.get_red_edges()[e]}" for e in input_graph.get_edges()])
        print([f"{input_graph.get_vertex_id(v)}: {input_graph.get_red_edges_pro_vertex()[v]}" for v in self.get_vertices()])
        self.twin_width = max(self.twin_width, self.get_red_edges_amount())


def find_random_contraction_sequence(input_graph):
    current_sequence = ""

    while input_graph.graph.num_vertices() > 1:
        v_id = random.choice(list(input_graph.graph.get_vertices()))
        u_id = random.choice(list(input_graph.graph.get_vertices()))
        while u_id == v_id:
            u_id = random.choice(list(input_graph.graph.get_vertices()))

        v, u = input_graph.get_vertex(v_id), input_graph.get_vertex(u_id)
        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        print(f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n")
        input_graph.merge_vertices(v, u)

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def read_input_graph_from_lines(input_lines):
    graph = Graph()
    first_line = input_lines[0].strip()
    num_vertices, num_edges = map(int, first_line.split()[2:])

    graph.add_vertices(num_vertices)

    for line in input_lines[1:]:
        line = line.strip()
        if not line or line.startswith('c'):
            continue
        v_id, u_id = map(int, line.split())
        graph.add_edge(v_id - 1, u_id - 1)

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

    best_sequence = find_random_contraction_sequence(input_graph)
    print(best_sequence)
