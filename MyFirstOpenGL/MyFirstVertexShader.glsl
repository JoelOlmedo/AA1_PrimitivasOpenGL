#version 440 core

layout(location = 0) in vec3 posicion;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(posicion, 1.0);
}