#version 450

layout(location = 0) in uint inPosition;

layout(location = 0) out vec3 fragColour;

vec2 offset = vec2(1.0/16.0, 1.0/12.0);

void main() {
	uint yVal = inPosition % 12;
	uint xVal = inPosition - (12 * yVal);

	gl_Position = vec4(xVal * offset.x, yVal * offset.y, 0.0, 1.0);
	fragColour = inColour;
}