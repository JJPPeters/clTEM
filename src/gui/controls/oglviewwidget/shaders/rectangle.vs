#version 150

uniform mat4 ModelView;
uniform mat4 Proj;

in vec3 PosBuf;
in vec4 ColBuf;

out vec4 ColPass;

void main() {
    ColPass = ColBuf;
    gl_Position = Proj * (ModelView * vec4(PosBuf, 1.0));
}