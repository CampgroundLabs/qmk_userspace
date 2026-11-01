#define SOFLE_GAME_CELL_SIZE 4
#define SOFLE_GAME_HEADER_HEIGHT 8
#define SOFLE_GAME_WIDTH (OLED_DISPLAY_WIDTH / SOFLE_GAME_CELL_SIZE)
#define SOFLE_GAME_HEIGHT ((OLED_DISPLAY_HEIGHT - SOFLE_GAME_HEADER_HEIGHT) / SOFLE_GAME_CELL_SIZE)
#define SOFLE_SNAKE_CAPACITY (SOFLE_GAME_WIDTH * SOFLE_GAME_HEIGHT)
#define SOFLE_GAME_FRAME_MS 100
#define SOFLE_LIFE_STEP_MS 220
#define SOFLE_SNAKE_STEP_MS 160

enum sofle_oled_game {
    SOFLE_OLED_GAME_NONE,
    SOFLE_OLED_GAME_LIFE,
    SOFLE_OLED_GAME_SNAKE,
};

enum sofle_snake_direction {
    SOFLE_SNAKE_UP,
    SOFLE_SNAKE_RIGHT,
    SOFLE_SNAKE_DOWN,
    SOFLE_SNAKE_LEFT,
};

static enum sofle_oled_game current_oled_game;
static uint16_t oled_game_frame_timer;
static uint16_t life_step_timer;
static uint16_t snake_step_timer;
static uint16_t game_random_state = 0xC0DE;
static uint8_t life_cells[2][SOFLE_GAME_HEIGHT][SOFLE_GAME_WIDTH];
static uint8_t life_active_buffer;
static uint16_t snake_cells[SOFLE_SNAKE_CAPACITY];
static uint16_t snake_length;
static uint16_t snake_food;
static uint8_t snake_direction;
static bool snake_game_over;

static uint16_t sofle_oled_game_random16(void) {
    game_random_state = game_random_state * 25173 + 13849;
    return game_random_state;
}

static void sofle_oled_game_draw_cell(uint8_t x, uint8_t y) {
    const uint8_t left = x * SOFLE_GAME_CELL_SIZE + 1;
    const uint8_t top  = SOFLE_GAME_HEADER_HEIGHT + y * SOFLE_GAME_CELL_SIZE + 1;

    oled_write_pixel(left, top, true);
    oled_write_pixel(left + 1, top, true);
    oled_write_pixel(left, top + 1, true);
    oled_write_pixel(left + 1, top + 1, true);
}

static void sofle_oled_life_init(void) {
    life_active_buffer = 0;
    for (uint8_t y = 0; y < SOFLE_GAME_HEIGHT; y++) {
        for (uint8_t x = 0; x < SOFLE_GAME_WIDTH; x++) {
            life_cells[0][y][x] = (sofle_oled_game_random16() & 0xFF) < 80;
            life_cells[1][y][x] = 0;
        }
    }
    life_step_timer = timer_read();
}

static void sofle_oled_life_step(void) {
    uint8_t next_buffer = life_active_buffer ^ 1;

    for (uint8_t y = 0; y < SOFLE_GAME_HEIGHT; y++) {
        for (uint8_t x = 0; x < SOFLE_GAME_WIDTH; x++) {
            uint8_t neighbors = 0;
            for (int8_t dy = -1; dy <= 1; dy++) {
                for (int8_t dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) {
                        continue;
                    }

                    int8_t neighbor_x = x + dx;
                    int8_t neighbor_y = y + dy;
                    if (neighbor_x >= 0 && neighbor_x < SOFLE_GAME_WIDTH && neighbor_y >= 0 && neighbor_y < SOFLE_GAME_HEIGHT) {
                        neighbors += life_cells[life_active_buffer][neighbor_y][neighbor_x];
                    }
                }
            }

            bool alive = life_cells[life_active_buffer][y][x];
            life_cells[next_buffer][y][x] = neighbors == 3 || (alive && neighbors == 2);
        }
    }

    life_active_buffer = next_buffer;
    life_step_timer = timer_read();
}

static bool sofle_snake_contains(uint16_t cell, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        if (snake_cells[i] == cell) {
            return true;
        }
    }
    return false;
}

static bool sofle_snake_spawn_food(void) {
    uint16_t total = SOFLE_GAME_WIDTH * SOFLE_GAME_HEIGHT;
    uint16_t start = sofle_oled_game_random16() % total;

    for (uint16_t offset = 0; offset < total; offset++) {
        uint16_t cell = (start + offset) % total;
        uint8_t x = cell % SOFLE_GAME_WIDTH;
        uint8_t y = cell / SOFLE_GAME_WIDTH;
        if (x > 0 && x < SOFLE_GAME_WIDTH - 1 && y > 0 && y < SOFLE_GAME_HEIGHT - 1 && !sofle_snake_contains(cell, snake_length)) {
            snake_food = cell;
            return true;
        }
    }

    return false;
}

static void sofle_oled_snake_init(void) {
    uint8_t center_x = SOFLE_GAME_WIDTH / 2;
    uint8_t center_y = SOFLE_GAME_HEIGHT / 2;

    snake_length = 3;
    snake_cells[0] = center_y * SOFLE_GAME_WIDTH + center_x;
    snake_cells[1] = center_y * SOFLE_GAME_WIDTH + center_x - 1;
    snake_cells[2] = center_y * SOFLE_GAME_WIDTH + center_x - 2;
    snake_direction = SOFLE_SNAKE_RIGHT;
    snake_game_over = !sofle_snake_spawn_food();
    snake_step_timer = timer_read();
}

