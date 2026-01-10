import networkx as nx
import random
import os

def save_graph(G, filename):
    # Przemianowanie wierzchołków na 1..N
    G = nx.convert_node_labels_to_integers(G, first_label=1)
    
    with open(filename, 'w') as f:
        f.write(f"{G.number_of_nodes()}\n")
        for u, v in G.edges():
            f.write(f"{u} {v}\n")
    print(f"Wygenerowano: {filename} (N={G.number_of_nodes()}, E={G.number_of_edges()})")

if not os.path.exists("utils/instance"):
    os.makedirs("utils/instance")

# 1. Mały Random (50, 0.5)
G1 = nx.erdos_renyi_graph(50, 0.5)
save_graph(G1, "utils/instance/inst_01_50_random.txt")

# 2. Mały Rzadki (150, 0.1)
G2 = nx.erdos_renyi_graph(150, 0.1)
save_graph(G2, "utils/instance/inst_02_150_sparse.txt")

# 3. 3-Partite (Ukryte optimum = 3, N=250)
# Tworzymy 3 grupy i łączymy je losowo, wewnątrz grup brak krawędzi
G3 = nx.planted_partition_graph(3, 84, p_in=0, p_out=0.5) # ~252 wierzchołki
save_graph(G3, "utils/instance/inst_03_250_flat3.txt")

# 4. Gęsty Random (400, 0.8)
G4 = nx.erdos_renyi_graph(400, 0.8)
save_graph(G4, "utils/instance/inst_04_400_dense.txt")

# 5. Geometryczny (500) - symuluje klastry
G5 = nx.random_geometric_graph(500, 0.125)
save_graph(G5, "utils/instance/inst_05_500_geo.txt")

# 6. 10-Partite (Ukryte optimum = 10, N=800)
G6 = nx.planted_partition_graph(10, 80, p_in=0, p_out=0.3)
save_graph(G6, "utils/instance/inst_06_800_flat10.txt")

# 7. Scale-Free (1000) - Huby
G7 = nx.barabasi_albert_graph(1000, 10)
save_graph(G7, "utils/instance/inst_07_1000_scale_free.txt")

# 8. Trudny Random (1500, 0.5) - typu DSJC
G8 = nx.erdos_renyi_graph(1500, 0.5)
save_graph(G8, "utils/instance/inst_08_1500_random_hard.txt")

# 9. Duży rzadki (2500, 0.05)
G9 = nx.erdos_renyi_graph(2500, 0.05)
save_graph(G9, "utils/instance/inst_09_2500_sparse.txt")

# 10. 50-Partite (Ukryte optimum = 50, N=3500)
G10 = nx.planted_partition_graph(50, 70, p_in=0, p_out=0.05)
save_graph(G10, "utils/instance/inst_10_3500_flat50.txt")

# 11. Massive Sparse (5000, 0.01)
# Uwaga: Gęstość bardzo mała, by plik nie ważył 100MB
G11 = nx.erdos_renyi_graph(5000, 0.002) 
save_graph(G11, "utils/instance/inst_11_5000_massive.txt")