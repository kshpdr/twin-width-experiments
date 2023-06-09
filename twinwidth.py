import os
import graph_tool.all as gt
import random
import sys

import graph_tool.topology
import numpy as np


class Graph:
    def __init__(self):

        self.graph = gt.Graph(directed=False)
        self.graph.properties[("v", "id")] = self.graph.new_vertex_property('int', val=False)
        self.graph.properties[("v", "red_edges")] = self.graph.new_vertex_property('int', val=False)
        self.graph.properties[("e", "red_edges")] = self.graph.new_edge_property('bool', val=False)
        self.twin_width = 0
        self.merge_costs = {}

    def set_merge_cost(self, v, u, cost):
        self.merge_costs[(v, u)] = cost

    def get_merge_cost(self, v, u):
        return self.merge_costs.get((v, u), None)

    def update_merge_scores(self, vertices):
        for v in vertices:
            v = self.get_vertex_by_id(v)
            for neighbor in self.get_vertices():
                if v == neighbor: continue
                # neighbor = self.get_vertex(neighbor)
                temp_graph = self.graph.copy()
                temp_input_graph = Graph()
                temp_input_graph.graph = temp_graph
                temp_input_graph.merge_vertices(v, neighbor)

                input_graph.set_merge_cost(input_graph.get_vertex_id(v), input_graph.get_vertex_id(neighbor), max(temp_graph.vp["red_edges"].a))
                input_graph.set_merge_cost(input_graph.get_vertex_id(neighbor), input_graph.get_vertex_id(v), max(temp_graph.vp["red_edges"].a))

    def print_edges(self):
        print([f"{e.source()} {e.target()}" for e in self.get_edges()])

    def print_vp(self, name):
        print(list(self.graph.vp[name]))

    def get_vertex(self, i):
        return self.graph.vertex(i)

    def get_vertex_by_id(self, id):
        for v in self.get_vertices():
            if self.get_vertices_id()[v] == id:
                return v

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

    def add_edges_from_tuple(self, edges):
        for e_tuple in edges:
            self.add_edge(e_tuple[0], e_tuple[1])

    def mark_edge_red(self, v, u):
        if self.get_red_edges()[self.get_edge(v, u)] == False:
            self.get_red_edges()[self.get_edge(v, u)] = True
            self.get_red_edges_pro_vertex()[v] += 1
            self.get_red_edges_pro_vertex()[u] += 1

    def mark_vertex_neighbors_red(self, source, neighbors):
        for neighbor in neighbors:
            self.mark_edge_red(source, neighbor)

    def remove_vertex(self, v):
        for e in list(v.out_edges()):
            self.remove_edge(v, e.target())

        self.graph.remove_vertex(v)

    def remove_edge(self, v, u):
        edge = self.graph.edge(v, u)
        if edge:
            self.graph.remove_edge(edge)
            if self.get_red_edges()[edge] == 1:
                self.get_red_edges()[edge] = 0  # since when deleting edges its index becomes vacant
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

    def get_red_edges_amount(self):
        return self.get_red_edges_pro_vertex().a.max()

    def transfer_edges(self, src, dest, common_neighbors):
        for v in common_neighbors:
            e = input_graph.get_edge(src, v)
            if self.get_red_edges()[e] == 1 and self.get_red_edges()[input_graph.get_edge(dest, v)] == 0:
                self.mark_edge_red(dest, v)

    def merge_vertices(self, source, twin):
        # print([f"{input_graph.get_vertex_id(e.source())} {input_graph.get_vertex_id(e.target())}: {input_graph.get_red_edges()[e]}" for e in input_graph.get_edges()])
        # print([f"{input_graph.get_vertex_id(v)}: {input_graph.get_red_edges_pro_vertex()[v]}" for v in self.get_vertices()])
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
        new_edges = [(source, self.get_vertex(twin_neighbor), 0) for twin_neighbor in twin_unique_neighbors]
        self.add_edges_from_tuple(new_edges)
        self.mark_vertex_neighbors_red(source, twin_unique_neighbors)

        # remove merged vertex
        self.delete_from_merge_costs(twin)
        self.remove_vertex(twin)
        # print([f"{input_graph.get_vertex_id(e.source())} {input_graph.get_vertex_id(e.target())}: {input_graph.get_red_edges()[e]}" for e in input_graph.get_edges()])
        # print([f"{input_graph.get_vertex_id(v)}: {input_graph.get_red_edges_pro_vertex()[v]}" for v in self.get_vertices()])
        self.twin_width = max(self.twin_width, self.get_red_edges_amount())

    def delete_from_merge_costs(self, to_delete):
        for v in self.get_vertices():
            if (input_graph.get_vertex_id(v), input_graph.get_vertex_id(to_delete)) in self.merge_costs:
                del self.merge_costs[(input_graph.get_vertex_id(v), input_graph.get_vertex_id(to_delete))]
            if (input_graph.get_vertex_id(to_delete), input_graph.get_vertex_id(v)) in self.merge_costs:
                del self.merge_costs[(input_graph.get_vertex_id(to_delete), input_graph.get_vertex_id(v))]

