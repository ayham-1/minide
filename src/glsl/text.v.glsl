#version 330 core

attribute vec4 coord;
varying vec2 texpos;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(coord.xy, 0.0, 1.0);
    texpos = coord.zw;
} 
