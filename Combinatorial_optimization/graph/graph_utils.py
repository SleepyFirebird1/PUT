def is_neighbour(graph, x, y):
    return graph[x][y] == 1


def get_degree(graph, node):
    return graph[node].count(1)


def sort_nodes_by_degree(graph):
    num_nodes = len(graph)
    degree = [0]
    for node in range(1, num_nodes):
        degree.append(get_degree(graph, node))

    node_degree_pairs = []
    for i in range(1, num_nodes):
        node_degree_pairs.append((i, degree[i]))
    node_degree_pairs.sort(key=lambda x: x[1], reverse=True)

    return [node for node, degree in node_degree_pairs]
