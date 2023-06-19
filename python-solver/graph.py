import graph_tool.all as gt
import time


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

                self.set_merge_cost(self.get_vertex_id(v), self.get_vertex_id(neighbor),
                                    max(temp_graph.vp["red_edges"].a))
                self.set_merge_cost(self.get_vertex_id(neighbor), self.get_vertex_id(v),
                                    max(temp_graph.vp["red_edges"].a))

    def update_merge_only_neighbors_scores(self, vertices):
        for v in vertices:
            v = self.get_vertex_by_id(v)
            for neighbor in self.get_vertices():
                if v == neighbor: continue
                self.set_merge_cost(self.get_vertex_id(v), self.get_vertex_id(neighbor),
                                    self.get_score(v, neighbor))
                self.set_merge_cost(self.get_vertex_id(neighbor), self.get_vertex_id(v),
                                    self.get_score(v, neighbor))


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

        self.graph.remove_vertex(v, fast = True)

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

    def get_new_degree_after_merge(self, v, u):
        all_neighbors = set(self.get_neighbors(v)).union(self.get_neighbors(u)).difference((v, u))
        return len(all_neighbors)

    def get_red_edges_amount(self):
        return self.get_red_edges_pro_vertex().a.max()

    def transfer_edges(self, src, dest, common_neighbors):
        for v in common_neighbors:
            e = self.get_edge(src, v)
            if self.get_red_edges()[e] == 1 and self.get_red_edges()[self.get_edge(dest, v)] == 0:
                self.mark_edge_red(dest, v)

    def merge_vertices(self, source, twin):
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
        # self.delete_from_merge_costs(twin)
        self.remove_vertex(twin)
        self.twin_width = max(self.twin_width, self.get_red_edges_amount())


    def optimized_merge_vertices(self, source, twin):
        source_neighbors = set(self.get_neighbors(source))
        twin_neighbors = set(self.get_neighbors(twin))

        # # any edge (black or red) between x and y gets deleted
        self.remove_edge(source, twin)

        common_neighbors = source_neighbors & twin_neighbors
        self.transfer_edges(twin, source, common_neighbors)

        # #  x retains all black edges to its neighbors that are adjacent to y
        # #  all edges from x to vertices that are not adjacent to y become red
        source_unique_neighbors = source_neighbors - twin_neighbors - {twin}
        self.mark_vertex_neighbors_red(source, source_unique_neighbors)

        # # x is connected with a red edge to all vertices that are connected to y but not to x
        twin_unique_neighbors = twin_neighbors - source_neighbors - {source}
        new_edges = [(source, self.get_vertex(twin_neighbor), 0) for twin_neighbor in twin_unique_neighbors]
        self.add_edges_from_tuple(new_edges)
        self.mark_vertex_neighbors_red(source, twin_unique_neighbors)

        # # remove merged vertex
        # # self.delete_from_merge_costs(twin)
        self.remove_vertex(twin)
        # self.twin_width = max(self.twin_width, self.get_red_edges_amount())

    def delete_from_merge_costs(self, to_delete):
        for v in self.get_vertices():
            if (self.get_vertex_id(v), self.get_vertex_id(to_delete)) in self.merge_costs:
                del self.merge_costs[(self.get_vertex_id(v), self.get_vertex_id(to_delete))]
            if (self.get_vertex_id(to_delete), self.get_vertex_id(v)) in self.merge_costs:
                del self.merge_costs[(self.get_vertex_id(to_delete), self.get_vertex_id(v))]

    def get_sequence_from_paths(self):
        sequence = ""
        for v in self.get_vertices():
            start = time.time()
            if not v.is_valid(): continue
            neighbors = self.get_neighbors(v)
            if v.out_degree() == 1 and self.get_vertex(neighbors[0]).out_degree() == 2:
                sequence += f"{self.get_vertex_id(self.get_vertex(neighbors[0]))} {self.get_vertex_id(v)} \n"
                self.merge_vertices(self.get_vertex(neighbors[0]), v)
                print("c Triggered")
                print(f"c Cycle in {time.time() - start}")
            elif v.out_degree() == 2:
                if self.get_vertex(neighbors[0]).out_degree() == 2:
                    sequence += f"{self.get_vertex_id(self.get_vertex(neighbors[0]))} {self.get_vertex_id(v)} \n"
                    self.merge_vertices(self.get_vertex(neighbors[0]), v)
                    print("c Triggered")
                    print(f"c Cycle in {time.time() - start}")
                elif self.get_vertex(neighbors[1]).out_degree() == 2:
                    sequence += f"{self.get_vertex_id(self.get_vertex(neighbors[1]))} {self.get_vertex_id(v)} \n"
                    self.merge_vertices(self.get_vertex(neighbors[1]), v)
                    print("c Triggered")
                    print(f"c Cycle in {time.time() - start}")
        return sequence