import graph_tool.all as gt
import random
import graph_tool.topology
import numpy as np
from graph import Graph
import heapq
from collections import defaultdict
import time


def find_random_contraction_sequence(input_graph: Graph):
    current_sequence = ""

    while input_graph.graph.num_vertices() > 1:
        v_id, u_id = random.sample(range(input_graph.graph.num_vertices()), 2)
        v, u = input_graph.get_vertex(v_id), input_graph.get_vertex(u_id)
        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        print(f"с Left ({input_graph.graph.num_vertices()}) {input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)}")
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
        input_graph.delete_from_merge_costs(u)
        input_graph.merge_vertices(v, u)
        input_graph.update_merge_scores(to_update_neighbors)

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def find_most_optimal_updated_contraction_sequence(input_graph):
    current_sequence = ""

    for v in input_graph.get_vertices():
        for u in input_graph.get_vertices():
            # print(f"{int(v)}: {int(u)}")
            if v == u: continue

            temp_graph = input_graph.graph.copy()

            # Merging vertices in the temporary graph
            temp_input_graph = Graph()
            temp_input_graph.graph = temp_graph
            temp_input_graph.merge_vertices(v, u)

            input_graph.set_merge_cost(input_graph.get_vertex_id(v), input_graph.get_vertex_id(u), max(temp_graph.vp["red_edges"].a))
            input_graph.set_merge_cost(input_graph.get_vertex_id(u), input_graph.get_vertex_id(v), max(temp_graph.vp["red_edges"].a))

    # print("Scores computed")
    while input_graph.graph.num_vertices() > 1:
        (v, u) = min(input_graph.merge_costs, key=input_graph.merge_costs.get)
        (v, u) = input_graph.get_vertex_by_id(v), input_graph.get_vertex_by_id(u)

        top_20_pairs = heapq.nsmallest(20, input_graph.merge_costs, key=input_graph.merge_costs.get)
        to_update_neighbors = set(input_graph.get_vertex_by_id(vertex) for pair in top_20_pairs for vertex in pair)
        # to_update_neighbors = set(input_graph.get_neighbors(v)).union(set(input_graph.get_neighbors(u))).union({int(v)})
        if u in to_update_neighbors: to_update_neighbors.remove(u)

        to_update_neighbors = [input_graph.get_vertex_id(v) for v in to_update_neighbors]

        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        # print(f"Found ({input_graph.get_merge_cost(input_graph.get_vertex_id(v), input_graph.get_vertex_id(u))}): ", input_graph.get_vertex_id(v), input_graph.get_vertex_id(u))
        input_graph.delete_from_merge_costs(u)
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


