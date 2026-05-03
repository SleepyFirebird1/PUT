import moderngl_window as mglw
from pyrr import Matrix44, Vector3
import math


class Window(mglw.WindowConfig):
    # Wymuszamy wersję OpenGL 3.3 (kompatybilną z macOS)
    gl_version = (3, 3)
    title = "Go 3D"
    window_size = (1280, 720)
    aspect_ratio = 16 / 9

    resource_dir = "assets"

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # Głebokość 3D
        self.ctx.enable(self.ctx.DEPTH_TEST | self.ctx.CULL_FACE)

        # Ładowanie modeli 3D
        try:
            self.table_scene = self.load_scene("Go-table.glb")
            self.stone_black = self.load_scene("Stone-black.glb")
        except Exception as e:
            print(f"Nie znaleziono modelu: {e}")
            self.table_scene = None
            self.stone_black = None
        # Ustawienie kamery
        self.camera_pos = Vector3(
            [0.0, 0.80, 0.75]
        )  # Kamera jest w górze(Y) i przesunięta w stronę widza(Z)
        self.camera_target = Vector3([0.0, 0.0, 0.0])  # Kamera patrzy w środek stołu
        self.up_vector = Vector3(
            [0.0, 1.0, 0.0]
        )  # Wektor wskazujący, gdzie jest "Góra" dla kamery

        # Macierz perspektywy
        self.projection = Matrix44.perspective_projection(
            45, self.aspect_ratio, 0.1, 1000.0
        )

    # Używamy on_render, aby zaspokoić nowszą wersję biblioteki
    def on_render(self, time, frame_time):
        self.ctx.clear(0.2, 0.3, 0.3, 1.0)

        if not self.table_scene:
            return

        # Macierz widoku
        view = Matrix44.look_at(self.camera_pos, self.camera_target, self.up_vector)

        # Rysowanie sceny ze stołem
        self.table_scene.draw(
            projection_matrix=self.projection.astype("f4"),
            camera_matrix=view.astype("f4"),
        )
        # 2. Rysujemy TESTOWY "czarny kamyczek"
        if self.stone_black:
            # Tworzymy ostateczną macierz łączącą przemieszczenie (Gdzie w grze) i skalę
            scale_matrix = Matrix44.from_scale([0.3, 0.3, 0.3])
            translate_matrix = Matrix44.from_translation([0.0, 0.30, 0.0])
            stone_matrix = (translate_matrix * scale_matrix).astype(
                "f4"
            )  # PRZEŻARTY SYSTEM OMIJAJĄCY BUG .draw() -> Dla każdego Node z Mesh, podaj macierz MODELU
            for node in self.stone_black.root_nodes:
                if node.mesh:  # To omija martwe wezły i swiatła
                    node.mesh.draw(
                        projection_matrix=self.projection.astype("f4"),
                        camera_matrix=view.astype("f4"),
                        model_matrix=stone_matrix,
                    )
