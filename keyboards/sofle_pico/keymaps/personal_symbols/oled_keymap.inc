#include "transactions.h"

static bool sofle_oled_keymap_active = true;

static void sofle_oled_keymap_sync(uint8_t initiator2target_buffer_size, const void *initiator2target_buffer, uint8_t target2initiator_buffer_size, void *target2initiator_buffer) {
    if (initiator2target_buffer_size == sizeof(sofle_oled_keymap_active)) {
        sofle_oled_keymap_active = *(const bool *)initiator2target_buffer;
    }
}

static void sofle_oled_keymap_write_outer_label(uint16_t keycode, bool shifted) {
    char label[4] = {' ', ' ', ' ', '\0'};

    switch (keycode) {
        case KC_ESC:
            memcpy(label, "Esc", 3);
            break;
        case KC_TAB:
            memcpy(label, "Tab", 3);
            break;
        case KC_BSPC:
            memcpy(label, "Bsp", 3);
            break;
        case KC_DEL:
            memcpy(label, "Del", 3);
            break;
        case KC_HOME:
            memcpy(label, "Hom", 3);
            break;
        case KC_END:
            memcpy(label, "End", 3);
            break;
        case KC_PGUP:
            memcpy(label, "Pup", 3);
            break;
        case KC_PGDN:
            memcpy(label, "Pdn", 3);
            break;
        case KC_LSFT:
        case KC_RSFT:
            memcpy(label, "Sft", 3);
            break;
        case KC_QUOT:
            label[1] = shifted ? '"' : '\'';
            break;
        case KC_GRV:
            label[1] = shifted ? '~' : '`';
            break;
        case KC_CIRC:
            label[1] = '^';
            break;
        case KC_TILD:
            label[1] = '~';
            break;
        case KC_UNDS:
            label[1] = '_';
            break;
        default:
            label[1] = '?';
            break;
    }

    oled_write(label, false);
}

