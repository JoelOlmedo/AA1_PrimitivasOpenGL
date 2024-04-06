#version 440 core

layout(location = 0) in vec2 posicion;

void main() {
    gl_Position = vec4(posicion, 0.0, 1.0);
}