def find_random_contraction_sequence(input_graph):
    current_sequence = ""

    while input_graph.graph.num_vertices() > 1:
        v_id, u_id = random.sample(range(input_graph.graph.num_vertices()), 2)
        v, u = input_graph.get_vertex(v_id), input_graph.get_vertex(u_id)
        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        # print(f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n")
        input_graph.merge_vertices(v, u)

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def find_table_contraction_sequence(input_graph):
    current_sequence = ""

    while input_graph.graph.num_vertices() > 1:
        best_pair = ()
        best_score = float("inf")
        for v in input_graph.get_vertices():
            for u in input_graph.get_vertices():
                if v == u: continue

                temp_graph = input_graph.graph.copy()

                # Merging vertices in the temporary graph
                temp_input_graph = Graph()
                temp_input_graph.graph = temp_graph
                temp_input_graph.merge_vertices(v, u)

                # Calculating the sum of red edges after contraction
                red_sum = max(temp_graph.vp["red_edges"].a)

                # Updating minimum score and best pair if needed
                if red_sum < best_score:
                    best_score = red_sum
                    best_pair = (v, u)

                # print(int(v) + 1, int(u) + 1)
                # temp_input_graph.print_vp("red_edges")

        v, u = best_pair[0], best_pair[1]
        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        # print(f"Found ({best_score}): ", input_graph.get_vertex_id(v), input_graph.get_vertex_id(u))
        input_graph.merge_vertices(v, u)
        # input_graph.print_vp("red_edges")

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def find_table_set_contraction_sequence(input_graph):
    current_sequence = ""

    for v in input_graph.get_vertices():
        for u in input_graph.get_vertices():
            if v == u: continue

            temp_graph = input_graph.graph.copy()

            # Merging vertices in the temporary graph
            temp_input_graph = Graph()
            temp_input_graph.graph = temp_graph
            temp_input_graph.merge_vertices(v, u)

            input_graph.set_merge_cost(input_graph.get_vertex_id(v), input_graph.get_vertex_id(u), max(temp_graph.vp["red_edges"].a))
            input_graph.set_merge_cost(input_graph.get_vertex_id(u), input_graph.get_vertex_id(v), max(temp_graph.vp["red_edges"].a))

    while input_graph.graph.num_vertices() > 1:
        (v, u) = min(input_graph.merge_costs, key=input_graph.merge_costs.get)
        (v, u) = input_graph.get_vertex_by_id(v), input_graph.get_vertex_by_id(u)

        to_update_neighbors = set(input_graph.get_neighbors(v)).union(set(input_graph.get_neighbors(u))).union({int(v)})
        if u in to_update_neighbors: to_update_neighbors.remove(u)

        to_update_neighbors = [input_graph.get_vertex_id(v) for v in to_update_neighbors]

        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        # print(f"Found ({input_graph.get_merge_cost(input_graph.get_vertex_id(v), input_graph.get_vertex_id(u))}): ", input_graph.get_vertex_id(v), input_graph.get_vertex_id(u))
        input_graph.merge_vertices(v, u)
        input_graph.update_merge_scores(to_update_neighbors)

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def find_vc_similarity_contraction_sequence(input_graph):
    current_sequence = ""

    while input_graph.graph.num_vertices() > 1:
        best_pair = ()
        best_score = float("-inf")
        for v in input_graph.get_vertices():
            for u in input_graph.get_vertices():
                if v == u: continue
                if input_graph.graph.num_vertices() == 2:
                    best_pair = (input_graph.graph.vertex(0), input_graph.graph.vertex(1))

                temp_graph = input_graph.graph.copy()

                # Merging vertices in the temporary graph
                temp_input_graph = Graph()
                temp_input_graph.graph = temp_graph
                temp_input_graph.merge_vertices(v, u)

                # Calculating the sum of red edges after contraction
                red_sum = graph_tool.topology.vertex_similarity(temp_graph, sim_type='leicht-holme-newman', vertex_pairs=[(v, u)])

                # Updating minimum score and best pair if needed
                if red_sum > best_score:
                    best_score = red_sum
                    best_pair = (v, u)

                # print(int(v) + 1, int(u) + 1)
                # temp_input_graph.print_vp("red_edges")

        v, u = best_pair[0], best_pair[1]
        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        print(f"Found ({best_score}): ", input_graph.get_vertex_id(v), input_graph.get_vertex_id(u))
        input_graph.merge_vertices(v, u)
        input_graph.print_vp("red_edges")

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def find_from_cliques_contraction_sequence(input_graph):
    current_sequence = ""

    while input_graph.graph.num_vertices() > 1:
        cliques = gt.max_cliques(input_graph.graph)
        clique = next(cliques)

        # print(len(list(input_graph.get_vertices())))
        while len(clique) > 1:
            v, u = input_graph.get_vertex(clique[0]), input_graph.get_vertex(clique[1])
            # print(f"Merged {input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)}")
            clique = [el - 1 if el > clique[1] else el for el in clique]
            input_graph.merge_vertices(v, u)
            current_sequence += f"{input_graph.get_vertex_id(clique[0])} {input_graph.get_vertex_id(clique[1])} \n"
            clique = np.delete(clique, 1)

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"
    return current_sequence


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

    # input_graph.print_edges()
    # [print(v) for v in gt.vertex_similarity(input_graph.graph)]

    print(find_table_set_contraction_sequence(input_graph))

    # best_sequence = simulate(input_graph)
    # print(best_sequence)
