#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;
uniform vec3 lightDir;      // kierunek œwiat³a (np. vec3(-1, -1, -1))
uniform vec3 lightColor;    // kolor œwiat³a (np. vec3(1, 1, 1))
uniform vec3 viewPos;       // pozycja kamery

void main() {
    // Wektory i normalizacja
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);

    // Sk³adowa ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Sk³adowa diffuse
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;

    // Sk³adowa specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDirNorm, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * texture(texture1, TexCoord).rgb;
    FragColor = vec4(result, 1.0);
}
