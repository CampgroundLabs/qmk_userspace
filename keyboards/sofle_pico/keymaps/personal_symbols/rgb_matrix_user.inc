#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
RGB_MATRIX_EFFECT(ORANGE_REACTIVE)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static uint32_t orange_reactive_last_hit[RGB_MATRIX_LED_COUNT];
static bool     orange_reactive_has_hit[RGB_MATRIX_LED_COUNT];

void sofle_orange_reactive_record_keypress(uint8_t row, uint8_t col) {
    uint8_t led_indices[RGB_MATRIX_LED_COUNT];
    uint8_t led_count = rgb_matrix_map_row_column_to_led(row, col, led_indices);
    uint32_t now      = timer_read32();

    for (uint8_t i = 0; i < led_count; i++) {
        if (led_indices[i] < RGB_MATRIX_LED_COUNT) {
            orange_reactive_last_hit[led_indices[i]] = now;
            orange_reactive_has_hit[led_indices[i]]  = true;
        }
    }
}

static uint8_t orange_reactive_fade(uint8_t led, uint8_t speed) {
    if (!orange_reactive_has_hit[led]) {
        return UINT8_MAX;
    }

    uint32_t elapsed  = timer_elapsed32(orange_reactive_last_hit[led]);
    uint32_t max_tick = UINT16_MAX / speed;
    if (elapsed >= max_tick) {
        return UINT8_MAX;
    }

    uint16_t offset = (elapsed * speed) / 256;
    return offset > UINT8_MAX ? UINT8_MAX : (uint8_t)offset;
}

static uint8_t orange_reactive_blend_channel(uint8_t from, uint8_t to, uint8_t fade, uint8_t brightness) {
    uint32_t value = ((uint32_t)from * (UINT8_MAX - fade) + (uint32_t)to * fade) * brightness;
    return (value + (UINT8_MAX * UINT8_MAX / 2)) / (UINT8_MAX * UINT8_MAX);
}

static bool ORANGE_REACTIVE(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint8_t speed     = qadd8(rgb_matrix_config.speed, 1);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        uint8_t fade  = orange_reactive_fade(i, speed);
        uint8_t red   = orange_reactive_blend_channel(150, 24, fade, rgb_matrix_config.hsv.v);
        uint8_t green = orange_reactive_blend_channel(55, 24, fade, rgb_matrix_config.hsv.v);
        uint8_t blue  = orange_reactive_blend_channel(8, 24, fade, rgb_matrix_config.hsv.v);
        rgb_matrix_set_color(i, red, green, blue);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // RGB_MATRIX_KEYREACTIVE_ENABLED
