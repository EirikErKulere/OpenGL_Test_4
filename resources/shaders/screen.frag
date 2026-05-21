#version 330 core

out vec4 FragColor;

in vec2 vTexCoords;

uniform sampler2D uTexture;

void main() {
    // vec3 color = texture(uTexture, vTexCoords).rgb;
    // FragColor = vec4(vec3((0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b)), 1.0);
    FragColor = texture(uTexture, vTexCoords);
}
