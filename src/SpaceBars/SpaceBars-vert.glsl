#version 410 core

in vec3 a_position;
in vec3 a_normal;
in vec3 a_color;

out vec3 v_vertex_color;

uniform mat4 u_mvp_matrix;
uniform mat4 u_matrix;
uniform vec3 u_light = vec3(-1 / sqrt(3), -1 / sqrt(3), 1 / sqrt(3));

void main()
{
    vec3 normal = normalize(mat3(u_matrix) * a_normal);
    v_vertex_color = a_color * ((dot(normal, u_light) + 1) / 2);
    gl_Position = u_mvp_matrix * vec4(a_position, 1.0);
}
