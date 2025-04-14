#include "util_shader.h"

void shader_use(Shader shader) {
    glUseProgram(shader.id);
}
void shader_set_v4(Shader shader, char *name, vec4_t v) {
    unsigned int loc = glGetUniformLocation(shader.id, name);
    glUniform4f(loc, v.x, v.y, v.z, v.w);
}
void shader_set_v3(Shader shader, char *name, vec3_t v) {
    unsigned int loc = glGetUniformLocation(shader.id, name);
    glUniform3f(loc, v.x, v.y, v.z);
}
void shader_set_v2(Shader shader, char *name, vec2_t v) {
    unsigned int loc = glGetUniformLocation(shader.id, name);
    glUniform2f(loc, v.x, v.y);
}
void shader_set_mat4(Shader shader, char *name, mat4_t *v) {
    unsigned int loc = glGetUniformLocation(shader.id, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat*)v->m);
}
void shader_set_float(Shader shader, char *name, float v) {
    unsigned int loc = glGetUniformLocation(shader.id, name);
    glUniform1f(loc, v);
}
void shader_set_int(Shader shader, char *name, int v) {
    unsigned int loc = glGetUniformLocation(shader.id, name);
    glUniform1i(loc, v);
}
Shader shader_create(MemoryArena *arena, str_t vertex_shader_string, str_t fragment_shader_string) {
    char *vertex_shader_string_c = str_to_cstr(arena, vertex_shader_string);
    char *fragment_shader_string_c = str_to_cstr(arena, fragment_shader_string);

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertex_shader_string_c, NULL);
    glCompileShader(vertexShader);

    {
        int  success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
            printf("Shader: %s\n\n\n\n", vertex_shader_string_c);
        }
    }
    //----FRAGMENT SHADER----
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader_string_c, NULL);
    glCompileShader(fragmentShader);

    {
        int success;
        char infoLog[512];
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
            printf("Shader: %s\n", fragment_shader_string_c);
        }
    }

    //----LINK SHADERS----
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    {
        int success;
        char infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            printf("ERROR::SHADER::LINKING::COMPILATION_FAILED\n%s\n", infoLog);
        }
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return (Shader) { shaderProgram };
}