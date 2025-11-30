from graph.graph_converter import read_from_file, matrix_from_edges
from algorithms.greedy_algorithm import greedy_coloring
from algorithms.tabu_search import tabu_search
from algorithms.printing_graph import coloring_graph

if __name__ == "__main__":
    file_path = "./utils//instance/instance.txt"
    num_nodes, edges = read_from_file(file_path)
    adjacency_matrix = matrix_from_edges(num_nodes, edges)
    colors, max_color = greedy_coloring(adjacency_matrix)
    greedy_coloring_result = {node: colors[node] for node in range(num_nodes)}
    print(greedy_coloring(adjacency_matrix))
    [tabu_search_result,tabu_search_max] = tabu_search(adjacency_matrix, 60)
    print(tabu_search_result,tabu_search_max)
    tabu_coloring_result_colors = {node: tabu_search_result[node] for node in range(num_nodes)}

    
    coloring_graph(edges, greedy_coloring_result, "greedy")
    coloring_graph(edges, tabu_coloring_result_colors, "tabu")
    
