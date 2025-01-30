#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 colour;

layout(location = 0) out vec4 FragColour;

layout (binding = 0) uniform ParameterUBO {
	mat4 view;
	mat4 proj;
	uint particleCount;
	float halfExtent;
	float velocityMult;
	float deltaTime;
} ubo;

void main() {
	FragColour = colour;
	gl_Position = position;
}