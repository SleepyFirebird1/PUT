import random


def is_neighbour(graph, x, y):
    return graph[x][y] == 1


def get_degree(graph, node):
    return graph[node].count(1)


def get_neighbours(graph, node):
    neighbours = []
    for i in range(1, len(graph)):
        if graph[node][i] == 1:
            neighbours.append(i)
    return neighbours


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


def neighbours_colors_count(graph, node, colors, max_color):
    count = [0] * (max_color + 1)
    neighbours = get_neighbours(graph, node)
    for neighbour in neighbours:
        color = colors[neighbour]
        count[color] += 1
    return count


def min_from_neighbours_colors(graph, node, colors, max_color):
    colors_list = neighbours_colors_count(graph, node, colors, max_color)
    min_c = min((val for val in colors_list if val > 0), default=float("inf"))
    if min_c == float("inf"):
        return 1
    index = []
    for i, element in enumerate(colors_list):
        if element == min_c:
            index.append(i)
    return random.choice(index)


def get_node_by_color(colors, color):
    nodes = []
    for i, node in enumerate(colors):
        if i == 0:
            continue
        if node == color:
            nodes.append(i)
    return nodes


def conflicts(graph, colors):
    conflict_nodes = []
    for node in range(1, len(graph)):
        for neighbour in get_neighbours(graph, node):
            if colors[node] == colors[neighbour]:
                if node not in conflict_nodes:
                    conflict_nodes.append(node)
    return conflict_nodes


def conflicts_for_node(graph, node, colors):
    conflicts_count = 0
    for neighbour in get_neighbours(graph, node):
        if colors[neighbour] == colors[node]:
            conflicts_count += 1
    return conflicts_count
