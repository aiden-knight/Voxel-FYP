#version 450

layout (points) in;
layout (triangle_strip, max_vertices = 36) out;

layout (location = 0) in vec4 FragColourIn[];

layout(location = 0) out vec4 FragColourOut;

layout (binding = 0) uniform ParameterUBO {
	mat4 view;
	mat4 proj;
	float halfExtent;
} ubo;

vec3 vertices[] = {
	vec3(-ubo.halfExtent, -ubo.halfExtent, -ubo.halfExtent), // 0 {- - -}
	vec3(-ubo.halfExtent,  ubo.halfExtent, -ubo.halfExtent), // 1 {- + -}
	vec3(-ubo.halfExtent, -ubo.halfExtent,  ubo.halfExtent), // 2 {- - +}
	vec3(-ubo.halfExtent,  ubo.halfExtent,  ubo.halfExtent), // 3 {- + +}
	vec3( ubo.halfExtent, -ubo.halfExtent, -ubo.halfExtent), // 4 {+ - -}
	vec3( ubo.halfExtent,  ubo.halfExtent, -ubo.halfExtent), // 5 {+ + -}
	vec3( ubo.halfExtent, -ubo.halfExtent,  ubo.halfExtent), // 6 {+ - +}
	vec3( ubo.halfExtent,  ubo.halfExtent,  ubo.halfExtent)  // 7 {+ + +}
};

void EmitVertexWithOffset(uint index, vec3 colourMult);
void EmitPrimitive(uint a, uint b, uint c, vec3 colourMult);

void main() {
	// TOP
	EmitPrimitive(5, 1, 3, vec3(1, 1, 1));
	EmitPrimitive(7, 5, 3, vec3(1, 1, 1));

	// BOTTOM
	EmitPrimitive(6, 2, 0, vec3(1, 0, 0));
	EmitPrimitive(4, 6, 0, vec3(1, 0, 0));

	// FRONT
	EmitPrimitive(5, 4, 0, vec3(1, 1, 0));
	EmitPrimitive(1, 5, 0, vec3(1, 1, 0));

	// BACK
	EmitPrimitive(2, 6, 3, vec3(0, 1, 1));
	EmitPrimitive(6, 7, 3, vec3(0, 1, 1));

	// RIGHT
	EmitPrimitive(3, 1, 0, vec3(1, 0, 1));
	EmitPrimitive(2, 3, 0, vec3(1, 0, 1));

	// LEFT
	EmitPrimitive(5, 7, 6, vec3(0, 0, 1));
	EmitPrimitive(4, 5, 6, vec3(0, 0, 1));
}

void EmitVertexWithOffset(uint index, vec3 colourMult)
{
	gl_Position = ubo.view * ubo.proj * (vec4(gl_in[0].gl_Position.xyz,1.0) + vec4(vertices[index], 0.0));
	FragColourOut = FragColourIn[0]; // must set output again for every vertex emitted
	EmitVertex();
}

void EmitPrimitive(uint a, uint b, uint c, vec3 colourMult)
{
	EmitVertexWithOffset(a, colourMult);
	EmitVertexWithOffset(b, colourMult);
	EmitVertexWithOffset(c, colourMult);
	EndPrimitive();
}