def find_degree_contraction(input_graph: Graph):
    """
    Takes first 10 vertices with the lowest degree,
    computes new red edges attached to those vertices,
    finds the best pair between them and merges it,
    update vertices' degrees
    Complexity O(n^2)
    """
    current_sequence = ""

    current_sequence += input_graph.get_sequence_from_paths()

    degree_to_vertices = defaultdict(list)
    for v in input_graph.get_vertices():
        degree_to_vertices[v.out_degree()].append(input_graph.get_vertex_id(v))

    while input_graph.graph.num_vertices() > 1:
        start_time = time.time()

        vertices_by_degree = sorted(degree_to_vertices.keys())
        candidates = []
        for degree in vertices_by_degree:
            while degree_to_vertices[degree] and len(candidates) < 20:
                candidates.append(degree_to_vertices[degree].pop())

        merge_scores = []

        for v in candidates:
            for u in candidates:
                if v != u:
                    merge_score = input_graph.get_score(input_graph.get_vertex_by_id(v), input_graph.get_vertex_by_id(u))
                    merge_scores.append(((v, u), merge_score))

        best_pair = min(merge_scores, key=lambda x: x[1])[0] if merge_scores else None

        v, u = best_pair
        current_sequence += f"{v} {u} \n"
        print(f"c Found ({input_graph.get_twin_width()}): {v} {u}")

        v, u = input_graph.get_vertex_by_id(v), input_graph.get_vertex_by_id(u)

        input_graph.merge_vertices(v, u)

        degree_to_vertices = defaultdict(list)
        for v in input_graph.get_vertices():
            degree_to_vertices[v.out_degree()].append(input_graph.get_vertex_id(v))

        end_time = time.time()
        elapsed_time = end_time - start_time
        print(f"c Time for loop: {elapsed_time} seconds")

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def find_degree_optimized_contraction(input_graph: Graph):
    """
    Takes first 20 vertices with the lowest degree,
    computes new red edges attached to those vertices,
    finds the best pair between them and merges it,
    update vertices' degrees
    Complexity O(n^2)
    """
    current_sequence = ""

    current_sequence += input_graph.get_sequence_from_paths()

    while input_graph.graph.num_vertices() > 1:
        start_time = time.time()

        vertex_degrees = input_graph.graph.get_out_degrees(input_graph.graph.get_vertices())

        vertices = np.array(list(input_graph.graph.vertices()))
        sorted_vertices = vertices[np.argsort(vertex_degrees)]

        candidates = sorted_vertices[:30]
        # candidates = sorted_vertices[-5:]

        merge_scores = []

        for v in candidates:
            for u in candidates:
                if v != u:
                    merge_score = input_graph.get_score(v, u)
                    merge_scores.append(((v, u), merge_score))

        best_pair = min(merge_scores, key=lambda x: x[1])[0] if merge_scores else None

        v, u = best_pair
        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        print(f"c Found ({input_graph.get_twin_width()}): {input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)}")

        input_graph.merge_vertices(v, u)

        end_time = time.time()
        elapsed_time = end_time - start_time
        print(f"c Time for loop: {elapsed_time} seconds")

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def find_degree_merge_simulation_contraction(input_graph: Graph):
    """
    Takes first 10 vertices with the lowest degree,
    simulates the merge and computes twin-width,
    finds the best pair between them and merges it,
    update vertices' degrees
    Complexity O(n^4) (probably because of the copying the graph)
    """
    current_sequence = ""

    degree_to_vertices = defaultdict(list)
    for v in input_graph.get_vertices():
        degree_to_vertices[v.out_degree()].append(input_graph.get_vertex_id(v))

    while input_graph.graph.num_vertices() > 1:
        start = time.time()
        vertices_by_degree = sorted(degree_to_vertices.keys())
        candidates = []
        for degree in vertices_by_degree:
            while degree_to_vertices[degree] and len(candidates) < 20:
                candidates.append(degree_to_vertices[degree].pop())

        merge_scores = []

        for v in candidates:
            for u in candidates:
                if v != u:
                    temp_graph = input_graph.graph.copy()

                    temp_input_graph = Graph()
                    temp_input_graph.graph = temp_graph
                    temp_input_graph.merge_vertices(input_graph.get_vertex_by_id(v), input_graph.get_vertex_by_id(u))

                    merge_scores.append(((v, u), max(temp_graph.vp["red_edges"].a)))

        best_pair = min(merge_scores, key=lambda x: x[1])[0] if merge_scores else None

        v, u = best_pair
        current_sequence += f"{v} {u} \n"
        print(f"c Found ({input_graph.get_twin_width()}): {v} {u}")

        v, u = input_graph.get_vertex_by_id(v), input_graph.get_vertex_by_id(u)

        input_graph.merge_vertices(v, u)

        degree_to_vertices = defaultdict(list)
        for v in input_graph.get_vertices():
            degree_to_vertices[v.out_degree()].append(input_graph.get_vertex_id(v))

        print(f"c Cycle in {time.time() - start} sec")

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def find_degree_merge_simulation_optimized_contraction(input_graph: Graph):
    """
    Takes first 10 vertices with the lowest degree,
    simulates the merge and computes twin-width,
    finds the best pair between them and merges it,
    update vertices' degrees
    Complexity O(n^3)
    """
    current_sequence = ""

    while input_graph.graph.num_vertices() > 1:
        start = time.time()
        vertex_degrees = input_graph.graph.get_out_degrees(input_graph.graph.get_vertices())

        vertices = np.array(list(input_graph.graph.vertices()))
        sorted_vertices = vertices[np.argsort(vertex_degrees)]

        candidates = sorted_vertices[:20]

        merge_scores = []

        for v in candidates:
            for u in candidates:
                if v != u:
                    merge_scores.append(((input_graph.get_vertex_id(v), input_graph.get_vertex_id(u)), input_graph.simulate_merge_vertices(v, u)))

        best_pair = min(merge_scores, key=lambda x: x[1])[0] if merge_scores else None

        v, u = best_pair
        current_sequence += f"{v} {u} \n"
        print(f"c Found ({input_graph.get_twin_width()}): {v} {u}")

        v, u = input_graph.get_vertex_by_id(v), input_graph.get_vertex_by_id(u)

        input_graph.merge_vertices(v, u)
        print(f"c Cycle in {time.time() - start} sec")

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def find_table_set_merge_score_contraction_sequence(input_graph: Graph):
    """
    Computes for all vertices nScore (neighborhood score),
    takes the best pair of them,
    updates only its neighbors' scores
    Complexity worst case O(n^3), best case O(n^2)
    """
    current_sequence = ""

    for v in input_graph.get_vertices():
        for u in input_graph.get_vertices():
            if v == u: continue
            input_graph.set_merge_cost(input_graph.get_vertex_id(v), input_graph.get_vertex_id(u), input_graph.get_score(v, u))
            input_graph.set_merge_cost(input_graph.get_vertex_id(u), input_graph.get_vertex_id(v), input_graph.get_score(v, u))
            print(f"c {v}, {u}")

    while input_graph.graph.num_vertices() > 1:
        (v, u) = min(input_graph.merge_costs, key=input_graph.merge_costs.get)
        (v, u) = input_graph.get_vertex_by_id(v), input_graph.get_vertex_by_id(u)

        to_update_neighbors = set(input_graph.get_neighbors(v)).union(set(input_graph.get_neighbors(u))).union({int(v)})
        if u in to_update_neighbors: to_update_neighbors.remove(u)

        to_update_neighbors = [input_graph.get_vertex_id(v) for v in to_update_neighbors]

        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        print(f"с ({input_graph.graph.num_vertices()}) Found ({input_graph.get_merge_cost(input_graph.get_vertex_id(v), input_graph.get_vertex_id(u))}): ", input_graph.get_vertex_id(v), input_graph.get_vertex_id(u))
        input_graph.delete_from_merge_costs(u)
        input_graph.merge_vertices(v, u)
        input_graph.update_merge_only_neighbors_scores(to_update_neighbors)

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence


