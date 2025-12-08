#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 uProj; // optional if you want a projection
void main() {
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}
