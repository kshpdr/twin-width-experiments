import graph_tool.all as gt
import random
import graph_tool.topology
import numpy as np
from graph import Graph

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