def find_table_set_merge_score_optimized_contraction_sequence(input_graph: Graph):
    """
    Computes for all vertices nScore (neighborhood score),
    takes the best pair of them,
    updates only its neighbors' scores
    Complexity worst case O(n^3), best case O(n^2)
    """
    current_sequence = ""

    for v in input_graph.get_vertices():
        for u in input_graph.get_vertices():
            if v == u: continue
            input_graph.set_merge_cost(input_graph.get_vertex_id(v), input_graph.get_vertex_id(u), input_graph.get_score(v, u))
            input_graph.set_merge_cost(input_graph.get_vertex_id(u), input_graph.get_vertex_id(v), input_graph.get_score(v, u))
            print(f"c {v}, {u}")

    while input_graph.graph.num_vertices() > 1:
        sorted_merge_costs = sorted(input_graph.merge_costs.items(), key=lambda item: item[1])
        top_20_pairs = sorted_merge_costs[:20]

        (v, u) = min(input_graph.merge_costs, key=input_graph.merge_costs.get)
        (v, u) = input_graph.get_vertex_by_id(v), input_graph.get_vertex_by_id(u)

        current_sequence += f"{input_graph.get_vertex_id(v)} {input_graph.get_vertex_id(u)} \n"
        print(f"с ({input_graph.graph.num_vertices()}) Found ({input_graph.get_merge_cost(input_graph.get_vertex_id(v), input_graph.get_vertex_id(u))}): ", input_graph.get_vertex_id(v), input_graph.get_vertex_id(u))
        input_graph.delete_from_merge_costs(u)
        input_graph.merge_vertices(v, u)

        for pair in top_20_pairs:
            (v1, v2) = pair[0]
            score = input_graph.get_score(v1, v2)
            input_graph.set_merge_cost(input_graph.get_vertex_id(v1), input_graph.get_vertex_id(v2), score)
            input_graph.set_merge_cost(input_graph.get_vertex_id(v2), input_graph.get_vertex_id(v1), score)

    current_sequence += f"c twin width: {input_graph.get_twin_width()}"

    return current_sequence