#version 330 core

out vec4 FragColor;

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;
// flat in int vTexIndex;

uniform bool uDoSunlight;

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

// #define NR_POINT_LIGHTS 2
// uniform PointLight PointLights[NR_POINT_LIGHTS];
uniform DirLight uSunlight;
uniform Material uMaterial;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main() {
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(-vFragPos);
    vec3 result = vec3(0.0);

    result += calcDirLight(uSunlight, norm, viewDir);
    // for (int i = 0; i < NR_POINT_LIGHTS; i++) {
    //     result += calcPointLight(PointLights[i], norm, FragPos, viewDir);
    // }

    FragColor = vec4(result, 1.0);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess);

    vec3 ambient = light.ambient * vec3(texture(uMaterial.diffuse, vTexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(uMaterial.diffuse, vTexCoord));
    vec3 specular = light.specular * spec * vec3(texture(uMaterial.specular, vTexCoord));

    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess);

    vec3 ambient = light.ambient * vec3(texture(uMaterial.diffuse, vTexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(uMaterial.diffuse, vTexCoord));
    vec3 specular = light.specular * spec * vec3(texture(uMaterial.specular, vTexCoord));
    return (ambient + diffuse + specular);
}