static void sofle_oled_keymap_write_label(uint16_t keycode, bool expanded, bool full_label, uint8_t layer, bool shifted) {
    if (keycode == TL_LOWR || keycode == TL_UPPR) {
        oled_write_char('L', false);
        oled_write_char(layer == _LOWER || layer == _RAISE ? '4' : keycode == TL_LOWR ? '2' : '3', false);
        return;
    }

    char label[4] = {' ', ' ', ' ', '\0'};

    if (keycode >= KC_A && keycode <= KC_Z) {
        label[1] = (shifted ? 'A' : 'a') + (keycode - KC_A);
    } else if (keycode >= KC_1 && keycode <= KC_0) {
        label[1] = '1' + (keycode - KC_1);
        if (keycode == KC_0) {
            label[1] = '0';
        }
        if (shifted) {
            static const char shifted_digits[] = "!@#$%^&*()";
            label[1] = shifted_digits[keycode == KC_0 ? 9 : keycode - KC_1];
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
            case KC_TILD:
                label[1] = '~';
                break;
            case KC_UNDS:
                label[1] = '_';
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

    if (shifted) {
        char shifted_symbol = '\0';
        switch (keycode) {
            case KC_MINS:
                shifted_symbol = '_';
                break;
            case KC_EQL:
                shifted_symbol = '+';
                break;
            case KC_LBRC:
                shifted_symbol = '{';
                break;
            case KC_RBRC:
                shifted_symbol = '}';
                break;
            case KC_BSLS:
                shifted_symbol = '|';
                break;
            case KC_SCLN:
                shifted_symbol = ':';
                break;
            case KC_QUOT:
                shifted_symbol = '"';
                break;
            case KC_COMM:
                shifted_symbol = '<';
                break;
            case KC_DOT:
                shifted_symbol = '>';
                break;
            case KC_SLSH:
                shifted_symbol = '?';
                break;
            case KC_GRV:
                shifted_symbol = '~';
                break;
        }
        if (shifted_symbol != '\0') {
            label[0] = ' ';
            label[1] = shifted_symbol;
            label[2] = ' ';
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

    if (full_label) {
        char first = compact_label;
        char second = label[0] != ' ' ? label[1] : label[2];
        switch (keycode) {
            case KC_LCTL:
            case KC_RCTL:
                first = 'C';
                second = 't';
                break;
            case KC_LALT:
            case KC_RALT:
                first = 'A';
                second = 'l';
                break;
            case KC_LGUI:
            case KC_RGUI:
                first = 'G';
                second = 'u';
                break;
            case KC_ENT:
                first = 'E';
                second = 'n';
                break;
            case KC_SPC:
                first = 'S';
                second = 'p';
                break;
            case KC_BSPC:
                first = 'B';
                second = 's';
                break;
            case KC_TAB:
                first = 'T';
                second = 'b';
                break;
            case KC_ESC:
                first = 'E';
                second = 's';
                break;
            case KC_LSFT:
            case KC_RSFT:
                first = 'S';
                second = 'f';
                break;
            case TL_LOWR:
                first = 'L';
                second = layer == _LOWER || layer == _RAISE ? '4' : '2';
                break;
            case TL_UPPR:
                first = 'L';
                second = layer == _LOWER || layer == _RAISE ? '4' : '3';
                break;
        }
        oled_write_char(first, false);
        oled_write_char(second, false);
        return;
    }

    if (expanded) {
        switch (keycode) {
            case KC_ESC:
                oled_write("Esc", false);
                return;
            case KC_TAB:
                oled_write("Tab", false);
                return;
            case KC_BSPC:
                oled_write("Bsp", false);
                return;
            case KC_LSFT:
            case KC_RSFT:
                oled_write("Sft", false);
                return;
        }
    }

    oled_write_char(compact_label, false);
}

static void sofle_oled_keymap_write_thumb_label(uint16_t keycode, uint8_t layer) {
    switch (keycode) {
        case KC_LGUI:
        case KC_RGUI:
            oled_write("Os", false);
            break;
        case KC_LALT:
        case KC_RALT:
            oled_write("Alt", false);
            break;
        case KC_LCTL:
        case KC_RCTL:
            oled_write("Ctl", false);
            break;
        case KC_ENT:
            oled_write("Ent", false);
            break;
        case KC_SPC:
            oled_write("Spc", false);
            break;
        case KC_BSPC:
            oled_write("Bsp", false);
            break;
        case TL_LOWR:
            oled_write(layer == _LOWER || layer == _RAISE ? "L4" : "L1", false);
            break;
        case TL_UPPR:
            oled_write(layer == _LOWER || layer == _RAISE ? "L4" : "L2", false);
            break;
        default:
            oled_write("???", false);
            break;
    }
}

static uint16_t sofle_oled_keymap_keycode(uint8_t row, uint8_t col) {
    keypos_t key = {.row = row, .col = col};
    uint8_t effective_layer = layer_switch_get_layer(key);
    return keymap_key_to_keycode(effective_layer, key);
}

static void sofle_oled_keymap_render_thumb_row(uint8_t row_offset, uint8_t row, bool is_left, uint8_t layer, uint8_t line) {
    static const uint8_t left_label_positions[] = {0, 3, 7};
    static const uint8_t right_label_positions[] = {0, 4, 8};
    static const uint8_t staggered_positions[] = {2, 6};

    for (uint8_t index = 0; index < 3; index++) {
        uint8_t col = is_left ? index * 2 : MATRIX_COLS - 2 - index * 2;
        oled_set_cursor(is_left ? left_label_positions[index] : right_label_positions[index], line);
        sofle_oled_keymap_write_thumb_label(sofle_oled_keymap_keycode(row_offset + row, col), layer);
    }

    for (uint8_t index = 0; index < 2; index++) {
        uint8_t col = is_left ? index * 2 + 1 : MATRIX_COLS - 3 - index * 2;
        oled_set_cursor(staggered_positions[index], line + 1);
        sofle_oled_keymap_write_thumb_label(sofle_oled_keymap_keycode(row_offset + row, col), layer);
    }
}

static void sofle_oled_keymap_render(void) {
    uint8_t layer = get_highest_layer(layer_state);
    uint8_t row_offset = is_keyboard_left() ? 0 : MATRIX_ROWS / 2;
    bool is_left = is_keyboard_left();
    bool shifted = (get_mods() & MOD_MASK_SHIFT) != 0;
    uint8_t line = 0;

    oled_on();
    oled_clear();
    oled_set_cursor(0, line++);
    oled_write(is_left ? "L Layer " : "R Layer ", false);
    oled_write_char(layer < 10 ? '0' + layer : '?', false);
    line++;

    for (uint8_t row = 0; row < MATRIX_ROWS / 2; row++) {
        if (row == MATRIX_ROWS / 2 - 1) {
            line++;
            sofle_oled_keymap_render_thumb_row(row_offset, row, is_left, layer, line);
            continue;
        }
        oled_set_cursor(0, line++);

        keypos_t key = {.row = row_offset + row, .col = 0};
        if (is_left) {
            uint8_t effective_layer = layer_switch_get_layer(key);
            sofle_oled_keymap_write_outer_label(keymap_key_to_keycode(effective_layer, key), shifted);
            oled_write_char(' ', false);
            for (uint8_t col = 1; col < MATRIX_COLS; col++) {
                key.col = col;
                effective_layer = layer_switch_get_layer(key);
                sofle_oled_keymap_write_label(keymap_key_to_keycode(effective_layer, key), false, false, layer, shifted);
            }
        } else {
            for (uint8_t col = MATRIX_COLS - 1; col > 0; col--) {
                key.col = col;
                uint8_t effective_layer = layer_switch_get_layer(key);
                sofle_oled_keymap_write_label(keymap_key_to_keycode(effective_layer, key), false, false, layer, shifted);
            }
            oled_write_char(' ', false);
            uint8_t effective_layer = layer_switch_get_layer(key);
            sofle_oled_keymap_write_outer_label(keymap_key_to_keycode(effective_layer, key), shifted);
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
