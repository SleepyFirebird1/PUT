#version 330

#if defined VERTEX_SHADER
in vec3 in_position;

uniform mat4 m_model;
uniform mat4 m_light_space;

void main() {
    gl_Position = m_light_space * m_model * vec4(in_position, 1.0);
}

#elif defined FRAGMENT_SHADER

void main() {
    // Empty fragment shader - OpenGL will automatically record depth
}
#endif
