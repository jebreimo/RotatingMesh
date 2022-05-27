#version 410 core

in vec3 position;
in vec3 normal;
in vec3 color;

out vec3 vertexColor;

uniform mat4 mvpMatrix;
uniform mat4 mMatrix;

uniform vec3 light = vec3(-1 / sqrt(3), -1 / sqrt(3), 1 / sqrt(3));

void main()
{
    vec3 tmpNormal = normalize(mat3(mMatrix) * normal);
    vertexColor = color * ((dot(tmpNormal, light) + 1) / 2);
    gl_Position = mvpMatrix * vec4(position, 1.0);
}
