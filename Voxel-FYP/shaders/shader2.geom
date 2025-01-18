#version 450

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

layout (location = 0) in vec4 FragColourIn[];

layout(location = 0) out vec4 FragColourOut;

layout (binding = 0) uniform ParameterUBO {
	mat4 view;
	mat4 proj;
	float deltaTime;
} ubo;

vec2 extent = vec2(0.5, 0.5);
vec2 vertices[] = {
	vec2(-extent.x/2, -extent.y/2), 
	vec2( extent.x/2, -extent.y/2),
	vec2( extent.x/2,  extent.y/2),
	vec2(-extent.x/2,  extent.y/2)
};


void main() {
	FragColourOut = FragColourIn[0];

	vec2 position = gl_in[0].gl_Position.xy; // component wise multiply 

	gl_Position = ubo.proj * ubo.view * vec4(position + vertices[0], 0.0, 1.0);
	EmitVertex();
	gl_Position = ubo.proj * ubo.view * vec4(position + vertices[2], 0.0, 1.0);
	EmitVertex();
	gl_Position = ubo.proj * ubo.view * vec4(position + vertices[1], 0.0, 1.0);
	EmitVertex();
	EndPrimitive();

	gl_Position = ubo.proj * ubo.view * vec4(position + vertices[2], 0.0, 1.0);
	EmitVertex();
	gl_Position = ubo.proj * ubo.view * vec4(position + vertices[0], 0.0, 1.0);
	EmitVertex();
	gl_Position = ubo.proj * ubo.view * vec4(position + vertices[3], 0.0, 1.0);
	EmitVertex();
	EndPrimitive();
}