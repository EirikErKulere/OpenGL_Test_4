#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
// layout(location = 3) in int aTexIndex;

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vTexCoord;
// flat out int vTexIndex;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    vFragPos = vec3(uView * uModel * vec4(aPos, 1.0));
    vNormal = mat3(transpose(inverse(uView * uModel))) * aNormal;
    vTexCoord = aTexCoord;
    // vTexIndex = aTexIndex;
}
