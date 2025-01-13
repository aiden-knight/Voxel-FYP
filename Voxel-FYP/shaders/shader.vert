#version 450

layout(location = 0) in uint position;

uint width = 16;
uint height = 12;
vec2 offset = vec2(2.0 / width, 2.0 / height);

void main() {
	uint xVal = position % width;
	uint yVal = uint(position / width);

	gl_Position = vec4(
		-1.0 + (xVal * offset.x),
		-1.0 + (yVal * offset.y),
		0.0, 1.0);
}