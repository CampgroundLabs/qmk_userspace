#include "transactions.h"

static bool sofle_oled_keymap_active = true;

static void sofle_oled_keymap_sync(uint8_t initiator2target_buffer_size, const void *initiator2target_buffer, uint8_t target2initiator_buffer_size, void *target2initiator_buffer) {
    if (initiator2target_buffer_size == sizeof(sofle_oled_keymap_active)) {
        sofle_oled_keymap_active = *(const bool *)initiator2target_buffer;
    }
}

static const char *sofle_oled_layer_name(uint8_t layer) {
    switch (layer) {
        case _QWERTY:
            return "QWERTY";
        case _LOWER:
            return "LOWER";
        case _RAISE:
            return "RAISE";
        case _ADJUST:
            return "ADJUST";
        default:
            return "LAYER";
    }
}

static void sofle_oled_keymap_write_label(uint16_t keycode) {
    char label[4] = {' ', ' ', ' ', '\0'};

    if (keycode >= KC_A && keycode <= KC_Z) {
        label[1] = 'A' + (keycode - KC_A);
    } else if (keycode >= KC_1 && keycode <= KC_0) {
        label[1] = '1' + (keycode - KC_1);
        if (keycode == KC_0) {
            label[1] = '0';
        }
    } else {
        switch (keycode) {
            case KC_NO:
                label[1] = '-';
                label[2] = '-';
                break;
            case KC_TRNS:
                label[1] = '.';
                label[2] = '.';
                break;
            case KC_LCTL:
            case KC_RCTL:
                label[0] = 'C';
                label[1] = 'T';
                label[2] = 'L';
                break;
            case KC_LSFT:
            case KC_RSFT:
                label[0] = 'S';
                label[1] = 'F';
                label[2] = 'T';
                break;
            case KC_LALT:
            case KC_RALT:
                label[0] = 'A';
                label[1] = 'L';
                label[2] = 'T';
                break;
            case KC_LGUI:
            case KC_RGUI:
                label[0] = 'G';
                label[1] = 'U';
                label[2] = 'I';
                break;
            case KC_ENT:
                label[0] = 'E';
                label[1] = 'N';
                label[2] = 'T';
                break;
            case KC_SPC:
                label[0] = 'S';
                label[1] = 'P';
                label[2] = 'C';
                break;
            case KC_BSPC:
                label[0] = 'B';
                label[1] = 'S';
                label[2] = 'P';
                break;
            case KC_TAB:
                label[0] = 'T';
                label[1] = 'A';
                label[2] = 'B';
                break;
            case KC_ESC:
                label[0] = 'E';
                label[1] = 'S';
                label[2] = 'C';
                break;
            case KC_UP:
                label[1] = 'U';
                label[2] = 'P';
                break;
            case KC_DOWN:
                label[1] = 'D';
                label[2] = 'N';
                break;
            case KC_LEFT:
                label[1] = 'L';
                label[2] = 'T';
                break;
            case KC_RIGHT:
                label[1] = 'R';
                label[2] = 'T';
                break;
            case KC_EXLM:
                label[1] = '!';
                break;
            case KC_AT:
                label[1] = '@';
                break;
            case KC_HASH:
                label[1] = '#';
                break;
            case KC_DLR:
                label[1] = '$';
                break;
            case KC_PERC:
                label[1] = '%';
                break;
            case KC_CIRC:
                label[1] = '^';
                break;
            case KC_AMPR:
                label[1] = '&';
                break;
            case KC_ASTR:
                label[1] = '*';
                break;
            case KC_LPRN:
                label[1] = '(';
                break;
            case KC_RPRN:
                label[1] = ')';
                break;
            case KC_PIPE:
                label[1] = '|';
                break;
            case KC_PLUS:
                label[1] = '+';
                break;
            case KC_LCBR:
                label[1] = '{';
                break;
            case KC_RCBR:
                label[1] = '}';
                break;
            case KC_COLN:
                label[1] = ':';
                break;
            case KC_INS:
                label[0] = 'I';
                label[1] = 'N';
                label[2] = 'S';
                break;
            case KC_PSCR:
                label[0] = 'P';
                label[1] = 'S';
                label[2] = 'C';
                break;
            case KC_APP:
                label[0] = 'A';
                label[1] = 'P';
                label[2] = 'P';
                break;
            case KC_PGUP:
                label[0] = 'P';
                label[1] = 'U';
                label[2] = 'P';
                break;
            case KC_PGDN:
                label[0] = 'P';
                label[1] = 'D';
                label[2] = 'N';
                break;
            case KC_DEL:
                label[0] = 'D';
                label[1] = 'E';
                label[2] = 'L';
                break;
            case KC_CAPS:
                label[0] = 'C';
                label[1] = 'A';
                label[2] = 'P';
                break;
            case C(KC_Z):
                label[0] = 'C';
                label[1] = '-';
                label[2] = 'Z';
                break;
            case C(KC_X):
                label[0] = 'C';
                label[1] = '-';
                label[2] = 'X';
                break;
            case C(KC_C):
                label[0] = 'C';
                label[1] = '-';
                label[2] = 'C';
                break;
            case C(KC_V):
                label[0] = 'C';
                label[1] = '-';
                label[2] = 'V';
                break;
#ifdef SOFLE_OLED_GAMES
            case OLED_GAME_LIFE:
                label[0] = 'L';
                label[1] = 'I';
                label[2] = 'F';
                break;
            case OLED_GAME_SNAKE:
                label[0] = 'S';
                label[1] = 'N';
                label[2] = 'K';
                break;
            case OLED_GAME_EXIT:
                label[0] = 'E';
                label[1] = 'X';
                label[2] = 'T';
                break;
#endif
            case OLED_KEYMAP_TOGGLE:
                label[0] = 'M';
                label[1] = 'A';
                label[2] = 'P';
                break;
            case KC_GRV:
                label[1] = '`';
                break;
            case KC_MINS:
                label[1] = '-';
                break;
            case KC_EQL:
                label[1] = '=';
                break;
            case KC_LBRC:
                label[1] = '[';
                break;
            case KC_RBRC:
                label[1] = ']';
                break;
            case KC_BSLS:
                label[1] = '\\';
                break;
            case KC_SCLN:
                label[1] = ';';
                break;
            case KC_QUOT:
                label[1] = '\'';
                break;
            case KC_COMM:
                label[1] = ',';
                break;
            case KC_DOT:
                label[1] = '.';
                break;
            case KC_SLSH:
                label[1] = '/';
                break;
            default: {
                const char *name = get_keycode_string(keycode);
                const char *keycode_name = name;
                for (uint8_t i = 0; name[i] != '\0'; i++) {
                    if (name[i] == 'K' && name[i + 1] == 'C' && name[i + 2] == '_') {
                        keycode_name = &name[i + 3];
                    }
                }
                for (uint8_t i = 0; i < 3 && keycode_name[i] != '\0'; i++) {
                    if (keycode_name[i] != '_' && keycode_name[i] != '(' && keycode_name[i] != ')') {
                        label[i] = keycode_name[i];
                    }
                }
                break;
            }
        }
    }

    char compact_label = label[0] != ' ' ? label[0] : label[1];
    switch (keycode) {
        case KC_SPC:
            compact_label = '_';
            break;
        case KC_BSPC:
            compact_label = '<';
            break;
        case KC_ESC:
            compact_label = 'e';
            break;
        case C(KC_Z):
            compact_label = 'z';
            break;
        case C(KC_X):
            compact_label = 'x';
            break;
        case C(KC_C):
            compact_label = 'c';
            break;
        case C(KC_V):
            compact_label = 'v';
            break;
    }

    oled_write_char(compact_label, false);
}

