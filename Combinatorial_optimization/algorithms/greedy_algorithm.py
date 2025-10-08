from graph.graph_utils import is_neighbour, sort_nodes_by_degree


def greedy_coloring(graph):
    num_nodes = len(graph)
    color = [0] * (num_nodes)
    queue = sort_nodes_by_degree(graph)

    for node in queue:
        lowest = 1
        for i in range(1, num_nodes):
            if is_neighbour(graph, node, i) and color[i] == lowest:
                lowest += 1
        color[node] = lowest

    return color[1:]
