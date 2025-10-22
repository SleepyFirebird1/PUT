import random

while True:
    try:
        n = int(input("Number of nodes: "))
        if n > 0:
            break
        print("Please enter a positive integer")
    except ValueError:
        print("Please enter a positive integer")

while True:
    try:
        s = int(input("Saturation (0-100 with step 1): "))
        if s > 0 and s <= 100:
            break
        print("Please enter a positive integer between 1-100")
    except ValueError:
        print("Please enter a positive integer between 1-100")

max_edges = n * (n - 1) // 2
num_edges = max_edges * s // 100
edges = []

i = 0
while i < num_edges:
    x = random.randint(1, n)
    if x + 1 > n:
        continue
    y = random.randint(x + 1, n)
    if (x, y) in edges:
        continue
    edges.append((x, y))
    i += 1

edges.sort()

with open("instance.txt", "w") as f:
    print(n, file=f)
    for edge in edges:
        print(*edge, file=f)

print("Edges generated check instance.txt")
