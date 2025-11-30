import networkx as nx
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

# Dodałem argument 'filename_suffix', żeby rozróżnić pliki (np. "greedy" vs "tabu")
def coloring_graph(edges_list, coloring_result, filename_suffix="result"):
    G = nx.Graph()
    G.add_edges_from(edges_list)

    print(f"Rysowanie grafu: {filename_suffix}...")

    
    if not coloring_result:
        max_color_index = 0
    else:
        max_color_index = max(coloring_result.values())
    
    cmap = plt.get_cmap('hsv')
    
    node_colors = []
    try:
        for node in G.nodes():
            
            if node in coloring_result:
                color_idx = coloring_result[node]
            elif (int(node) - 1) in coloring_result: # Próba konwersji 1-based na 0-based
                color_idx = coloring_result[int(node) - 1]
            else:
                color_idx = 0 
            
            rgba = cmap(color_idx / (max_color_index + 1) if max_color_index > 0 else 0.5)
            node_colors.append(rgba)
            
    except Exception as e:
        print(f"Błąd podczas przypisywania kolorów: {e}")
        return

    plt.figure(figsize=(10, 8))
    pos = nx.spring_layout(G, seed=42) 

    nx.draw(
        G, 
        pos,
        with_labels=True,
        node_color=node_colors,  
        node_size=500,
        font_color='black', # Czcionka czarna jest często czytelniejsza na kolorach
        font_weight='bold',
        edge_color='lightgray',
        width=1.0
    )

    title = f"Kolorowanie: {filename_suffix} (Max kolor: {max_color_index})"
    plt.title(title)

    filename = f"Graphs_plots/wynik_{filename_suffix}.png"
    plt.savefig(filename)
    print(f"Wykres zapisano jako {filename}")
    
    plt.close() 