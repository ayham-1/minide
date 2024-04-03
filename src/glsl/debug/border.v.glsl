#version 330 core

attribute vec2 coord;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(coord, 0.0, 1.0);
} 
