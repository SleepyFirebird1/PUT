from graph.graph_converter import read_from_file, matrix_from_edges
from algorithms.greedy_algorithm import greedy_coloring
from algorithms.tabu_search import tabu_search

if __name__ == "__main__":
    file_path = "./utils//instance/instance100.txt"
    num_nodes, edges = read_from_file(file_path)
    adjacency_matrix = matrix_from_edges(num_nodes, edges)
    colors, max_color = greedy_coloring(adjacency_matrix)
    print(greedy_coloring(adjacency_matrix))
    print(tabu_search(adjacency_matrix, 60))
