import moderngl_window.scene as mglws

print([m for m in dir(mglws.Scene) if not m.startswith("_")])
print([m for m in dir(mglws.Node) if not m.startswith("_")])
import sys

sys.exit(0)
