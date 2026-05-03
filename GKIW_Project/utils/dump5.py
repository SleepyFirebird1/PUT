from moderngl_window.loaders.scene.gltf2 import GLTF2

s = GLTF2("assets/Stone-black.glb").load()
s.calc_scene_bbox()
print("BBox min:", s.bbox.min)
print("BBox max:", s.bbox.max)
for n in s.nodes:
    print(n.name, n.matrix)
