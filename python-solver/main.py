import os
import sys
import time
import cProfile, pstats, io
from heuristics import *
from input_reader import read_input_graph_from_lines, read_input_graph_from_file

def generate_random_solution(vertices_list):
    vertices = list(vertices_list)

    while len(vertices) > 1:
        random.shuffle(vertices)

        i = 0
        new_vertices = []

        while i < len(vertices):
            v1 = vertices[i]
            if i + 1 < len(vertices):
                v2 = vertices[i + 1]
                print(v1, v2)
                new_vertices.append(v1)  # Since v1 is already adjusted, no need to add 1 again
                i += 2
            else:
                new_vertices.append(v1)
                i += 1

        vertices = new_vertices

def timing_decorator(func):
    def wrapper(*args, **kwargs):
        start_time = time.time()
        result = func(*args, **kwargs)
        end_time = time.time()
        print(f"c {func.__name__} took {end_time - start_time} seconds")
        return result
    return wrapper


if __name__ == '__main__':

    # # Assuming g is an instance of your Graph class:
    # g = Graph()
    #
    # # Adding a vertex to the graph
    # g.add_vertices(5)
    # v1 = g.get_vertex(0)
    # # Modifying the red_edges_neighbors property for that vertex
    # g.add_neighbor_to_red_edges_neighbors(v1, g.get_vertex(4))
    # print(g.graph.vp.red_edges_neighbors[v1])  # This should print [1, 2, 3]
    #
    # g.remove_neighbor_from_red_edges_neighbors(v1, g.get_vertex(4))
    # print(g.graph.vp.red_edges_neighbors[v1])  # This should print [1, 2, 3]

    profile = False
    profiler = None

    if '--profile' in sys.argv:
        profile = True
        sys.argv.remove('--profile')

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

    if profile:
        profiler = cProfile.Profile()
        profiler.enable()

    contraction_sequence = find_red_edges_contraction(input_graph)
    print(contraction_sequence, end='')

    remaining_vertices = set(input_graph.get_vertices_id())
    generate_random_solution(remaining_vertices)

    if profile:
        profiler.disable()
        s = io.StringIO()
        sortby = 'cumulative'
        ps = pstats.Stats(profiler, stream=s).sort_stats(sortby)
        ps.print_stats()
        print(s.getvalue())

    # best_sequence = simulate(input_graph)
