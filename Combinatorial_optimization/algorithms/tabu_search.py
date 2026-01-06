import time
import random

from graph.graph_utils import (
    get_node_by_color,
    min_from_neighbours_colors,
    conflicts,
    conflicts_for_node,
)
from algorithms.greedy_algorithm import greedy_coloring


def tabu_initialize(graph, colored_graph_1):
    if colored_graph_1:
        colored_graph = list(colored_graph_1)
        max_color = max(colored_graph)
    else:
        colored_graph, max_color = greedy_coloring(graph)
    goal = max_color - 1
    targets_list = get_node_by_color(colored_graph, max_color)
    for target in targets_list:
        replacement = min_from_neighbours_colors(graph, target, colored_graph, goal)
        colored_graph[target] = replacement
    return colored_graph, goal


def tabu_search(graph, t, max_lenght=10):
    start_time = time.time()
    initial_solution, _ = greedy_coloring(graph)
    best_solution = list(initial_solution)
    last_improvement_time = start_time

    while time.time() - start_time < t:
        temp_solution, max_color = tabu_initialize(graph, best_solution)
        tabu_max_lenght = max_lenght
        tabu = []
        conflicts_list = conflicts(graph, temp_solution)
        while conflicts_list and time.time() - start_time < t:
            if time.time() - last_improvement_time > 30:
                break
            conflict_node = random.choice(conflicts_list)
            conflicts_for_each_color = [float("inf")] * (max_color + 1)
            # check every possible solution
            for color in range(1, max_color + 1):
                temp_solution_cp = list(temp_solution)
                temp_solution_cp[conflict_node] = color
                conflicts_for_each_color[color] = conflicts_for_node(
                    graph, conflict_node, temp_solution_cp
                )
            # tabu filter except instant solution, if 0 move alowed
            for color_idx, conflict_count in enumerate(conflicts_for_each_color):
                if conflict_count == float("inf"):
                    continue
                if (conflict_node, color_idx) in tabu and conflict_count > 0:
                    conflicts_for_each_color[color_idx] = float("inf")
            # choose move
            min_conflict_val = min(conflicts_for_each_color)
            if min_conflict_val == float("inf"):
                continue
            candidates = [
                color
                for color, val in enumerate(conflicts_for_each_color)
                if val == min_conflict_val
            ]
            # execute move
            pre_move = temp_solution[conflict_node]
            move = random.choice(candidates)
            temp_solution[conflict_node] = move
            tabu.insert(0, (conflict_node, pre_move))
            if len(tabu) > tabu_max_lenght:
                tabu.pop()
            conflicts_list = conflicts(graph, temp_solution)
        if not conflicts_list:
            best_solution = list(temp_solution)
            last_improvement_time = time.time()
    return best_solution, max(best_solution)


def run_tabu(args):
    i, matrix, t, tabu_lenght = args
    print(f"Algorithm starts {i}\n")
    result, max_val = tabu_search(matrix, t, tabu_lenght)
    return i, tabu_lenght, max_val