static void sofle_oled_snake_turn(uint8_t direction) {
    if ((direction + 2) % 4 != snake_direction) {
        snake_direction = direction;
    }
}

static void sofle_oled_snake_step(void) {
    uint16_t head = snake_cells[0];
    int8_t x = head % SOFLE_GAME_WIDTH;
    int8_t y = head / SOFLE_GAME_WIDTH;

    switch (snake_direction) {
        case SOFLE_SNAKE_UP:
            y--;
            break;
        case SOFLE_SNAKE_RIGHT:
            x++;
            break;
        case SOFLE_SNAKE_DOWN:
            y++;
            break;
        case SOFLE_SNAKE_LEFT:
            x--;
            break;
    }

    if (x <= 0 || x >= SOFLE_GAME_WIDTH - 1 || y <= 0 || y >= SOFLE_GAME_HEIGHT - 1) {
        snake_game_over = true;
        return;
    }

    uint16_t next_head = y * SOFLE_GAME_WIDTH + x;
    bool eating = next_head == snake_food;
    uint16_t collision_length = snake_length - (eating ? 0 : 1);
    if (sofle_snake_contains(next_head, collision_length)) {
        snake_game_over = true;
        return;
    }

    if (eating && snake_length < SOFLE_SNAKE_CAPACITY) {
        snake_length++;
    }
    for (uint16_t i = snake_length - 1; i > 0; i--) {
        snake_cells[i] = snake_cells[i - 1];
    }
    snake_cells[0] = next_head;

    if (eating && snake_length < SOFLE_SNAKE_CAPACITY && !sofle_snake_spawn_food()) {
        snake_game_over = true;
    }
    snake_step_timer = timer_read();
}

static void sofle_oled_game_draw_border(void) {
    uint8_t right = SOFLE_GAME_WIDTH * SOFLE_GAME_CELL_SIZE - 1;
    uint8_t bottom = OLED_DISPLAY_HEIGHT - 1;

    for (uint8_t x = 0; x <= right; x++) {
        oled_write_pixel(x, SOFLE_GAME_HEADER_HEIGHT, true);
        oled_write_pixel(x, bottom, true);
    }
    for (uint8_t y = SOFLE_GAME_HEADER_HEIGHT; y <= bottom; y++) {
        oled_write_pixel(0, y, true);
        oled_write_pixel(right, y, true);
    }
}

static void sofle_oled_life_render(void) {
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write_P(PSTR("GAME OF LIFE"), false);

    for (uint8_t y = 0; y < SOFLE_GAME_HEIGHT; y++) {
        for (uint8_t x = 0; x < SOFLE_GAME_WIDTH; x++) {
            if (life_cells[life_active_buffer][y][x]) {
                sofle_oled_game_draw_cell(x, y);
            }
        }
    }
}

static void sofle_oled_snake_render(void) {
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write_P(PSTR("SNAKE"), false);
    if (snake_game_over) {
        oled_write_P(PSTR(" OVER"), false);
    }
    sofle_oled_game_draw_border();

    for (uint16_t i = 0; i < snake_length; i++) {
        sofle_oled_game_draw_cell(snake_cells[i] % SOFLE_GAME_WIDTH, snake_cells[i] / SOFLE_GAME_WIDTH);
    }
    if (!snake_game_over) {
        sofle_oled_game_draw_cell(snake_food % SOFLE_GAME_WIDTH, snake_food / SOFLE_GAME_WIDTH);
    }
}

static void sofle_oled_game_start(enum sofle_oled_game game) {
    current_oled_game = game;
    game_random_state ^= timer_read();
    oled_game_frame_timer = timer_read() - SOFLE_GAME_FRAME_MS;

    if (game == SOFLE_OLED_GAME_LIFE) {
        sofle_oled_life_init();
    } else if (game == SOFLE_OLED_GAME_SNAKE) {
        sofle_oled_snake_init();
    }
}

static void sofle_oled_game_stop(void) {
    current_oled_game = SOFLE_OLED_GAME_NONE;
    oled_clear();
}

static bool sofle_oled_game_active(void) {
    return current_oled_game != SOFLE_OLED_GAME_NONE;
}

static bool sofle_oled_game_is_snake(void) {
    return current_oled_game == SOFLE_OLED_GAME_SNAKE;
}

static void sofle_oled_game_turn(uint16_t keycode) {
    switch (keycode) {
        case KC_UP:
            sofle_oled_snake_turn(SOFLE_SNAKE_UP);
            break;
        case KC_RIGHT:
            sofle_oled_snake_turn(SOFLE_SNAKE_RIGHT);
            break;
        case KC_DOWN:
            sofle_oled_snake_turn(SOFLE_SNAKE_DOWN);
            break;
        case KC_LEFT:
            sofle_oled_snake_turn(SOFLE_SNAKE_LEFT);
            break;
    }
}

static void sofle_oled_game_task(void) {
    if (timer_elapsed(oled_game_frame_timer) < SOFLE_GAME_FRAME_MS) {
        return;
    }
    oled_game_frame_timer = timer_read();
    oled_on();

    if (current_oled_game == SOFLE_OLED_GAME_LIFE && timer_elapsed(life_step_timer) >= SOFLE_LIFE_STEP_MS) {
        sofle_oled_life_step();
    } else if (current_oled_game == SOFLE_OLED_GAME_SNAKE && !snake_game_over && timer_elapsed(snake_step_timer) >= SOFLE_SNAKE_STEP_MS) {
        sofle_oled_snake_step();
    }

    if (current_oled_game == SOFLE_OLED_GAME_LIFE) {
        sofle_oled_life_render();
    } else if (current_oled_game == SOFLE_OLED_GAME_SNAKE) {
        sofle_oled_snake_render();
    }
}
