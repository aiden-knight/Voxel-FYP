#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 colour;

layout(location = 0) out vec4 FragColour;

layout (binding = 0) uniform ParameterUBO {
	mat4 view;
	mat4 proj;
	float deltaTime;
} ubo;

void main() {
	FragColour = colour;
	gl_Position = vec4(position, 0.0, 1.0);
}