#version 450

layout(location = 0) in uint position;

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

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

	gl_Position = ubo.proj * ubo.view * ubo.model * gl_Position;
}