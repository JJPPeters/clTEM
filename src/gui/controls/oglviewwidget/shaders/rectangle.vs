#version 150

uniform mat4 ModelView;
uniform mat4 Proj;

in vec3 PosBuf;

out vec4 PosPass;

void main() {
    gl_Position = Proj * (ModelView * vec4(PosBuf, 1.0));
    PosPass = vec4(PosBuf, 1.0);
}