static void sofle_oled_keymap_render(void) {
    uint8_t layer = get_highest_layer(layer_state);
    uint8_t row_offset = is_keyboard_left() ? 0 : MATRIX_ROWS / 2;
    uint8_t line = 0;

    oled_on();
    oled_clear();
    oled_set_cursor(0, line++);
    oled_write(is_keyboard_left() ? "L " : "R ", false);
    oled_write(sofle_oled_layer_name(layer), false);

    for (uint8_t row = 0; row < MATRIX_ROWS / 2; row++) {
        oled_set_cursor(0, line++);
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (col == MATRIX_COLS / 2) {
                oled_write_char(' ', false);
            }
            keypos_t key = {.row = row_offset + row, .col = col};
            uint8_t effective_layer = layer_switch_get_layer(key);
            uint16_t keycode = keymap_key_to_keycode(effective_layer, key);
            sofle_oled_keymap_write_label(keycode);
        }
    }
}

static bool sofle_oled_keymap_is_active(void) {
    return sofle_oled_keymap_active;
}

static void sofle_oled_keymap_set_active(bool active) {
    if (sofle_oled_keymap_active == active) {
        return;
    }

    sofle_oled_keymap_active = active;
    if (is_keyboard_master() && !transaction_rpc_send(SOFLE_OLED_KEYMAP_SYNC, sizeof(sofle_oled_keymap_active), &sofle_oled_keymap_active)) {
        dprintf("OLED keymap screen sync failed\n");
    }
}

static void sofle_oled_keymap_toggle(void) {
    sofle_oled_keymap_set_active(!sofle_oled_keymap_active);
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(SOFLE_OLED_KEYMAP_SYNC, sofle_oled_keymap_sync);
}
