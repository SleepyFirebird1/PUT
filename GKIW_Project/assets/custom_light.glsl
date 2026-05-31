#version 330

#if defined VERTEX_SHADER
in vec3 in_position;
in vec3 in_normal;
in vec2 in_texcoord_0;

uniform mat4 m_proj;
uniform mat4 m_model;
uniform mat4 m_cam;

out vec3 normal;
out vec3 world_pos;
out vec2 uv;

void main() {
    // 1. Obliczanie pozycji globalnej (World Space), niezależnej od kamery!
    vec4 worldPos = m_model * vec4(in_position, 1.0);
    world_pos = worldPos.xyz;
    
    // 2. Widok m_cam uzyty TYLKO do ustalenia na co patrzymy (gl_Position)
    gl_Position = m_proj * m_cam * worldPos;
    
    // 3. Normalne rowniez tylko względem świata (aby swiatlo nie zalezalo od obrotu kamery)
    mat3 m_normal = transpose(inverse(mat3(m_model)));
    normal = m_normal * in_normal;
    uv = in_texcoord_0;
}

#elif defined FRAGMENT_SHADER
uniform sampler2D texture0;
uniform sampler2D shadowMap;
uniform mat4 m_light_space;

uniform vec3 ambientColor;
uniform float ambientPower;

uniform vec3 tableLightPos;
uniform vec3 tableLightColor;
uniform float tableLightPower;

in vec3 normal;
in vec3 world_pos;
in vec2 uv;
out vec4 fragColor;

void main() {
    vec4 texColor = texture(texture0, uv);
    vec3 norm = normalize(normal);
    
    // Obliczanie dystansu od lampy do danego punktu na świecie
    float distance = length(tableLightPos - world_pos);
    
    // Parametry tłumienia (Attenuation) - tu sterujesz zasięgiem światła
    // Jeśli zwiększysz kwadratowy współczynnik (0.05), promień światła będzie mniejszy
    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032; 
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
    
    // Kierunek światła
    vec3 tableLightDir = normalize(tableLightPos - world_pos);
    float tableDiff = max(dot(norm, tableLightDir), 0.0);
    
    // Wygaszamy światło względem dystansu!
    vec3 tableLighting = tableDiff * tableLightColor * tableLightPower * attenuation;
    
    // --- Obliczanie cienia ---
    vec4 fragPosLightSpace = m_light_space * vec4(world_pos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    float shadow = 0.0;
    if (projCoords.z <= 1.0) {
        float currentDepth = projCoords.z;
        float bias = max(0.005 * (1.0 - dot(norm, tableLightDir)), 0.001);
        
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;
    }
    
    tableLighting *= (1.0 - shadow);
    // -------------------------
    
    vec3 ambientLighting = ambientColor * ambientPower;
    
    vec3 finalColor = texColor.rgb * (ambientLighting + tableLighting);
    fragColor = vec4(finalColor, texColor.a);
}
#endif