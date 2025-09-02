#include "util_renderer2d.h"

#define MAX_QUADS 4000
#define MAX_VERTICES (MAX_QUADS*4)
#define MAX_INDICES (MAX_QUADS*6)

#define MAX_LINES 2000

#define MAX_CHARACTERS 4000

#define MAX_TEXTURE_COUNT 256
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

typedef struct text_vertex {
    vec2_t position;
    vec2_t tex_coords;
    float tex_id;
} text_vertex_t;

typedef struct Renderer2dData {
    Shader default_shader, line_shader, text_shader;
    Texture white_texture;
    
    render_batch_t quad_batch;
    render_batch_t line_batch;
    render_batch_t text_batch;
    
    TextureArray texture_array, font_texture_array;
    
    cmap_map_t *character_map;
    uint32_t line_height;
    int32_t line_descender;
} Renderer2dData;

Renderer2dData *g_r2d_data;

void r2d_init(MemoryArena *arena) {
    g_r2d_data = arena_alloc(arena, 1, Renderer2dData);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_r2d_data->texture_array = texture_3d_create(MAX_TEXTURE_SIZE, MAX_TEXTURE_COUNT, TextureFormat_RGBA);
    g_r2d_data->font_texture_array = texture_3d_create(FONT_SIZE, 256, TextureFormat_RED);

    unsigned int white = 0xffffffff;
    g_r2d_data->white_texture = texture_3d_add(&g_r2d_data->texture_array, 1, 1, 0, TextureFormat_RGBA, (unsigned char *)&white);
    
    g_r2d_data->default_shader = shader_create(g_vertex_shader, g_fragment_shader);
    shader_use(g_r2d_data->default_shader);
    shader_set_int(g_r2d_data->default_shader, "image", 0);
    mat4_t view = mat4_identity();
    shader_set_mat4(g_r2d_data->default_shader, "view", &view);
    shader_set_int(g_r2d_data->default_shader, "texture_array", GL_TEXTURE0);
    
    g_r2d_data->line_shader = shader_create(g_vertex_shader_line, g_fragment_shader_line);
    shader_use(g_r2d_data->line_shader);
    shader_set_mat4(g_r2d_data->line_shader, "view", &view);
    
    g_r2d_data->text_shader = shader_create(g_vertex_shader_text, g_fragment_shader_text);
    shader_use(g_r2d_data->text_shader);
    shader_set_int(g_r2d_data->text_shader, "text", GL_TEXTURE0);

    //Batch Data
    {
        g_r2d_data->quad_batch.vertices = arena_alloc(arena, MAX_VERTICES, vertex_t);
        g_r2d_data->quad_batch.vertices_begin = g_r2d_data->quad_batch.vertices;

        unsigned int ebo;

        glGenVertexArrays(1, &g_r2d_data->quad_batch.vertex_array);
        glGenBuffers(1, &g_r2d_data->quad_batch.vertex_buffer);
        glGenBuffers(1, &ebo);

        glBindVertexArray(g_r2d_data->quad_batch.vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, g_r2d_data->quad_batch.vertex_buffer);
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
        g_r2d_data->line_batch.vertices = arena_alloc(arena, MAX_LINES * 2, line_vertex_t);
        g_r2d_data->line_batch.vertices_begin = g_r2d_data->line_batch.vertices;

        glGenVertexArrays(1, &g_r2d_data->line_batch.vertex_array);
        glBindVertexArray(g_r2d_data->line_batch.vertex_array);

        glGenBuffers(1, &g_r2d_data->line_batch.vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, g_r2d_data->line_batch.vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, MAX_LINES * 2 * sizeof(line_vertex_t), 0, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(line_vertex_t), (void*)offsetof(line_vertex_t, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(line_vertex_t), (void*)offsetof(line_vertex_t, color));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    {
        g_r2d_data->text_batch.vertices = arena_alloc(arena, MAX_CHARACTERS * 4, text_vertex_t);
        g_r2d_data->text_batch.vertices_begin = g_r2d_data->text_batch.vertices;

        unsigned int ebo;

        glGenVertexArrays(1, &g_r2d_data->text_batch.vertex_array);
        glGenBuffers(1, &g_r2d_data->text_batch.vertex_buffer);
        glGenBuffers(1, &ebo);

        glBindVertexArray(g_r2d_data->text_batch.vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, g_r2d_data->text_batch.vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, MAX_CHARACTERS * 4 * sizeof(text_vertex_t), 0, GL_DYNAMIC_DRAW);
       
        StackMemoryArena stack_arena = arena_push_stack_arena(arena);
        unsigned int* indices = arena_alloc(stack_arena.arena, MAX_CHARACTERS * 6, unsigned int);
        
        int offset = 0;
        for (unsigned int i = 0; i < MAX_CHARACTERS * 6; i += 6) {
            indices[i + 0] = 0 + offset;
            indices[i + 1] = 1 + offset;
            indices[i + 2] = 2 + offset;
            indices[i + 3] = 2 + offset;
            indices[i + 4] = 3 + offset;
            indices[i + 5] = 0 + offset;

            offset += 4;
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * MAX_CHARACTERS * 6, indices, GL_STATIC_DRAW);
        arena_pop_stack_arena(&stack_arena);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(text_vertex_t), (void*)offsetof(text_vertex_t, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(text_vertex_t), (void*)offsetof(text_vertex_t, tex_coords));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(text_vertex_t), (void*)offsetof(text_vertex_t, tex_id));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    FT_Library ft;
    UTIL_ASSERT(FT_Init_FreeType(&ft) == 0, "Could not init FreeType library.");

    FT_Face face;
    UTIL_ASSERT(FT_New_Face(ft, "UtilRenderer/res/ARIAL.TTF", 0, &face) == 0, "Failed to load font.");
    
    FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    UTIL_ASSERT(FT_Load_Char(face, 'X', FT_LOAD_RENDER) == 0, "Failed to load glyph");
    
    g_r2d_data->line_height = face->size->metrics.height >> 6;
    g_r2d_data->line_descender = face->size->metrics.descender >> 6;

    g_r2d_data->character_map = arena_alloc(arena, 1, cmap_map_t);

    StackMemoryArena stack_arena = arena_push_stack_arena(arena);

    for (unsigned char c = 0; c < 128; c++) {

        UTIL_ASSERT(FT_Load_Char(face, c, FT_LOAD_RENDER) == 0, "Failed to load glyph");

        Texture texture = texture_3d_add(&g_r2d_data->font_texture_array, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, TextureFormat_RED, face->glyph->bitmap.buffer);
        
        cmap_char_t character;
        character.texture = texture;
        character.size = (vec2_t) { face->glyph->bitmap.width, face->glyph->bitmap.rows };
        character.bearing = (vec2_t) { face->glyph->bitmap_left, face->glyph->bitmap_top };
        character.advance = face->glyph->advance.x;

        str_t str = str_create_empty(stack_arena.arena, 1);
        str.buf[0] = c;
        
        cmap_insert(arena, g_r2d_data->character_map, str, character);
    }
    arena_pop_stack_arena(&stack_arena);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    _r2d_batch_begin(&g_r2d_data->quad_batch);
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

    shader_use(g_r2d_data->text_shader);
    shader_set_mat4(g_r2d_data->text_shader, "projection", &projection);

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

void r2d_render_text(str_t text, vec2_t position, float scale, vec3_t color) {
    static int init = 0;
    static unsigned int VAO, VBO;

    if (init == 0) {
        init = 1;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 5, NULL, GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (2 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (4 * sizeof(float)));
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); 

    }

    shader_use(g_r2d_data->text_shader);
    shader_set_v3(g_r2d_data->text_shader, "textColor", color);
    shader_set_int(g_r2d_data->text_shader, "texture_array", GL_TEXTURE0);

    glBindVertexArray(VAO);

    for (uint32_t i = 0; i < text.size; i++) {
        str_t c = str_substr(text, i, i+1);
        int32_t index = cmap_get_index(g_r2d_data->character_map, c);
        cmap_char_t ch = g_r2d_data->character_map->values[index];

        float xpos = position.x + ch.bearing.x * scale;
        float ypos = position.y - ch.bearing.y * scale + (g_r2d_data->line_height + g_r2d_data->line_descender) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[6][5] = {
            { xpos,     ypos + h,   0.0f, ch.size.y / (float) FONT_SIZE, ch.texture.handle },            
            { xpos,     ypos,       0.0f, 0, ch.texture.handle },
            { xpos + w, ypos,       ch.size.x / (float) FONT_SIZE, 0, ch.texture.handle },

            { xpos,     ypos + h,   0.0f, ch.size.y / (float) FONT_SIZE, ch.texture.handle },
            { xpos + w, ypos,       ch.size.x / (float) FONT_SIZE, 0, ch.texture.handle },
            { xpos + w, ypos + h,   ch.size.x / (float) FONT_SIZE, ch.size.y / (float) FONT_SIZE, ch.texture.handle }           
        };

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, g_r2d_data->font_texture_array.handle);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        position.x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void r2d_render_sprite_rounded(vec2_t position, vec2_t size, Texture texture, vec4_t color, float rotation, vec2_t pivot, float rounded_radius) {
        
    if (g_r2d_data->quad_batch.count >= MAX_QUADS) {
        r2d_batch_flush(&g_r2d_data->quad_batch, 6, g_r2d_data->default_shader, g_r2d_data->texture_array);
    }

    float texture_id = (float) texture.handle;

    pivot = (vec2_t) { position.x + pivot.x * size.x, position.y + pivot.y * size.y };
    
    vec2_t lower_left = (vec2_t) { position.x, position.y + size.y };

    vertex_t *v = g_r2d_data->quad_batch.vertices;
    vec2_t pos = position;
    vec2_t rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    v[0].position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    v[0].tex_coords = (vec2_t) { 0.0f, 0.0f };
    v[0].color = color;
    v[0].rounded_radius = rounded_radius;
    v[0].rect_pos = lower_left;
    v[0].rect_size = size;
    v[0].tex_id = texture_id;
    v[0].use_bilinear = texture.use_bilinear;

    pos = (vec2_t) { position.x + size.x, position.y };
    rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    v[1].position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    v[1].tex_coords = (vec2_t) { (float) texture.width  / MAX_TEXTURE_SIZE, 0.0f };
    v[1].color = color;
    v[1].rounded_radius = rounded_radius;
    v[1].rect_pos = lower_left;
    v[1].rect_size = size;
    v[1].tex_id = texture_id;
    v[1].use_bilinear = texture.use_bilinear;

    pos = (vec2_t) { position.x + size.x, position.y + size.y };
    rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    v[2].position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    v[2].tex_coords = (vec2_t) { (float) texture.width / MAX_TEXTURE_SIZE, (float)texture.height / MAX_TEXTURE_SIZE };
    v[2].color = color;
    v[2].rounded_radius = rounded_radius;
    v[2].rect_pos = lower_left;
    v[2].rect_size = size;
    v[2].tex_id = texture_id;
    v[2].use_bilinear = texture.use_bilinear;

    pos = (vec2_t) { position.x, position.y + size.y };
    rot_pos = vec2_rotate((vec2_t) { pos.x - pivot.x, pos.y - pivot.y }, ((float) PI / 180.0f) * rotation);
    v[3].position = (vec2_t) { pivot.x + rot_pos.x, pivot.y + rot_pos.y };
    v[3].tex_coords = (vec2_t) { 0.0f, (float) texture.height / MAX_TEXTURE_SIZE };
    v[3].color = color;
    v[3].rounded_radius = rounded_radius;
    v[3].rect_pos = lower_left;
    v[3].rect_size = size;
    v[3].tex_id = texture_id;
    v[3].use_bilinear = texture.use_bilinear;

    (vertex_t*)g_r2d_data->quad_batch.vertices += 4;

    g_r2d_data->quad_batch.count++;
}

void r2d_render_thick_line(vec2_t start, vec2_t end, float thickness, vec4_t color) {
    if (g_r2d_data->quad_batch.count >= MAX_QUADS) {
        r2d_batch_flush(&g_r2d_data->quad_batch, 6, g_r2d_data->default_shader, g_r2d_data->texture_array);
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

    vertex_t *v = g_r2d_data->quad_batch.vertices;

    v[0] = (vertex_t){ p0, {0,0}, color, {0,0}, {0,0}, 0, tex_id, use_bilinear };
    v[1] = (vertex_t){ p1, {0,0}, color, {0,0}, {0,0}, 0, tex_id, use_bilinear };
    v[2] = (vertex_t){ p2, {0,0}, color, {0,0}, {0,0}, 0, tex_id, use_bilinear };
    v[3] = (vertex_t){ p3, {0,0}, color, {0,0}, {0,0}, 0, tex_id, use_bilinear };

    (vertex_t*) g_r2d_data->quad_batch.vertices += 4;
    g_r2d_data->quad_batch.count++;
}

void r2d_batch_flush(render_batch_t *batch, uint32_t indices_per, Shader shader, TextureArray texture_array) {
    _r2d_batch_end(batch);

    shader_use(shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array.handle);

    //Flush data
    glBindVertexArray(batch->vertex_array);
    glDrawElements(GL_TRIANGLES, batch->count * indices_per, GL_UNSIGNED_INT, 0);

    _r2d_batch_begin(batch);
}

void r2d_flush() {
    r2d_batch_flush(&g_r2d_data->quad_batch, 6, g_r2d_data->default_shader, g_r2d_data->texture_array);
}

Texture r2d_texture_array_add(str_t path, uint8_t bilinear, TextureFormat format) {
    TextureData texture_data = texture_load_data(path);

    Texture texture = texture_3d_add(&g_r2d_data->texture_array, texture_data.width, texture_data.height, bilinear, format, texture_data.data);

    texture_free(texture_data);

    return texture;
}

cmap_map_t *r2d_get_character_map() {
    return g_r2d_data->character_map;
}
uint32_t r2d_get_line_height() {
    return g_r2d_data->line_height;
}

void _r2d_batch_begin(render_batch_t *batch) {
    batch->count = 0;
    batch->vertices = batch->vertices_begin;
}

void _r2d_batch_end(render_batch_t *batch) {
    GLsizeiptr size = (uint8_t *) batch->vertices - (uint8_t *) batch->vertices_begin;
    glBindBuffer(GL_ARRAY_BUFFER, batch->vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, batch->vertices_begin);
}