"""@package main
@brief Main entry point for the Go 3D application.
"""

import moderngl_window as mglw
from core.window import Window

if __name__ == "__main__":
    mglw.run_window_config(Window)
