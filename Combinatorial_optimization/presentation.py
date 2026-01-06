from graph.graph_converter import read_from_file, matrix_from_edges
from algorithms.greedy_algorithm import greedy_coloring
from algorithms.tabu_search import run_tabu
import multiprocessing

if __name__ == "__main__":
    instances = [
        ("gc_1000", 5),
        ("gc500", 5),
        ("le450_5a", 4),
        ("miles250", 4),
        ("queen6", 4),
    ]
    matrixes = []
    for instance, max_lenght in instances:
        new_file_path = "./utils/presentation.txt"
        file_path = f"./utils//instance/{instance}.txt"
        num_nodes, edges = read_from_file(file_path)
        adjacency_matrix = matrix_from_edges(num_nodes, edges)
        colors, max_color = greedy_coloring(adjacency_matrix)
        matrixes.append((instance, adjacency_matrix, max_lenght))

    with multiprocessing.Pool() as pool:
        tasks = []
        for instance, adjacency_matrix, tabu_lenght in matrixes:
            tasks.append((instance, adjacency_matrix, 180, tabu_lenght))
        results = pool.map(run_tabu, tasks)
        for instance, tabu_lenght, tabu_search_max in results:
            with open(new_file_path, "a") as file:
                file.write("----------------------------\n")
                file.write(f"Instance: {instance}\n")
                file.write(f"Tabu: {tabu_lenght}, {tabu_search_max}\n")
