#include "util_renderer2d.h"

#define MAX_QUADS 4000
#define MAX_VERTICES (MAX_QUADS*4)
#define MAX_INDICES (MAX_QUADS*6)

#define MAX_LINES 2000

#define MAX_TEXTURE_COUNT 128
#define MAX_TEXTURE_SIZE 256

#undef X
#define X(proc, name) proc name;
GLFUNCTIONS

typedef struct vertex {
    vec2_t position;
    vec2_t tex_coords;
    vec4_t color;
    vec2_t rect_pos;
    vec2_t rect_size;
    float rounded_radius;
    float tex_id, use_bilinear;
} vertex_t;

typedef struct line_vertex {
    vec2_t position;
    vec4_t color;
} line_vertex_t;

typedef struct Renderer2dData {
    Shader default_shader, line_shader;
    Texture white_texture;

    vertex_t *vertices, *vertices_begin;
    uint32_t current_quad_count;
    uint32_t batch_vertex_array, batch_vertex_buffer;

    line_vertex_t *line_vertices, *line_vertices_begin;
    uint32_t current_line_count;
    uint32_t line_batch_vertex_array, line_batch_vertex_buffer;
    
    uint32_t texture_3d;
    uint32_t next_texture_handle;

} Renderer2dData;

Renderer2dData *g_r2d_data;

