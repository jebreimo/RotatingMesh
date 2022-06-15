#version 410 core

in vec3 a_position;
in vec3 a_normal;

out vec3 v_vertex_color;

uniform mat4 u_mv_matrix;
uniform mat4 u_proj_matrix;
uniform vec3 u_light_vec = vec3(-1 / sqrt(3), -1 / sqrt(3), 1 / sqrt(3));

void main()
{
    vec4 pos = u_mv_matrix * vec4(a_position, 1.0);
    vec3 normal = normalize(mat3(u_mv_matrix) * a_normal);
    float c = ((dot(normal, u_light_vec) + 1) / 2);
    v_vertex_color = vec3(c, c, c);
    gl_Position = u_proj_matrix * pos;
}
