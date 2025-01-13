#version 450

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

vec2 vertices[] = {
	vec2(-0.5, -0.5), 
	vec2(0.5, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
};
vec3 colours[] = {
	vec3(1.0, 0.0, 0.0)
	vec3(0.0, 1.0, 0.0)
	vec3(0.0, 0.0, 1.0)
	vec3(1.0, 1.0, 1.0)
};


const std::vector<uint16_t> g_indices = {
	0, 1, 2, 2, 3, 0
};

void main() {
	
}