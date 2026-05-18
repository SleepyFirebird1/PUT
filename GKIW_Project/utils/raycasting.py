import glm


def calculate_click_raycast(
    x,
    y,
    window_size,
    projection,
    camera_pos,
    camera_target,
    up_vector,
    TABLE_HEIGTH,
    CELL_SPACING,
):
    """
    @brief Calculates the grid coordinates based on a mouse click on the screen.

    @param x Mouse X coordinate.
    @param y Mouse Y coordinate.
    @param window_size Tuple representing window width and height.
    @param projection Projection matrix.
    @param camera_pos Camera position vector.
    @param camera_target Camera target vector.
    @param up_vector Camera up vector.
    @param TABLE_HEIGTH Height of the table.
    @param CELL_SPACING Spacing between grid cells.

    @return Tuple (grid_x, grid_y) if a valid cell was clicked, None otherwise.
    """

    # Okno -> NDC (-1 do 1)
    view = glm.lookAt(camera_pos, camera_target, up_vector)
    viewport = glm.vec4(0.0, 0.0, float(window_size[0]), float(window_size[1]))

    win_y = float(window_size[1]) - float(y)

    try:
        near_point = glm.unProject(glm.vec3(x, win_y, 0.0), view, projection, viewport)
        far_point = glm.unProject(glm.vec3(x, win_y, 1.0), view, projection, viewport)
    except Exception:
        return None

    ray_origin = near_point
    ray_direction = glm.normalize(far_point - near_point)

    if ray_direction.y == 0.0:
        return

    # Obliczenie odległości na promieniu
    t = (TABLE_HEIGTH - ray_origin.y) / ray_direction.y

    if t < 0:
        return

    # Obliczamy wektor 3d uderzenia
    hit_point = ray_origin + ray_direction * t

    # Zamiana fizycznych offsetów na kordynaty planszy
    grid_x = round((hit_point.x / CELL_SPACING) + 9)
    grid_y = round((hit_point.z / CELL_SPACING) + 9)

    if 0 <= grid_x <= 18 and 0 <= grid_y <= 18:
        return grid_x, grid_y
