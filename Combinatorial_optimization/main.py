from graph.graph_converter import read_from_file, matrix_from_edges
from algorithms.greedy_algorithm import greedy_coloring

if __name__ == "__main__":
    file_path = "./utils/instance.txt"
    num_nodes, edges = read_from_file(file_path)
    adjacency_matrix = matrix_from_edges(num_nodes, edges)
    print(greedy_coloring(adjacency_matrix))
