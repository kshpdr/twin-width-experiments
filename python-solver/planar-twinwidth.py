import os
import graph_tool.all as gt
import networkx as nx
import sys


class PlanarGraph:
    def __init__(self):
        self.graph = gt.Graph(directed=False)
        self.graph.properties[("v", "id")] = self.graph.new_vertex_property('int', val=False)
        self.graph.properties[("v", "red_edges")] = self.graph.new_vertex_property('int', val=False)
        self.graph.properties[("e", "red_edges")] = self.graph.new_edge_property('bool', val=False)
        self.twin_width = 0

        self.graph.properties[("v", "skeleton_vertices")] = self.graph.new_vertex_property("bool", val=True)
        self.graph.properties[("e", "skeleton_edges")] = self.graph.new_edge_property("bool", val=True)
        self.graph.properties[("v", "layer")] = self.graph.new_vertex_property('int', val=0)

        self.networkx = nx.Graph()
        self.faces = []
        self.face_vertices = {}

    def print_edges(self):
        print([f"{e.source()} {e.target()}" for e in self.get_edges()])

    def print_vp(self, name):
        print(list(self.graph.vp[name]))

    def print_ep(self, name):
        print(list(self.graph.ep[name]))

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
        self.networkx.add_edge(v, u)

    def initialize_skeleton(self):
        self.graph.ep["skeleton_edges"].a = True
        self.graph.vp["skeleton_vertices"].a = True


    def add_edges_from_tuple(self, edges):
        for e_tuple in edges:
            self.add_edge(e_tuple[0], e_tuple[1])

    def compute_faces(self):
        is_planar, P = nx.check_planarity(self.networkx)
        faces = [tuple(sorted(P.traverse_face(v, u))) for (v, u) in P.edges()]
        self.faces = list(set(faces))
        for face in self.faces:
            if face not in self.face_vertices:
                self.face_vertices[face] = set()

    def mark_edge_red(self, v, u):
        if not self.get_red_edges()[self.get_edge(v, u)]:
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

    def construct_layering(self, root):
        layer_property = self.graph.new_vertex_property('int')
        layer_property[root] = 0

        bfs_generator = gt.bfs_iterator(self.graph, source=root)
        for src, tgt in bfs_generator:
            layer = layer_property[src] + 1
            if layer_property[tgt] == 0:
                layer_property[tgt] = layer

        self.graph.vertex_properties["layer"] = layer_property

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
        self.remove_vertex(twin)
        # print([f"{input_graph.get_vertex_id(e.source())} {input_graph.get_vertex_id(e.target())}: {input_graph.get_red_edges()[e]}" for e in input_graph.get_edges()])
        # print([f"{input_graph.get_vertex_id(v)}: {input_graph.get_red_edges_pro_vertex()[v]}" for v in self.get_vertices()])
        self.twin_width = max(self.twin_width, self.get_red_edges_amount())

    def is_all_faces_empty(self):
        for face in self.faces:
            if len(self.face_vertices[face]) != 0:
                return False
        return True

    def find_edge(self):
        layering = self.graph.vertex_properties["layer"]
        bfs_tree = gt.min_spanning_tree(self.graph, weights=layering)
        # Convert to set for efficient membership checking
        bfs_tree_edges = set(bfs_tree.get_array().tolist())

        for e in self.get_edges():
            if e not in bfs_tree_edges:
                new_graph = self.graph.copy()
                new_graph.add_edge(e.source(), e.target())
                # Now we need to find the unique cycle De in T + e
                cycle = gt.topology.extract_cycle(new_graph, e.source(), e.target())
                # Iterate over all the faces of T + e
                for face, vertices in self.face_vertices.items():
                    leaves_count = sum(1 for v in vertices if self.get_red_edges_pro_vertex()[v] == 1)
                    root_in_face = root in vertices
                    # Check if the face contains exactly one leaf of T and does not contain the root of T
                    if leaves_count == 1 and not root_in_face:
                        return e
        return None

    def compute_contraction(self):
        if self.is_all_faces_empty():  # Case 1
            print("First check")


def read_input_graph_from_lines(input_lines):
    graph = PlanarGraph()
    first_line = input_lines[0].strip()
    num_vertices, num_edges = map(int, first_line.split()[2:])

    graph.add_vertices(num_vertices)

    for line in input_lines[1:]:
        line = line.strip()
        if not line or line.startswith('c'):
            continue
        v_id, u_id = map(int, line.split())
        graph.add_edge(v_id - 1, u_id - 1)

    graph.compute_faces()

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

    gt.make_maximal_planar(input_graph.graph)
    input_graph.initialize_skeleton()
    input_graph.construct_layering(0)

    input_graph.compute_contraction()
