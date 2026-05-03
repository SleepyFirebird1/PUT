class Board:
    def __init__(self):
        # Lista słowników przykład elementu: {'color: 'black', 'grid_x': 5, 'grid_y': 5}
        self.stones = []

        self.current_turn = "black"

    def add_stone(self, grid_x, grid_y):
        for stone in self.stones:
            if stone["grid_x"] == grid_x and stone["grid_y"] == grid_y:
                return False

        self.stones.append(
            {"color": self.current_turn, "grid_x": grid_x, "grid_y": grid_y}
        )

        if self.current_turn == "black":
            self.current_turn = "white"
        else:
            self.current_turn = "black"
        return True
