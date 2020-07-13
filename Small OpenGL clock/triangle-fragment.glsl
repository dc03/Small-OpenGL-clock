#version 330 core

uniform vec3 triangle_color;

out vec4 frag_result;

void main()
{
    frag_result = vec4(triangle_color, 1.0f);
}