// Copyright 2024 Ryan Neff (@JellyTitan)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

enum sofle_layers {
    _QWERTY,
    _LOWER,
    _RAISE,
    _ADJUST,
};

#if defined(RGB_MATRIX_KEYREACTIVE_ENABLED)
void sofle_orange_reactive_record_keypress(uint8_t row, uint8_t col);
#endif

#ifdef SOFLE_OLED_GAMES
enum sofle_oled_keycodes {
    OLED_GAME_LIFE = SAFE_RANGE,
    OLED_GAME_SNAKE,
    OLED_GAME_EXIT,
};

#include "oled_games.inc"
#endif

#ifdef SOFLE_OLED_KEYMAP
#ifdef SOFLE_OLED_GAMES
enum { OLED_KEYMAP_TOGGLE = OLED_GAME_EXIT + 1 };
#else
enum { OLED_KEYMAP_TOGGLE = SAFE_RANGE };
#endif
#include "oled_keymap.inc"
#endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // clang-format off
        [_QWERTY] = LAYOUT(
        KC_GRV,   KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                          KC_6,    KC_7,    KC_8,    KC_9,   KC_0,    KC_GRV,
        KC_ESC,   KC_Q,   KC_W,    KC_E,    KC_R,    KC_T,                          KC_Y,    KC_U,    KC_I,    KC_O,   KC_P,    KC_BSPC,
        KC_TAB,   KC_A,   KC_S,    KC_D,    KC_F,    KC_G,                          KC_H,    KC_J,    KC_K,    KC_L,   KC_SCLN, KC_QUOT,
        KC_LSFT,  KC_Z,   KC_X,    KC_C,    KC_V,    KC_B,    KC_MUTE,     XXXXXXX, KC_N,    KC_M,    KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
                          KC_LGUI, KC_LALT, KC_LCTL, TL_LOWR, KC_ENT,      KC_SPC , TL_UPPR, KC_RCTL, KC_RALT, KC_RGUI
        ),
        [_LOWER] = LAYOUT(
        _______, _______, _______, _______, _______, _______,                         KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC, KC_CIRC,
        KC_PGUP, _______, _______, _______, _______, _______,                         _______, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, _______,
        KC_HOME, KC_LEFT, KC_UP,   KC_DOWN, KC_RGHT, KC_END,                          KC_MINS, KC_EQL,  KC_SLSH, KC_LBRC, KC_RBRC, KC_GRV,
        KC_PGDN, _______, _______, _______, _______, _______, _______,       _______,  KC_UNDS, KC_PLUS, KC_BSLS, KC_LCBR, KC_RCBR, KC_TILD,
                          _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______
        ),
        [_RAISE] = LAYOUT(
        _______, _______, _______, _______, _______, _______,                          _______, _______, _______, _______, _______, _______,
        KC_PGUP, _______, _______, _______, _______, _______,                          _______, _______, _______, _______, _______, KC_PGUP,
        KC_HOME, KC_LEFT, KC_UP,   KC_DOWN, KC_RGHT, KC_END,                           KC_END,  KC_LEFT, KC_UP,   KC_DOWN, KC_RGHT, KC_HOME,
        KC_PGDN, C(KC_Z), C(KC_X), C(KC_C), C(KC_V), _______, _______,        _______, _______, _______, _______, _______, _______, KC_PGDN,
                          _______, _______, _______, _______, _______,        _______, _______, _______, _______, _______
        ),
        [_ADJUST] = LAYOUT(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                       RM_TOGG, RM_VALU, RM_VALD, RM_SPDU, RM_SPDD, XXXXXXX,
        QK_BOOT,
#ifdef SOFLE_OLED_GAMES
                 OLED_GAME_LIFE, OLED_GAME_SNAKE, OLED_GAME_EXIT,
#else
                 XXXXXXX, XXXXXXX, XXXXXXX,
#endif
                                            CG_TOGG,
#ifdef SOFLE_OLED_KEYMAP
                                                     OLED_KEYMAP_TOGGLE,
#else
                                                     XXXXXXX,
#endif
                                                                                     RM_NEXT, RM_PREV, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, CG_TOGG, XXXXXXX, XXXXXXX, XXXXXXX,                       XXXXXXX, KC_VOLD, KC_MUTE, KC_VOLU, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,     XXXXXXX, XXXXXXX, KC_MPRV, KC_MPLY, KC_MNXT, XXXXXXX, XXXXXXX,
                          _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______
        )
    // clang-format on
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [1] = {ENCODER_CCW_CW(KC_PGUP, KC_PGDN)},
    [2] = {ENCODER_CCW_CW(_______, _______)},
    [3] = {ENCODER_CCW_CW(_______, _______)},
};
#endif

#if defined(SOFLE_OLED_GAMES) || defined(SOFLE_OLED_KEYMAP) || defined(RGB_MATRIX_KEYREACTIVE_ENABLED)
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#if defined(RGB_MATRIX_KEYREACTIVE_ENABLED)
    if (record->event.pressed) {
        sofle_orange_reactive_record_keypress(record->event.key.row, record->event.key.col);
    }
#endif

#ifdef SOFLE_OLED_GAMES
    switch (keycode) {
        case OLED_GAME_LIFE:
            if (record->event.pressed) {
#ifdef SOFLE_OLED_KEYMAP
                sofle_oled_keymap_set_active(false);
#endif
                sofle_oled_game_start(SOFLE_OLED_GAME_LIFE);
            }
            return false;
        case OLED_GAME_SNAKE:
            if (record->event.pressed) {
#ifdef SOFLE_OLED_KEYMAP
                sofle_oled_keymap_set_active(false);
#endif
                sofle_oled_game_start(SOFLE_OLED_GAME_SNAKE);
            }
            return false;
        case OLED_GAME_EXIT:
            if (record->event.pressed) {
                sofle_oled_game_stop();
#ifdef SOFLE_OLED_KEYMAP
                sofle_oled_keymap_set_active(true);
#endif
            }
            return false;
        case KC_UP:
        case KC_DOWN:
        case KC_LEFT:
        case KC_RIGHT:
            if (sofle_oled_game_is_snake()) {
                if (record->event.pressed) {
                    sofle_oled_game_turn(keycode);
                }
                return false;
            }
            break;
    }
#endif

#ifdef SOFLE_OLED_KEYMAP
    if (keycode == OLED_KEYMAP_TOGGLE) {
        if (record->event.pressed) {
            sofle_oled_keymap_toggle();
        }
        return false;
    }
#endif

    return true;
}
#endif

#if defined(SOFLE_OLED_GAMES) || defined(SOFLE_OLED_KEYMAP)
bool oled_task_user(void) {
#ifdef SOFLE_OLED_GAMES
    if (is_keyboard_master() && sofle_oled_game_active()) {
        sofle_oled_game_task();
#ifdef SOFLE_OLED_KEYMAP
        if (sofle_oled_keymap_is_active()) {
            sofle_oled_keymap_render();
        }
#endif
        return false;
    }
#endif

#ifdef SOFLE_OLED_KEYMAP
    if (sofle_oled_keymap_is_active()) {
        sofle_oled_keymap_render();
        return false;
    }
#endif

    return true;
}
#endif
