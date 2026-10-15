"""
@file window.py
@brief Główny plik aplikacji Go 3D kontrolujący okno, cykl życia renderowania i wejście użytkownika.
"""

import moderngl_window as mglw
import glm
from game.board import Board
from game.scene import Scene
from utils.raycasting import calculate_click_raycast
import numpy as np
from core.mesh_programs import (
    CustomMeshProgram,
    CustomSolidMeshProgram,
    apply_custom_shaders,
    draw_depth_node_global,
    draw_node_with_matrix,
)
from core.floor import Floor
from core.score_display import ScoreDisplay


class Window(mglw.WindowConfig):
    """
    @class Window
    @brief Główne okno aplikacji odpowiedzialne za renderowanie sceny, obsługę kamery oraz interakcję z użytkownikiem.
    @details Klasa konfiguruje okno gry, inicjalizuje bufory mapy cieni, zarządza dwuetapową pętlą renderowania oraz przechwytuje zdarzenia wejścia (mysz/raycasting).
    """

    # Wymuszenie wersji OpenGL 3.3 (kompatybilnej z macOS)
    gl_version = (3, 3)
    title = "Go 3D"
    window_size = (3840, 2160)
    aspect_ratio = None
    samples = 8  # Wygładzanie krawędzi MSAA 8x

    resource_dir = "assets"

    def __init__(self, **kwargs):
        """
        @brief Inicjalizacja gry, ładowanie modeli, shaderów, konfiguracja oświetlenia i mapy cieni.
        @param kwargs Dodatkowe argumenty konfiguracji okna.
        """
        super().__init__(**kwargs)
        self.CELL_SPACING = 0.026
        self.TABLE_HEIGTH = 0.27

        self.board = Board()
        self.scene = Scene()

        # Generowanie trawy
        self.scene.generation((0.4, 0.4), (5.0, 5.0), (0.9, 0.6), 15)

        # Włączenie testu głębokości oraz usuwania niewidocznych ścian
        self.ctx.enable(self.ctx.DEPTH_TEST | self.ctx.CULL_FACE)

        # Ładowanie modeli 3D
        try:
            self.table_scene = self.load_scene("Go_tablev2.glb")
            self.stone_black = self.load_scene("Stone-black.glb")
            self.stone_white = self.load_scene("Stone-white.glb")
            self.grass = self.load_scene("Grass.glb")

        except Exception as e:
            print(f"Nie znaleziono modelu: {e}")
            self.table_scene = None
            self.stone_black = None

        # Konfiguracja shaderów i mapy cieni
        try:
            self.custom_shader = self.load_program("custom_light.glsl")
            self.solid_shader = self.load_program("custom_light_solid.glsl")
            self.depth_program = self.load_program("depth.glsl")

            # Konfiguracja mapy cieni (Shadow Map)
            self.shadow_size = (2048, 2048)
            self.shadow_map = self.ctx.depth_texture(self.shadow_size)
            self.shadow_fbo = self.ctx.framebuffer(depth_attachment=self.shadow_map)

            self.light_pos = glm.vec3(0, 1, 1)

            # Obliczanie macierzy rzutowania światła dla cieni
            light_proj = glm.ortho(-2.5, 2.5, -2.5, 2.5, 0.1, 10.0)
            light_view = glm.lookAt(
                self.light_pos, glm.vec3(0, 0, 0), glm.vec3(0, 1, 0)
            )
            self.light_space_matrix = light_proj * light_view

            # Konfiguracja parametrów oświetlenia otoczenia i światła punktowego
            for shader in [self.custom_shader, self.solid_shader]:
                shader["ambientColor"].value = (0.2, 0.25, 0.35)
                shader["ambientPower"].value = 1

                shader["tableLightPos"].value = tuple(self.light_pos)
                shader["tableLightColor"].value = (1.0, 0.9, 0.7)
                shader["tableLightPower"].value = 1

                if "shadowMap" in shader:
                    shader["shadowMap"].value = 1
                if "m_light_space" in shader:
                    shader["m_light_space"].write(self.light_space_matrix)

            custom_prog = CustomMeshProgram(self.custom_shader)
            solid_prog = CustomSolidMeshProgram(self.solid_shader)

            # Przypisanie shaderów do wczytanych modeli
            apply_custom_shaders(self.table_scene, custom_prog, solid_prog)
            apply_custom_shaders(self.stone_black, custom_prog, solid_prog)
            apply_custom_shaders(self.stone_white, custom_prog, solid_prog)
            apply_custom_shaders(self.grass, custom_prog, solid_prog)

            # Inicjalizacja płaszczyzny ziemi
            self.floor = Floor(self.ctx, self.depth_program, self.solid_shader)

            # Inicjalizacja wyświetlacza punktów
            self.score_display = ScoreDisplay(
                self.ctx, self.solid_shader, self.depth_program
            )

        except Exception as e:
            print(f"Nie udało się załadować shadera: {e}")

        # Ustawienie domyślnego widoku kamery
        self.set_camera_preset("isometric")

        # Ustawienie macierzy perspektywy
        self.projection = glm.perspective(glm.radians(102.33), 16 / 9, 0.1, 1000.0)

    def resize(self, width: int, height: int):
        """
        @brief Aktualizacja stosunku boków projekcji przy zmianie rozmiaru okna.
        @param width Nowa szerokość okna.
        @param height Nowa wysokość okna.
        """
        if height == 0:
            return
        actual_aspect_ratio = width / height
        self.projection = glm.perspective(
            glm.radians(102.33), actual_aspect_ratio, 0.1, 1000.0
        )

    def draw_scene_objects(self, view, projection, is_depth_pass=False):
        """
        @brief Rysowanie wszystkich fizycznych obiektów sceny (podłoga, stół, trawa, kamienie).
        @param view Macierz widoku kamery (lub światła).
        @param projection Macierz rzutowania kamery (lub światła).
        @param is_depth_pass Flaga informująca, czy wykonywane jest renderowanie głębi (Shadow Map).
        """
        # Rysowanie płaszczyzny ziemi
        if hasattr(self, "floor") and self.floor:
            self.floor.draw(view, projection, is_depth_pass)

        # Rysowanie stołu
        if is_depth_pass:
            self.depth_program["m_light_space"].write(self.light_space_matrix)
            for node in self.table_scene.root_nodes:
                draw_depth_node_global(node, self.depth_program)
        else:
            self.table_scene.draw(
                projection_matrix=projection,
                camera_matrix=view,
            )

        # Rysowanie wygenerowanej trawy
        if self.grass:
            for gx, gz in self.scene.objects:
                base_matrix = glm.mat4(1.0)
                translate_matrix = glm.translate(base_matrix, glm.vec3(gx, 0.0, gz))
                scale_matrix = glm.scale(translate_matrix, glm.vec3(0.02, 0.02, 0.02))

                for node in self.grass.root_nodes:
                    draw_node_with_matrix(
                        node,
                        self.depth_program,
                        projection,
                        view,
                        scale_matrix,
                        is_depth_pass,
                    )

        # Rysowanie kamieni na planszy
        for stone in self.board.stones:
            offset_x = (stone["grid_x"] - 9) * self.CELL_SPACING
            offset_y = self.TABLE_HEIGTH
            offset_z = (stone["grid_y"] - 9) * self.CELL_SPACING

            model_to_draw = (
                self.stone_black if stone["color"] == "black" else self.stone_white
            )
            if model_to_draw:
                base_matrix = glm.mat4(1.0)
                translate_matrix = glm.translate(
                    base_matrix, glm.vec3(offset_x, offset_y, offset_z)
                )
                scale_matrix = glm.scale(translate_matrix, glm.vec3(0.25, 0.25, 0.25))

                for node in model_to_draw.root_nodes:
                    draw_node_with_matrix(
                        node,
                        self.depth_program,
                        projection,
                        view,
                        scale_matrix,
                        is_depth_pass,
                    )

        # Rysowanie wyników w 3D
        if hasattr(self, "score_display"):
            black_count = sum(
                1 for stone in self.board.stones if stone["color"] == "black"
            )
            white_count = sum(
                1 for stone in self.board.stones if stone["color"] == "white"
            )

            # Punktacja czarnych
            black_pos = glm.vec3(0.0, self.TABLE_HEIGTH, -0.45)
            self.score_display.draw_number(
                black_count,
                black_pos,
                np.pi / 2,
                (0.1, 0.1, 0.1, 1.0),
                projection,
                view,
                is_depth_pass,
            )

            # Punktacja białych
            white_pos = glm.vec3(0.0, self.TABLE_HEIGTH, 0.45)
            self.score_display.draw_number(
                white_count,
                white_pos,
                np.pi / 2,
                (0.9, 0.9, 0.9, 1.0),
                projection,
                view,
                is_depth_pass,
            )

    def on_render(self, time, frame_time):
        """
        @brief Główna pętla renderująca wywoływana w każdej ramce. Kontroluje obrót kamery i dwuetapowe rysowanie.
        @param time Czas systemowy od startu okna.
        @param frame_time Delta czasu od poprzedniej klatki.
        """
        if (
            getattr(self, "camera_mode", "isometric") == "isometric"
            and hasattr(self, "target_angle")
            and self.camera_angle != self.target_angle
        ):
            self.camera_angle += (
                (self.target_angle - self.camera_angle) * frame_time * 1.75
            )

            # Wyrównanie kąta kamery w celu eliminacji drgań
            if abs(self.target_angle - self.camera_angle) < 0.001:
                self.camera_angle = self.target_angle

            self.camera_pos.x = np.sin(self.camera_angle) * self.camera_radius
            self.camera_pos.z = np.cos(self.camera_angle) * self.camera_radius

        if not self.table_scene:
            return

        # Obliczanie macierzy widoku kamery
        view = glm.lookAt(self.camera_pos, self.camera_target, self.up_vector)

        # Generowanie mapy głębokości
        self.shadow_fbo.use()
        self.shadow_fbo.clear(depth=1.0)
        self.draw_scene_objects(view, self.projection, is_depth_pass=True)

        # Renderowanie właściwej sceny z kolorami i cieniowaniem
        self.wnd.fbo.use()
        self.ctx.clear(0.05, 0.05, 0.08, 1.0)
        self.shadow_map.use(location=1)
        self.draw_scene_objects(view, self.projection, is_depth_pass=False)

    def on_mouse_press_event(self, x, y, button):
        """
        @brief Obsługa kliknięcia myszy, raycastingu i interakcji z kamieniami planszy Go.
        @param x Współrzędna X kursora w pikselach okna.
        @param y Współrzędna Y kursora w pikselach okna.
        @param button Kliknięty przycisk (lewy przycisk = 1).
        """
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
                    if hasattr(self, "target_angle"):
                        self.target_angle += np.pi

    def set_camera_preset(self, preset="isometric"):
        """
        @brief Ustawienie wstępnie zdefiniowanej pozycji i orientacji kamery.
        @param preset Nazwa układu kamery (np. "isometric").
        """
        self.camera_mode = preset
        if preset == "isometric" or preset == "start":
            self.camera_radius = 0.80
            self.camera_height = 0.65
            self.camera_angle = np.pi / 2
            self.target_angle = self.camera_angle

            self.camera_pos = glm.vec3(
                np.sin(self.camera_angle) * self.camera_radius,
                self.camera_height,
                np.cos(self.camera_angle) * self.camera_radius,
            )
            self.camera_target = glm.vec3(0.0, 0.05, 0.0)
            self.up_vector = glm.vec3(0.0, 1.0, 0.0)
