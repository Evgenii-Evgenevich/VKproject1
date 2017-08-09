#ifndef __ShaderStructures_H
#define __ShaderStructures_H 1

struct ModelViewProjectionBuffer
{
	float model[16];
	float view[16];
	float projection[16];
};

struct VertexPositionColor
{
	float position[3];
	float color[3];
};

#endif // !__ShaderStructures_H
