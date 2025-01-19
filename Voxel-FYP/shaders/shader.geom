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

vec2 diff = vec2(0.25, 0.33) * 2;
uint particleCount = 4096;

vec2 extent = vec2(0.05, 0.05);
vec2 vertices[] = {
	vec2(-extent.x/2, -extent.y/2), 
	vec2( extent.x/2, -extent.y/2),
	vec2( extent.x/2,  extent.y/2),
	vec2(-extent.x/2,  extent.y/2)
};

void EmitVertexWithOffset(uint index);

void main() {
	EmitVertexWithOffset(0);
	EmitVertexWithOffset(1);
	EmitVertexWithOffset(2);
	EndPrimitive();
	EmitVertexWithOffset(3);
	EmitVertexWithOffset(0);
	EmitVertexWithOffset(2);
	EndPrimitive();
}

void EmitVertexWithOffset(uint index)
{
	gl_Position = ubo.view * ubo.proj * (gl_in[0].gl_Position + vec4(vertices[index],0.0, 0.0));
	FragColourOut = FragColourIn[0]; // must set output again for every vertex emitted
	EmitVertex();
}