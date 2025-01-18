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
	(ubo.view * ubo.proj * vec4(-extent.x/2, -extent.y/2, 0, 0)).xy, 
	(ubo.view * ubo.proj * vec4( extent.x/2, -extent.y/2, 0, 0)).xy,
	(ubo.view * ubo.proj * vec4( extent.x/2,  extent.y/2, 0, 0)).xy,
	(ubo.view * ubo.proj * vec4(-extent.x/2,  extent.y/2, 0, 0)).xy
};


void main() {
	FragColourOut = FragColourIn[0];

	gl_Position = (gl_in[0].gl_Position + vec4(vertices[0],0.0, 0.0));
	EmitVertex();
	gl_Position = (gl_in[0].gl_Position + vec4(vertices[1],0.0, 0.0));
	EmitVertex();
	gl_Position = (gl_in[0].gl_Position + vec4(vertices[2],0.0, 0.0));
	EmitVertex();
	EndPrimitive();

	gl_Position = (gl_in[0].gl_Position + vec4(vertices[3],0.0, 0.0));
	EmitVertex();
	gl_Position = (gl_in[0].gl_Position + vec4(vertices[0],0.0, 0.0));
	EmitVertex();
	gl_Position = (gl_in[0].gl_Position + vec4(vertices[2],0.0, 0.0));
	EmitVertex();
	EndPrimitive();
}