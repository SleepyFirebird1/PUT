def read_from_file(file):
    with open(file) as f:
        num_nodes = int(f.readline().strip())
        lines = [line.strip() for line in f]

    edges = []
    for line in lines:
        x, y = map(int, line.split())
        edges.append((x, y))

    return num_nodes, edges


def matrix_from_edges(num_nodes, edges):

    matrix = []

    for i in range(num_nodes + 1):
        matrix.append([0 for i in range(num_nodes + 1)])

    for x, y in edges:
        matrix[x][y] = 1
        matrix[y][x] = 1

    return matrix
