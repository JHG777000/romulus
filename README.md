romulus
=====
romulus README

romulus is a work in progress cross-platform opengl based rendering library written in C.

See main.c for example.

Uses:

 -STB Image https://github.com/nothings/stb/blob/master/stb_image.h 

 -RKLib https://github.com/JHG777000/RKLib
 
 -Nuklear https://github.com/vurtun/nuklear 

 -IDK https://github.com/JHG777000/IDK

 -GLFW http://www.glfw.org
 
## Building

romulus uses [builder][1] for its build system.

[1]:https://github.com/JHG777000/builder

To download and build use this command:


	builder -u https://raw.githubusercontent.com/JHG777000/romulus/master/buildfile
	
To run test, add -i __t:

	builder -i __t -u https://raw.githubusercontent.com/JHG777000/romulus/master/buildfile