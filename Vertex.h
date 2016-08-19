// Basic vertex data definitions

#ifndef COMMON_VERTEX_H
#define COMMON_VERTEX_H

struct Vertex2D{
	float position[2];
};

struct ColorF{
	float color[3];
};

struct Vertex2D_ColorF_pack{
	Vertex2D position;
	ColorF color;
};

#endif //COMMON_VERTEX_H
