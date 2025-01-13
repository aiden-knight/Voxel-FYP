#version 450

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 6) out;

uint width = 16;
uint height = 12;
vec2 offset = vec2(2.0/width, 2.0/height);
vec2 vertices[] = {
	vec2(0, 0), 
	vec2(offset.x, 0),
	vec2(offset.x,  offset.y),
	vec2(0,  offset.y)
};

vec3 colours[] = {
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 1.0, 1.0)
};

layout(location = 0) out vec3 fragColour;

void main() {
	gl_Position = gl_in[0].gl_Position + vec4(vertices[0], 0.0, 0.0);
	fragColour = colours[0];
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + vec4(vertices[1], 0.0, 0.0);
	fragColour = colours[1];
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + vec4(vertices[2], 0.0, 0.0);
	fragColour = colours[2];
	EmitVertex();
	EndPrimitive();

	gl_Position = gl_in[0].gl_Position + vec4(vertices[2], 0.0, 0.0);
	fragColour = colours[2];
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + vec4(vertices[3], 0.0, 0.0);
	fragColour = colours[3];
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + vec4(vertices[0], 0.0, 0.0);
	fragColour = colours[0];
	EmitVertex();
	EndPrimitive();
}