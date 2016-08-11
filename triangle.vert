#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec3 inColor;

layout (location = 0) smooth out vec3 outColor;

void main(){
	outColor = inColor;
	gl_Position = vec4( inPos.xy, 0.0, 1.0 );
}
