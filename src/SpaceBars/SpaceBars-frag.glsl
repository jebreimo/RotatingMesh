#version 410 core
in vec3 v_vertex_color;
out vec4 outColor;

void main()
{
    outColor = vec4(v_vertex_color, 1.0);
}
