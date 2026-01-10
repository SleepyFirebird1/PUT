from graph.graph_converter import read_from_file, matrix_from_edges
from algorithms.greedy_algorithm import greedy_coloring
from algorithms.tabu_search import tabu_search, run_tabu
from algorithms.printing_graph import coloring_graph
import multiprocessing

if __name__ == "__main__":
    instances = [
        "gc500",
        "gc_1000",
        "instance_1_20",
        "instance_1_100",
        "instance20",
        "instance100",
        "inst_01_50_random",
        "inst_02_150_sparse",
        "inst_03_250_flat3",
        "inst_04_400_dense",
        "inst_05_500_geo",
        "inst_06_800_flat10",
        "inst_07_1000_scale_free",
        "inst_08_1500_random_hard",
        "inst_09_2500_sparse",
        "inst_10_3500_flat50",
        "inst_11_5000_massive",
    ]
    instances2 =[
        "fpsol2.i.1",
        "mulsol.i.1",
        "mulsol.i.2",
        "mulsol.i.3",
        "flat300_20_0",
        "flat300_26_0",
        "flat300_28_0",
        "flat1000_60_0",
        "flat1000_76_0",
        "flat1000",
    ]
    for instance in instances:
        new_file_path = "./utils/presentation_results.txt"
        file_path = f"./utils//instance/{instance}.txt"
        num_nodes, edges = read_from_file(file_path)
        adjacency_matrix = matrix_from_edges(num_nodes, edges)
        colors, max_color = greedy_coloring(adjacency_matrix)

        with open(new_file_path, "a") as file:
            file.write("----------------------------\n")
            file.write(f"Instance: {instance}\n")
        with multiprocessing.Pool() as pool:
            tasks = []
            for tabu_lenght in range(1, 21):
                for i in range(0, 10):
                    tasks.append((i, adjacency_matrix, 180, tabu_lenght))
            results = pool.map(run_tabu, tasks)
            for i, tabu_lenght, tabu_search_max in results:
                with open(new_file_path, "a") as file:
                    file.write(f"Tabu: {tabu_lenght}-{i}, {tabu_search_max}\n")
                    file.write(f"Greedy: {max_color}\n")

        """
        while(True):
        dec = input("Czy chcesz zobaczyć graf(T of F):")
        if dec == "T":
            coloring_graph(edges, greedy_coloring_result, "greedy")
            coloring_graph(edges, tabu_coloring_result_colors, "tabu")
        """
