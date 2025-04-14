#include "util_renderer2d.h"

#define MAX_QUADS 4000
#define MAX_VERTICES (MAX_QUADS*4)
#define MAX_INDICES (MAX_QUADS*6)

#define MAX_TEXTURE_COUNT 128
#define MAX_TEXTURE_SIZE 256

#undef X
#define X(proc, name) proc name;
GLFUNCTIONS

typedef struct vertex_t {
    vec2_t position;
    vec2_t tex_coords;
    vec4_t color;
    float tex_id, use_bilinear;
} vertex_t;

typedef struct Renderer2dData {
    Shader default_shader;
    Texture white_texture;

    vertex_t *vertices, *vertices_begin;
    uint32_t current_quad_count;
    unsigned int batch_vertex_array, batch_vertex_buffer;

    uint32_t texture_3d;
    uint32_t next_texture_handle;

} Renderer2dData;

Renderer2dData *g_r2d_data;

void r2d_init(MemoryArena *arena) {
    g_r2d_data = arena_alloc(arena, 1, Renderer2dData);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_r2d_data->next_texture_handle = 1;

    str_t vertex_shader = str_lit(
        "#version 330 core\n"
        "layout (location = 0) in vec2 a_pos;\n"
        "layout (location = 1) in vec2 a_tex_coords;\n"
        "layout (location = 2) in vec4 a_color;\n"
        "layout (location = 3) in float a_tex_index;\n"
        "layout (location = 4) in float a_use_bilinear;\n"
        ""
        "out vec2 f_tex_coords;\n"
        "out vec4 f_color;\n"
        "flat out float f_tex_index;\n"
        "flat out float f_use_bilinear;\n"
        ""
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        ""
        "void main()\n"
        "{\n"
        "    f_color = a_color;\n"
        "    f_tex_coords = a_tex_coords;\n"
        "    f_tex_index = a_tex_index;\n"
        "    f_use_bilinear = a_use_bilinear;\n"
        "    gl_Position = projection * view * vec4(a_pos, 0.0, 1.0);\n"
        "}\n"
    );
    str_t fragment_shader = str_lit(
        "#version 330 core\n"
        ""
        "in vec4 f_color;\n"
        "in vec2 f_tex_coords;\n"
        "flat in float f_tex_index;\n"
        "flat in float f_use_bilinear;\n"
        "out vec4 FragColor;\n"
        ""
        "uniform sampler2DArray texture_array;\n"
        ""
        "void main()\n"
        "{\n"
        "    ivec3 texture_size = textureSize(texture_array, 0);\n"
        "    vec2 uv = f_tex_coords;\n"
        "    if (f_use_bilinear == 0) {\n"
        "        uv = (floor(f_tex_coords * (ivec2(texture_size) - 0.0001)) + 0.5) / ivec2(texture_size);\n"
        "    }\n"
        "    vec3 array_uv = vec3(uv.x, uv.y, f_tex_index);\n"
        "    FragColor = f_color * texture(texture_array, array_uv);\n"
        "}\n"
    );
    
    g_r2d_data->default_shader = shader_create(arena, vertex_shader, fragment_shader);
    unsigned char data[4] = { 0xFFFFFFFF };
    g_r2d_data->white_texture = r2d_create_texture(data, 1, 1, 0);
    
    shader_use(g_r2d_data->default_shader);
    shader_set_int(g_r2d_data->default_shader, "image", 0);
    mat4_t view = mat4_identity();
    shader_set_mat4(g_r2d_data->default_shader, "view", &view);
    shader_set_int(g_r2d_data->default_shader, "texture_array", GL_TEXTURE0);

    //Batch Data
    {
        g_r2d_data->vertices = arena_alloc(arena, MAX_VERTICES, vertex_t);
        g_r2d_data->vertices_begin = g_r2d_data->vertices;

        unsigned int ebo;

        glGenVertexArrays(1, &g_r2d_data->batch_vertex_array);
        glGenBuffers(1, &g_r2d_data->batch_vertex_buffer);
        glGenBuffers(1, &ebo);

        glBindVertexArray(g_r2d_data->batch_vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, g_r2d_data->batch_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(vertex_t), 0, GL_DYNAMIC_DRAW);
       

        StackMemoryArena stack_arena = arena_push_stack_arena(arena);
        unsigned int* indices = arena_alloc(stack_arena.arena, MAX_INDICES, unsigned int);
        
        int offset = 0;
        for (unsigned int i = 0; i < MAX_INDICES; i += 6) {
            indices[i + 0] = 0 + offset;
            indices[i + 1] = 1 + offset;
            indices[i + 2] = 2 + offset;
            indices[i + 3] = 2 + offset;
            indices[i + 4] = 3 + offset;
            indices[i + 5] = 0 + offset;

            offset += 4;
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * MAX_INDICES, indices, GL_STATIC_DRAW);
        arena_pop_stack_arena(&stack_arena);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, position));
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, tex_coords));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, color));
        
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, tex_id));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, use_bilinear));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    _r2d_create_texture_3d();
    _r2d_begin_batch();
}
void r2d_clear(vec4_t color) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(color.r, color.g, color.b, color.a);
}
void r2d_update_projection(vec2_t window_size) {
    mat4_t projection = mat4_ortho(0.0f, window_size.width, window_size.height, 0.0f, -1.0f, 1.0f);

    shader_use(g_r2d_data->default_shader);
    shader_set_mat4(g_r2d_data->default_shader, "projection", &projection);

    glViewport(0, 0, window_size.width, window_size.height);
}
void r2d_render_rect(vec2_t position, vec2_t size, vec4_t color, float rotation, vec2_t pivot) {
    r2d_render_sprite(position, size, g_r2d_data->white_texture, color, rotation, pivot);
}
void r2d_render_sprite(vec2_t position, vec2_t size, Texture texture, vec4_t color, float rotation, vec2_t pivot) {
        
    if (g_r2d_data->current_quad_count >= MAX_QUADS) {
        r2d_flush();
    }

    shader_use(g_r2d_data->default_shader);
    float texture_id = (float) texture.handle;

    pivot = (vec2_t) { position.x + pivot.x * size.x, position.y + pivot.y * size.y };
    
    vec2_t pos = position;
    vec2_t rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    g_r2d_data->vertices->position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    g_r2d_data->vertices->tex_coords = (vec2_t) { 0.0f, 0.0f };
    g_r2d_data->vertices->color = color;
    g_r2d_data->vertices->tex_id = texture_id;
    g_r2d_data->vertices->use_bilinear = texture.use_bilinear;
    g_r2d_data->vertices++;

    pos = (vec2_t) { position.x + size.x, position.y };
    rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    g_r2d_data->vertices->position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    g_r2d_data->vertices->tex_coords = (vec2_t) { (float) texture.width  / MAX_TEXTURE_SIZE, 0.0f };
    g_r2d_data->vertices->color = color;
    g_r2d_data->vertices->tex_id = texture_id;
    g_r2d_data->vertices->use_bilinear = texture.use_bilinear;
    g_r2d_data->vertices++;

    pos = (vec2_t) { position.x + size.x, position.y + size.y };
    rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    g_r2d_data->vertices->position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    g_r2d_data->vertices->tex_coords = (vec2_t) { (float) texture.width / MAX_TEXTURE_SIZE, (float)texture.height / MAX_TEXTURE_SIZE };
    g_r2d_data->vertices->color = color;
    g_r2d_data->vertices->tex_id = texture_id;
    g_r2d_data->vertices->use_bilinear = texture.use_bilinear;
    g_r2d_data->vertices++;

    pos = (vec2_t) { position.x, position.y + size.y };
    rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    g_r2d_data->vertices->position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    g_r2d_data->vertices->tex_coords = (vec2_t) { 0.0f, (float) texture.height / MAX_TEXTURE_SIZE };
    g_r2d_data->vertices->color = color;
    g_r2d_data->vertices->tex_id = texture_id;
    g_r2d_data->vertices->use_bilinear = texture.use_bilinear;
    g_r2d_data->vertices++;

    g_r2d_data->current_quad_count++;
}
void r2d_flush() {
    _r2d_end_batch();

    shader_use(g_r2d_data->default_shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, g_r2d_data->texture_3d);

    //Flush data
    glBindVertexArray(g_r2d_data->batch_vertex_array);
    glDrawElements(GL_TRIANGLES, g_r2d_data->current_quad_count * 6, GL_UNSIGNED_INT, 0);

    _r2d_begin_batch();
}
void _r2d_begin_batch() {
    g_r2d_data->current_quad_count = 0;
    g_r2d_data->vertices = g_r2d_data->vertices_begin;
}
void _r2d_end_batch() {
    GLsizeiptr size = (uint8_t *)g_r2d_data->vertices - (uint8_t *)g_r2d_data->vertices_begin;
    glBindBuffer(GL_ARRAY_BUFFER, g_r2d_data->batch_vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, g_r2d_data->vertices_begin);
}
void _r2d_create_texture_3d() {
    glGenTextures(1, &g_r2d_data->texture_3d);
    glBindTexture(GL_TEXTURE_2D_ARRAY, g_r2d_data->texture_3d);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, MAX_TEXTURE_COUNT);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned int white = 0xffffffff;

    glBindTexture(GL_TEXTURE_2D_ARRAY, g_r2d_data->texture_3d);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)&white);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
Texture r2d_create_texture_from_file(MemoryArena *arena, str_t path, uint8_t bilinear) {
    TextureData texture_data = texture_load_data(arena, path);
    return r2d_create_texture_from_data(texture_data, bilinear);
}
Texture r2d_create_texture_from_data(TextureData texture_data, uint8_t bilinear) {
    Texture texture = r2d_create_texture(texture_data.data, texture_data.width, texture_data.height, bilinear);
    texture_free(texture_data);
    return texture;
}
Texture r2d_create_texture(unsigned char *data, uint32_t width, uint32_t height, uint8_t bilinear) {
    Texture texture = {0};
    texture.handle = g_r2d_data->next_texture_handle;
    texture.use_bilinear = bilinear;
    texture.width = width;
    texture.height = height;

    glBindTexture(GL_TEXTURE_2D_ARRAY, g_r2d_data->texture_3d);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, g_r2d_data->next_texture_handle, texture.width, texture.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    g_r2d_data->next_texture_handle++;

    return texture;
}