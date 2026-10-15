from algorithms.printing_graph import coloring_graph
from graph.graph_converter import read_from_file, matrix_from_edges
from algorithms.greedy_algorithm import greedy_coloring
from algorithms.tabu_search import run_tabu
import multiprocessing

if __name__ == "__main__":
    instances = [
        ("instance_small", 3),
    ]
    matrixes = []
    for instance, max_lenght in instances:
        new_file_path = "./utils/report.txt"
        file_path = f"./utils//instance/{instance}.txt"
        num_nodes, edges = read_from_file(file_path)
        adjacency_matrix = matrix_from_edges(num_nodes, edges)
        colors, max_color = greedy_coloring(adjacency_matrix)
        matrixes.append((instance, adjacency_matrix, max_lenght))
        
    greedy_coloring_result = {node: color for node, color in enumerate(colors)}
    coloring_graph(edges, greedy_coloring_result, "greedy")


    with multiprocessing.Pool() as pool:
        tasks = []
        for instance, adjacency_matrix, tabu_lenght in matrixes:
            tasks.append((instance, adjacency_matrix, 250, tabu_lenght))
        results = pool.map(run_tabu, tasks)
        for instance, tabu_lenght, tabu_search_max, result in results:
            with open(new_file_path, "a") as file:
                file.write("----------------------------\n")
                file.write(f"Instance: {instance}\n")
                file.write(f"Tabu: {tabu_lenght}, {tabu_search_max}\n")
                file.write(f"Tabu Solution: {result}\n")
                file.write(f"Greedy: {max_color}\n")
                file.write(f"Initial Solution: {colors}\n")
                file.write("----------------------------\n")
        tabu_coloring_result_colors = {
            node: color for node, color in enumerate(results[0][3])
        }
        coloring_graph(edges, tabu_coloring_result_colors, "tabu")
