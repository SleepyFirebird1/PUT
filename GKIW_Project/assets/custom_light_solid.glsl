#version 330

#if defined VERTEX_SHADER
in vec3 in_position;
in vec3 in_normal;
// Usunęliśmy texcoord (bo roślinność go nie ma!)

uniform mat4 m_proj;
uniform mat4 m_model;
uniform mat4 m_cam;

out vec3 normal;
out vec3 world_pos;

void main() {
    vec4 worldPos = m_model * vec4(in_position, 1.0);
    world_pos = worldPos.xyz;
    gl_Position = m_proj * m_cam * worldPos;
    
    mat3 m_normal = transpose(inverse(mat3(m_model)));
    normal = m_normal * in_normal;
}

#elif defined FRAGMENT_SHADER
uniform vec3 ambientColor;
uniform float ambientPower;

uniform vec3 tableLightPos;
uniform vec3 tableLightColor;
uniform float tableLightPower;

// Zamiast tekstury używamy jednolitego koloru z Pythona
uniform vec4 baseColor;

in vec3 normal;
in vec3 world_pos;
out vec4 fragColor;

void main() {
    vec3 norm = normalize(normal);
    
    float distance = length(tableLightPos - world_pos);
    
    // Parametry tłumienia (muszą być takie same jak w głównym shaderze)
    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032; 
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
    
    vec3 tableLightDir = normalize(tableLightPos - world_pos);
    float tableDiff = max(dot(norm, tableLightDir), 0.0);
    
    // Aplikacja tłumienia do siły światła
    vec3 tableLighting = tableDiff * tableLightColor * tableLightPower * attenuation;
    
    vec3 ambientLighting = ambientColor * ambientPower;
    
    vec3 finalColor = baseColor.rgb * (ambientLighting + tableLighting);
    fragColor = vec4(finalColor, baseColor.a);
}
#endif