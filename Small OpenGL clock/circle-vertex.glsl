#version 330 core

layout (location = 0) in vec3 vert_pos;

uniform mat4 model;

out vec3 frag_pos;

void main()
{
    gl_Position = model * vec4(vert_pos, 1.0f);
    frag_pos = vert_pos;
}