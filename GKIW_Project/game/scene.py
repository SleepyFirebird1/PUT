import random


class Interval:
    def __init__(self, left, right):
        self.left = left
        self.right = right

    def overlaps(self, other):
        return self.left <= other.right and other.left <= self.right


class Scene:
    def __init__(self):
        self.objects = []

    def validation(self, x, z, object_size):
        x_object_size, z_object_size = object_size
        for object in self.objects:
            x_compare, z_compare = object
            # Obiekt porównawczy
            x_compare_box = Interval(
                x_compare - x_object_size / 2, x_compare + x_object_size / 2
            )
            z_compare_box = Interval(
                z_compare - z_object_size / 2, z_compare + z_object_size / 2
            )
            # Obiekt wstawiany
            x_box = Interval(x - x_object_size / 2, x + x_object_size / 2)
            z_box = Interval(z - z_object_size / 2, z + z_object_size / 2)
            if x_box.overlaps(x_compare_box) and z_box.overlaps(z_compare_box):
                return False
        return True

    # 0,4x0,4 trawka 5x5 pole, 0,9x0,6 board
    def generation(self, object_size, scene_size, board_size, counter):
        x_object_size, z_object_size = object_size
        x_scene_size, z_scene_size = scene_size
        x_board_size, z_board_size = board_size
        # Losowanie
        # Przedziały dozwolone
        x_1 = (
            x_board_size / 2 + x_object_size / 2,
            x_scene_size / 2 - x_object_size / 2,
        )
        x_2 = (
            -x_board_size / 2 - x_object_size / 2,
            -x_scene_size / 2 + x_object_size / 2,
        )
        z_1 = (
            z_board_size / 2 + z_object_size / 2,
            z_scene_size / 2 - z_object_size / 2,
        )
        z_2 = (
            -z_board_size / 2 - z_object_size / 2,
            -z_scene_size / 2 + z_object_size / 2,
        )

        while counter > 0:
            decision = random.randint(0, 1)
            if decision == 0:
                x_beg, x_end = x_1
            else:
                x_beg, x_end = x_2
            x = round(random.uniform(x_beg, x_end), 4)
            decision = random.randint(0, 1)
            if decision == 0:
                z_beg, z_end = z_1
            else:
                z_beg, z_end = z_2
            z = round(random.uniform(z_beg, z_end), 4)
            if self.validation(x, z, object_size):
                self.objects.append((x, z))
                counter -= 1
        return self.objects
