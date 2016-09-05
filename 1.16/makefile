
all : space_walk

space_walk: space_walk.c GL_utilities.c 
	gcc -std=c99 -o space_walk  -DGL_GLEXT_PROTOTYPES space_walk.c GL_utilities.c MicroGlut.c loadobj.c VectorUtils3.c -lGL -lm -lX11 -lXext  

clean :
	rm  space_walk