void r2d_init(MemoryArena *arena) {
    g_r2d_data = arena_alloc(arena, 1, Renderer2dData);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_r2d_data->next_texture_handle = 1;
    
    unsigned char data[4] = { 255, 255, 255, 255 };
    g_r2d_data->white_texture = r2d_create_texture(data, 1, 1, 0);
    
    g_r2d_data->default_shader = shader_create(g_vertex_shader, g_fragment_shader);
    shader_use(g_r2d_data->default_shader);
    shader_set_int(g_r2d_data->default_shader, "image", 0);
    mat4_t view = mat4_identity();
    shader_set_mat4(g_r2d_data->default_shader, "view", &view);
    shader_set_int(g_r2d_data->default_shader, "texture_array", GL_TEXTURE0);
    
    g_r2d_data->line_shader = shader_create(g_vertex_shader_line, g_fragment_shader_line);
    shader_use(g_r2d_data->line_shader);
    shader_set_mat4(g_r2d_data->line_shader, "view", &view);

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
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, rect_pos));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, rect_size));
        
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, rounded_radius));

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, tex_id));

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, use_bilinear));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    {
        g_r2d_data->line_vertices = arena_alloc(arena, MAX_LINES * 2, line_vertex_t);
        g_r2d_data->line_vertices_begin = g_r2d_data->line_vertices;

        glGenVertexArrays(1, &g_r2d_data->line_batch_vertex_array);
        glBindVertexArray(g_r2d_data->line_batch_vertex_array);

        glGenBuffers(1, &g_r2d_data->line_batch_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, g_r2d_data->line_batch_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, MAX_LINES * 2 * sizeof(line_vertex_t), 0, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(line_vertex_t), (void*)offsetof(line_vertex_t, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(line_vertex_t), (void*)offsetof(line_vertex_t, color));

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
    shader_set_v2(g_r2d_data->default_shader, "screen_resolution", window_size);

    shader_use(g_r2d_data->line_shader);
    shader_set_mat4(g_r2d_data->line_shader, "projection", &projection);

    glViewport(0, 0, window_size.width, window_size.height);
}
void r2d_render_rect(vec2_t position, vec2_t size, vec4_t color, float rotation, vec2_t pivot) {
    r2d_render_sprite(position, size, g_r2d_data->white_texture, color, rotation, pivot);
}
void r2d_render_sprite(vec2_t position, vec2_t size, Texture texture, vec4_t color, float rotation, vec2_t pivot) {
    r2d_render_sprite_rounded(position, size, texture, color, rotation, pivot, 0);
}
void r2d_render_rect_rounded(vec2_t position, vec2_t size, vec4_t color, float rotation, vec2_t pivot, float rounded_radius) {
    r2d_render_sprite_rounded(position, size, g_r2d_data->white_texture, color, rotation, pivot, rounded_radius);
}
void r2d_render_sprite_rounded(vec2_t position, vec2_t size, Texture texture, vec4_t color, float rotation, vec2_t pivot, float rounded_radius) {
        
    if (g_r2d_data->current_quad_count >= MAX_QUADS) {
        r2d_flush();
    }

    float texture_id = (float) texture.handle;

    pivot = (vec2_t) { position.x + pivot.x * size.x, position.y + pivot.y * size.y };
    
    vec2_t lower_left = (vec2_t) { position.x, position.y + size.y };

    vec2_t pos = position;
    vec2_t rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    g_r2d_data->vertices->position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    g_r2d_data->vertices->tex_coords = (vec2_t) { 0.0f, 0.0f };
    g_r2d_data->vertices->color = color;
    g_r2d_data->vertices->rounded_radius = rounded_radius;
    g_r2d_data->vertices->rect_pos = lower_left;
    g_r2d_data->vertices->rect_size = size;
    g_r2d_data->vertices->tex_id = texture_id;
    g_r2d_data->vertices->use_bilinear = texture.use_bilinear;
    g_r2d_data->vertices++;

    pos = (vec2_t) { position.x + size.x, position.y };
    rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    g_r2d_data->vertices->position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    g_r2d_data->vertices->tex_coords = (vec2_t) { (float) texture.width  / MAX_TEXTURE_SIZE, 0.0f };
    g_r2d_data->vertices->color = color;
    g_r2d_data->vertices->rounded_radius = rounded_radius;
    g_r2d_data->vertices->rect_pos = lower_left;
    g_r2d_data->vertices->rect_size = size;
    g_r2d_data->vertices->tex_id = texture_id;
    g_r2d_data->vertices->use_bilinear = texture.use_bilinear;
    g_r2d_data->vertices++;

    pos = (vec2_t) { position.x + size.x, position.y + size.y };
    rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    g_r2d_data->vertices->position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    g_r2d_data->vertices->tex_coords = (vec2_t) { (float) texture.width / MAX_TEXTURE_SIZE, (float)texture.height / MAX_TEXTURE_SIZE };
    g_r2d_data->vertices->color = color;
    g_r2d_data->vertices->rounded_radius = rounded_radius;
    g_r2d_data->vertices->rect_pos = lower_left;
    g_r2d_data->vertices->rect_size = size;
    g_r2d_data->vertices->tex_id = texture_id;
    g_r2d_data->vertices->use_bilinear = texture.use_bilinear;
    g_r2d_data->vertices++;

    pos = (vec2_t) { position.x, position.y + size.y };
    rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    g_r2d_data->vertices->position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    g_r2d_data->vertices->tex_coords = (vec2_t) { 0.0f, (float) texture.height / MAX_TEXTURE_SIZE };
    g_r2d_data->vertices->color = color;
    g_r2d_data->vertices->rounded_radius = rounded_radius;
    g_r2d_data->vertices->rect_pos = lower_left;
    g_r2d_data->vertices->rect_size = size;
    g_r2d_data->vertices->tex_id = texture_id;
    g_r2d_data->vertices->use_bilinear = texture.use_bilinear;
    g_r2d_data->vertices++;

    g_r2d_data->current_quad_count++;
}

void r2d_render_thick_line(vec2_t start, vec2_t end, float thickness, vec4_t color) {
    if (g_r2d_data->current_quad_count >= MAX_QUADS) {
        r2d_flush();
    }

    vec2_t direction = vec2_normalize(vec2_sub(end, start));
    vec2_t normal = { -direction.y, direction.x };

    vec2_t offset = vec2_mul(normal, thickness * 0.5f);

    vec2_t p0 = vec2_add(start, offset);
    vec2_t p1 = vec2_sub(start, offset);
    vec2_t p2 = vec2_sub(end, offset);
    vec2_t p3 = vec2_add(end, offset);

    float tex_id = (float)g_r2d_data->white_texture.handle;
    float use_bilinear = g_r2d_data->white_texture.use_bilinear;

    vertex_t *v = g_r2d_data->vertices;

    v[0] = (vertex_t){ p0, {0,0}, color, {0,0}, {0,0}, 0, tex_id, use_bilinear };
    v[1] = (vertex_t){ p1, {0,0}, color, {0,0}, {0,0}, 0, tex_id, use_bilinear };
    v[2] = (vertex_t){ p2, {0,0}, color, {0,0}, {0,0}, 0, tex_id, use_bilinear };
    v[3] = (vertex_t){ p3, {0,0}, color, {0,0}, {0,0}, 0, tex_id, use_bilinear };

    g_r2d_data->vertices += 4;
    g_r2d_data->current_quad_count++;
}

void r2d_render_line(vec2_t start, vec2_t end, vec4_t color) {
    if (g_r2d_data->current_line_count >= MAX_LINES) {
        r2d_flush_lines();
    }

    g_r2d_data->line_vertices->position = start;
    g_r2d_data->line_vertices->color = color;
    g_r2d_data->line_vertices++;

    g_r2d_data->line_vertices->position = end;
    g_r2d_data->line_vertices->color = color;
    g_r2d_data->line_vertices++;

    g_r2d_data->current_line_count++;
}

void r2d_flush_lines() {
    
    //End batch
    GLsizeiptr size = (uint8_t *)g_r2d_data->line_vertices - (uint8_t *)g_r2d_data->line_vertices_begin;
    glBindBuffer(GL_ARRAY_BUFFER, g_r2d_data->line_batch_vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, g_r2d_data->line_vertices_begin);
    
    shader_use(g_r2d_data->line_shader);

    //Flush data
    glBindVertexArray(g_r2d_data->line_batch_vertex_array);
    glDrawArrays(GL_LINES, 0, g_r2d_data->current_line_count * 2);

    //Begin batch
    g_r2d_data->current_line_count = 0;
    g_r2d_data->line_vertices = g_r2d_data->line_vertices_begin;
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
Texture r2d_create_texture_from_file(str_t path, uint8_t bilinear) {
    TextureData texture_data = texture_load_data(path);
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