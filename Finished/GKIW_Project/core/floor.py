"""
@file floor.py
@brief Komponent odpowiedzialny za generowanie i renderowanie płaszczyzny ziemi.
"""

import numpy as np
import glm


class Floor:
    """
    @class Floor
    @brief Klasa reprezentująca płaszczyznę ziemi bezpośrednio w ModernGL.
    @details Generuje geometrię o wymiarach 100x100m, konfiguruje bufory GPU (VBO, IBO) oraz VAO dla cieni i kolorów.
    """

    def __init__(self, ctx, depth_program, solid_shader):
        """
        @brief Inicjalizacja płaszczyzny ziemi i buforów GPU.
        @param ctx Kontekst ModernGL.
        @param depth_program Program cieniowania głębi (Shadow Map).
        @param solid_shader Program cieniowania jednolitego (solid color).
        """
        # Definicja wierzchołków (format: x, y, z, nx, ny, nz)
        vertices = np.array(
            [
                -50.0,
                -0.001,
                -50.0,
                0.0,
                1.0,
                0.0,
                -50.0,
                -0.001,
                50.0,
                0.0,
                1.0,
                0.0,
                50.0,
                -0.001,
                -50.0,
                0.0,
                1.0,
                0.0,
                50.0,
                -0.001,
                50.0,
                0.0,
                1.0,
                0.0,
            ],
            dtype="f4",
        )
        indices = np.array([0, 1, 2, 2, 1, 3], dtype="i4")

        # Inicjalizacja buforów na GPU
        self.vbo = ctx.buffer(vertices.tobytes())
        self.ibo = ctx.buffer(indices.tobytes())

        # VAO dla cieni (depth pass) - bez normalnych (pominięte za pomocą 3x4)
        self.vao_depth = ctx.vertex_array(
            depth_program,
            [(self.vbo, "3f 3x4", "in_position")],
            index_buffer=self.ibo,
        )

        # VAO dla kolorowania (color pass) - z normalnymi
        self.vao_color = ctx.vertex_array(
            solid_shader,
            [(self.vbo, "3f 3f", "in_position", "in_normal")],
            index_buffer=self.ibo,
        )

        self.solid_shader = solid_shader
        self.depth_program = depth_program

    def draw(self, view, projection, is_depth_pass=False):
        """
        @brief Rysowanie płaszczyzny ziemi z oświetleniem i cieniowaniem.
        @param view Macierz widoku kamery.
        @param projection Macierz rzutowania perspektywicznego.
        @param is_depth_pass Flaga określająca, czy renderowana jest mapa cieni (głębi).
        """
        identity = glm.mat4(1.0)
        if is_depth_pass:
            self.depth_program["m_model"].write(identity)
            self.vao_depth.render()
        else:
            self.solid_shader["m_model"].write(identity)
            self.solid_shader["m_proj"].write(projection)
            self.solid_shader["m_cam"].write(view)
            self.solid_shader["baseColor"].value = (
                0.16,
                0.28,
                0.13,
                1.0,
            )  # Odcień butelkowej zieleni (trawa)
            self.vao_color.render()
