#version 450

layout(location = 0) in vec3 position;

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) out vec3 fragColour;

void main() {
	fragColour = vec3(0.5, 0.5, 0.5);
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position, 1.0);
}