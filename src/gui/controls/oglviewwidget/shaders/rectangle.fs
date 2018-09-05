#version 150

in vec4 ColPass;

out vec4 FragColor;

void main() {
    FragColor = ColPass;
}