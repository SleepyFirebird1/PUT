"""
@file mesh_programs.py
@brief Klasy i funkcje pomocnicze do obsługi programów renderowania siatek (MeshProgram) w ModernGL.
"""

import moderngl_window as mglw
import glm


class CustomMeshProgram(mglw.scene.MeshProgram):
    """
    @class CustomMeshProgram
    @brief Program renderowania siatek z obsługą tekstur i cieni.
    @details Wykorzystuje shader z obsługą oświetlenia punktowego, mapy cieni oraz mapowania UV (TEXCOORD_0).
    """

    def __init__(self, program):
        """
        @brief Inicjalizacja programu.
        @param program Instancja programu cieniowania ModernGL.
        """
        super().__init__(program=program)

    def draw(
        self,
        mesh,
        projection_matrix: glm.mat4,
        model_matrix: glm.mat4,
        camera_matrix: glm.mat4,
        time: float = 0.0,
    ) -> None:
        """
        @brief Rysowanie siatki z teksturowaniem i oświetleniem.
        @param mesh Siatka modelu do narysowania.
        @param projection_matrix Macierz rzutowania perspektywicznego.
        @param model_matrix Macierz transformacji modelu.
        @param camera_matrix Macierz widoku kamery.
        @param time Czas systemowy (opcjonalny).
        """
        if (
            mesh.material
            and hasattr(mesh.material, "mat_texture")
            and mesh.material.mat_texture
            and mesh.material.mat_texture.texture
        ):
            mesh.material.mat_texture.texture.use(0)
            if "texture0" in self.program:
                self.program["texture0"].value = 0

        self.program["m_proj"].write(projection_matrix)
        self.program["m_model"].write(model_matrix)
        self.program["m_cam"].write(camera_matrix)
        mesh.vao.render(self.program)


class CustomSolidMeshProgram(mglw.scene.MeshProgram):
    """
    @class CustomSolidMeshProgram
    @brief Program renderowania siatek bezteksturowych o jednolitym kolorze.
    @details Wykorzystuje baseColor pobierany z Blenderowego materiału lub zadany ręcznie, zachowując cienie i oświetlenie.
    """

    def __init__(self, program):
        """
        @brief Inicjalizacja programu.
        @param program Instancja programu cieniowania ModernGL.
        """
        super().__init__(program=program)

    def draw(
        self,
        mesh,
        projection_matrix: glm.mat4,
        model_matrix: glm.mat4,
        camera_matrix: glm.mat4,
        time: float = 0.0,
    ) -> None:
        """
        @brief Rysowanie siatki o jednolitym kolorze.
        @param mesh Siatka modelu do narysowania.
        @param projection_matrix Macierz rzutowania perspektywicznego.
        @param model_matrix Macierz transformacji modelu.
        @param camera_matrix Macierz widoku kamery.
        @param time Czas systemowy (opcjonalny).
        """
        # Użycie wbudowanego koloru materiału z Blendera
        if mesh.material and mesh.material.color:
            self.program["baseColor"].value = tuple(mesh.material.color)
        else:
            self.program["baseColor"].value = (1.0, 1.0, 1.0, 1.0)

        self.program["m_proj"].write(projection_matrix)
        self.program["m_model"].write(model_matrix)
        self.program["m_cam"].write(camera_matrix)
        mesh.vao.render(self.program)


def apply_custom_shaders(scene_model, custom_prog, solid_prog):
    """
    @fn apply_custom_shaders
    @brief Stosowanie odpowiednich programów cieniowania (teksturowanych lub jednolitych) dla siatek modelu.
    @param scene_model Model sceny 3D do przetworzenia.
    @param custom_prog Program cieniowania z obsługą tekstur.
    @param solid_prog Program cieniowania jednolitego (solid).
    """
    # Stosowanie odpowiednich programów cieniowania (teksturowanych lub jednolitych) dla siatek modelu.
    if not scene_model:
        return
    for mesh in scene_model.meshes:
        has_texture = (
            mesh.material
            and hasattr(mesh.material, "mat_texture")
            and mesh.material.mat_texture
            and mesh.material.mat_texture.texture
        )
        if "TEXCOORD_0" in mesh.attributes and has_texture:
            mesh.mesh_program = custom_prog
        else:
            mesh.mesh_program = solid_prog


def draw_depth_node_global(node, depth_program):
    """
    @fn draw_depth_node_global
    @brief Rekurencyjne renderowanie głębokości węzła na podstawie macierzy globalnej.
    @param node Aktualny węzeł sceny 3D.
    @param depth_program Program cieniowania głębi (Shadow Map).
    """
    # Rekurencyjne renderowanie głębokości węzła na podstawie macierzy globalnej.
    if node.mesh:
        depth_program["m_model"].write(node.matrix_global)
        node.mesh.vao.render(depth_program)
    if hasattr(node, "children"):
        for child in node.children:
            draw_depth_node_global(child, depth_program)


def draw_node_with_matrix(
    node, depth_program, projection_matrix, camera_matrix, matrix, is_depth_pass
):
    """
    @fn draw_node_with_matrix
    @brief Uniwersalne rekurencyjne renderowanie węzła modelu z zadaną macierzą transformacji.
    @details Wykorzystywane do rysowania wielu instancji modeli (np. trawy i kamieni) o zadanej skali i pozycji.
    @param node Aktualny węzeł sceny 3D.
    @param depth_program Program cieniowania głębi.
    @param projection_matrix Macierz rzutowania kamery.
    @param camera_matrix Macierz widoku kamery.
    @param matrix Macierz przekształcenia (skala i przesunięcie).
    @param is_depth_pass Flaga fazy generowania cieni (True) lub kolorowania (False).
    """
    # Uniwersalne rekurencyjne renderowanie węzła modelu z zadaną macierzą transformacji
    if node.mesh:
        if is_depth_pass:
            depth_program["m_model"].write(matrix)
            node.mesh.vao.render(depth_program)
        else:
            node.mesh.draw(
                projection_matrix=projection_matrix,
                camera_matrix=camera_matrix,
                model_matrix=matrix,
            )
    if hasattr(node, "children"):
        for child in node.children:
            draw_node_with_matrix(
                child,
                depth_program,
                projection_matrix,
                camera_matrix,
                matrix,
                is_depth_pass,
            )
