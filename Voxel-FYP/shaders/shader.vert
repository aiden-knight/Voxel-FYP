#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 colour;

layout(location = 0) out vec4 FragColour;

void main() {
	FragColour = colour;
	gl_Position = position;
}