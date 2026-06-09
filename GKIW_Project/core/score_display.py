import moderngl_window as mglw
import moderngl_window.geometry as geom
import glm

DIGITS = {
    0: [
        (0, 4),
        (1, 4),
        (2, 4),
        (0, 3),
        (2, 3),
        (0, 2),
        (2, 2),
        (0, 1),
        (2, 1),
        (0, 0),
        (1, 0),
        (2, 0),
    ],
    1: [(1, 4), (0, 3), (1, 3), (1, 2), (1, 1), (0, 0), (1, 0), (2, 0)],
    2: [
        (0, 4),
        (1, 4),
        (2, 4),
        (2, 3),
        (0, 2),
        (1, 2),
        (2, 2),
        (0, 1),
        (0, 0),
        (1, 0),
        (2, 0),
    ],
    3: [
        (0, 4),
        (1, 4),
        (2, 4),
        (2, 3),
        (0, 2),
        (1, 2),
        (2, 2),
        (2, 1),
        (0, 0),
        (1, 0),
        (2, 0),
    ],
    4: [(0, 4), (2, 4), (0, 3), (2, 3), (0, 2), (1, 2), (2, 2), (2, 1), (2, 0)],
    5: [
        (0, 4),
        (1, 4),
        (2, 4),
        (0, 3),
        (0, 2),
        (1, 2),
        (2, 2),
        (2, 1),
        (0, 0),
        (1, 0),
        (2, 0),
    ],
    6: [
        (0, 4),
        (1, 4),
        (2, 4),
        (0, 3),
        (0, 2),
        (1, 2),
        (2, 2),
        (0, 1),
        (2, 1),
        (0, 0),
        (1, 0),
        (2, 0),
    ],
    7: [(0, 4), (1, 4), (2, 4), (2, 3), (2, 2), (1, 1), (1, 0)],
    8: [
        (0, 4),
        (1, 4),
        (2, 4),
        (0, 3),
        (2, 3),
        (0, 2),
        (1, 2),
        (2, 2),
        (0, 1),
        (2, 1),
        (0, 0),
        (1, 0),
        (2, 0),
    ],
    9: [
        (0, 4),
        (1, 4),
        (2, 4),
        (0, 3),
        (2, 3),
        (0, 2),
        (1, 2),
        (2, 2),
        (2, 1),
        (0, 0),
        (1, 0),
        (2, 0),
    ],
}


class ScoreDisplay:
    def __init__(self, ctx, program, depth_program=None):
        self.ctx = ctx
        self.program = program
        self.depth_program = depth_program
        self.cube_size = 0.02
        self.spacing = 0.025
        self.cube = geom.cube(size=(self.cube_size, self.cube_size, self.cube_size))

    def draw_digit(
        self, digit, base_matrix, color, projection, view, is_depth_pass=False
    ):
        if digit not in DIGITS:
            return
        prog = self.depth_program if is_depth_pass else self.program
        if not is_depth_pass:
            prog["baseColor"].value = color

        for x, y in DIGITS[digit]:
            offset_x = x * self.spacing
            offset_y = y * self.spacing

            model_matrix = glm.translate(base_matrix, glm.vec3(offset_x, offset_y, 0))
            prog["m_model"].write(model_matrix)
            if not is_depth_pass:
                prog["m_proj"].write(projection)
                prog["m_cam"].write(view)

            self.cube.render(prog)

    def draw_number(
        self, number, position, rotation_y, color, projection, view, is_depth_pass=False
    ):
        digits = []
        for d in str(number):
            digits.append(int(d))

        base_matrix = glm.mat4(1.0)
        base_matrix = glm.translate(base_matrix, position)
        if rotation_y != 0:
            base_matrix = glm.rotate(base_matrix, rotation_y, glm.vec3(0, 1, 0))
        digit_spacing = self.spacing * 4

        for i, digit in enumerate(digits):
            digit_matrix = glm.translate(base_matrix, glm.vec3(i * digit_spacing, 0, 0))
            self.draw_digit(digit, digit_matrix, color, projection, view, is_depth_pass)
