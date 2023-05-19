#version 410 core

layout (location = 0) out vec4 fragColour;

uniform vec3 colour;

void main()
{
    fragColour.rgb = colour;
}