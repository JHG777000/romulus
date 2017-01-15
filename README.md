romulus
=====
romulus README

romulus is a work in progress cross-platform opengl based rendering library written in C.

-romulus is designed to be simple and easy to use, to create a rendering context and window:

 romulus_display display = romulus_new_display(1077, 640, "My window for 3D rendering!!!!", 60, 0.1f, 1000.0f, 1) ; //win_x win_y win_title fov ZNear ZFar VSync(bool)

