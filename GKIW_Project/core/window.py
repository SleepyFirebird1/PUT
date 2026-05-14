import moderngl_window as mglw
import glm
from game.board import Board
from utils.raycasting import calculate_click_raycast
import numpy as np


class Window(mglw.WindowConfig):
    # Wymuszamy wersję OpenGL 3.3 (kompatybilną z macOS)
    gl_version = (3, 3)
    title = "Go 3D"
    window_size = (3840, 2160)
    aspect_ratio = None
    samples = 8  # Wygładzanie krawędzi (MSAA 8x) - eliminuje szarpanie ekranu w ruchu

    resource_dir = "assets"

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.CELL_SPACING = 0.026
        self.TABLE_HEIGTH = 0.27

        self.board = Board()

        # Obsługa myszki
        self.mouse_pos = (0, 0)
        self.left_mouse_button = False

        # Głebokość 3D
        self.ctx.enable(self.ctx.DEPTH_TEST | self.ctx.CULL_FACE)

        # Ładowanie modeli 3D
        try:
            self.table_scene = self.load_scene("Go-table.glb")
            self.stone_black = self.load_scene("Stone-black.glb")
            self.stone_white = self.load_scene("Stone-white.glb")

            # Włączenie MipMap i filtrowania by wygładzić teksturę o dużej skali (tzw. anti-aliasing)
            if self.table_scene:
                for mat in self.table_scene.materials:
                    if mat.mat_texture and mat.mat_texture.texture:
                        tex = mat.mat_texture.texture
                        tex.build_mipmaps()
                        tex.filter = (self.ctx.LINEAR_MIPMAP_LINEAR, self.ctx.LINEAR)
                        tex.anisotropy = 16.0  # maksymalne filtrowanie anizotropowe

        except Exception as e:
            print(f"Nie znaleziono modelu: {e}")
            self.table_scene = None
            self.stone_black = None
        # Ustawienie kamery
        # Lista dostępnych widoków: "isometric", "top_down"
        self.set_camera_preset("isometric")

        # Macierz perspektywy z większym FOV w celu zachowania wielkości stołu przy 4K
        self.projection = glm.perspective(glm.radians(102.33), 16 / 9, 0.1, 1000.0)

    def resize(self, width: int, height: int):
        if height == 0:
            return
        actual_aspect_ratio = width / height
        self.projection = glm.perspective(
            glm.radians(102.33), actual_aspect_ratio, 0.1, 1000.0
        )

    def on_render(self, time, frame_time):
        self.ctx.clear(0.53, 0.81, 0.92, 1.0)
        # Bezpieczne sprawdzanie orbity i płynne animowanie kamery
        if (
            getattr(self, "camera_mode", "isometric") == "isometric"
            and hasattr(self, "target_angle")
            and self.camera_angle != self.target_angle
        ):
            # Płynne podążanie kąta (Delta Time * prędkosć interpolacji 5.0)
            self.camera_angle += (
                (self.target_angle - self.camera_angle) * frame_time * 1.75
            )

            # Wyrównanie w celu unikniecia nieskoñczenie mikroskopijnych drgań
            if abs(self.target_angle - self.camera_angle) < 0.001:
                self.camera_angle = self.target_angle

            self.camera_pos.x = np.sin(self.camera_angle) * self.camera_radius
            self.camera_pos.z = np.cos(self.camera_angle) * self.camera_radius

        if not self.table_scene:
            return

        # Macierz widoku
        view = glm.lookAt(self.camera_pos, self.camera_target, self.up_vector)

        # Rysowanie sceny ze stołem
        self.table_scene.draw(
            projection_matrix=self.projection,
            camera_matrix=view,
        )
        for stone in self.board.stones:
            offset_x = (stone["grid_x"] - 9) * self.CELL_SPACING
            offset_y = self.TABLE_HEIGTH
            offset_z = (stone["grid_y"] - 9) * self.CELL_SPACING

            model_to_draw = (
                self.stone_black if stone["color"] == "black" else self.stone_white
            )
            if model_to_draw:
                # Macierz jednostkowa
                base_matrix = glm.mat4(1.0)
                # Translacja
                translate_matrix = glm.translate(
                    base_matrix, glm.vec3(offset_x, offset_y, offset_z)
                )
                # Skalowanie
                scale_matrix = glm.scale(translate_matrix, glm.vec3(0.25, 0.25, 0.25))

                for node in model_to_draw.root_nodes:
                    if node.mesh:
                        node.mesh.draw(
                            projection_matrix=self.projection,
                            camera_matrix=view,
                            model_matrix=scale_matrix,
                        )

    def on_mouse_press_event(self, x, y, button):
        if button != 1:
            return
        window_width, window_height = self.wnd.size

        result = calculate_click_raycast(
            x,
            y,
            (window_width, window_height),
            self.projection,
            self.camera_pos,
            self.camera_target,
            self.up_vector,
            self.TABLE_HEIGTH,
            self.CELL_SPACING,
        )
        if result is not None:
            grid_x, grid_y = result
            if self.board.check_stone(grid_x, grid_y):
                self.board.delete_stone(grid_x, grid_y)
            else:
                if self.board.add_stone(grid_x, grid_y):
                    # Dodaj 180 stopni do target angle po pomyślnym postawieniu kamienia (oś obrotu P)
                    if hasattr(self, "target_angle"):
                        self.target_angle += np.pi

    def set_camera_preset(self, preset="isometric"):
        self.camera_mode = preset
        if preset == "isometric" or preset == "start":
            self.camera_radius = 0.80
            self.camera_height = 0.65
            # Obrót początkowy o 90 stopni (pi/2) by celowała na prawidłowy krótki bok stolu
            self.camera_angle = np.pi / 2
            self.target_angle = self.camera_angle

            self.camera_pos = glm.vec3(
                np.sin(self.camera_angle) * self.camera_radius,
                self.camera_height,
                np.cos(self.camera_angle) * self.camera_radius,
            )
            self.camera_target = glm.vec3(0.0, 0.05, 0.0)
            self.up_vector = glm.vec3(0.0, 1.0, 0.0)
        elif preset == "top_down":
            self.camera_pos = glm.vec3(0.0, 0.75, 0.0)
            self.camera_target = glm.vec3(0.0, 0.0, 0.0)
            self.up_vector = glm.vec3(0.0, 0.0, -1.0)
            self.camera_angle = 0
            self.target_angle = 0
