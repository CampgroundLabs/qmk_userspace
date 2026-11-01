#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
RGB_MATRIX_EFFECT(ORANGE_REACTIVE)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static bool ORANGE_REACTIVE(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint8_t speed     = qadd8(rgb_matrix_config.speed, 1);
    uint16_t max_tick = UINT16_MAX / speed;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        uint16_t tick = max_tick;
        for (int8_t j = g_last_hit_tracker.count - 1; j >= 0; j--) {
            if (g_last_hit_tracker.index[j] == i && g_last_hit_tracker.tick[j] < tick) {
                tick = g_last_hit_tracker.tick[j];
                break;
            }
        }

        uint16_t offset = scale16by8(tick, speed);
        if (offset > UINT8_MAX) {
            offset = UINT8_MAX;
        }

        uint8_t orange = UINT8_MAX - offset;
        uint8_t red    = scale8(scale8(127, orange) + scale8(16, offset), rgb_matrix_config.hsv.v);
        uint8_t green  = scale8(scale8(44, orange) + scale8(16, offset), rgb_matrix_config.hsv.v);
        uint8_t blue   = scale8(scale8(16, offset), rgb_matrix_config.hsv.v);
        rgb_matrix_set_color(i, red, green, blue);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // RGB_MATRIX_KEYREACTIVE_ENABLED
