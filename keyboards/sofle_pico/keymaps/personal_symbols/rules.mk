TRI_LAYER_ENABLE = yes
RGB_MATRIX_CUSTOM_USER = yes
KEYCODE_STRING_ENABLE = yes

SOFLE_ENABLE_VIAL ?= no
ifeq ($(SOFLE_ENABLE_VIAL),yes)
VIA_ENABLE = yes
VIAL_ENABLE = yes
ENCODER_MAP_ENABLE = yes
OPT_DEFS += -DSOFLE_ENABLE_VIAL
endif

# Set to no to exclude the OLED Game of Life and Snake controls.
SOFLE_OLED_GAMES ?= yes
ifeq ($(SOFLE_OLED_GAMES),yes)
OPT_DEFS += -DSOFLE_OLED_GAMES
endif

# Set to no to exclude the compiled keymap OLED screen.
SOFLE_OLED_KEYMAP ?= yes
ifeq ($(SOFLE_OLED_KEYMAP),yes)
OPT_DEFS += -DSOFLE_OLED_KEYMAP

.PHONY: oled-keymap-previews
all: oled-keymap-previews
oled-keymap-previews: $(INTERMEDIATE_OUTPUT)/src/default_keyboard.h $(KEYMAP_C)
	python3 "$(QMK_USERSPACE)/tools/oled_text_preview.py" \
		--keymap "$(KEYMAP_C)" \
		--layout-header "$(INTERMEDIATE_OUTPUT)/src/default_keyboard.h" \
		--output-dir "$(QMK_USERSPACE)/.build/oled-preview/personal_symbols" \
		--games "$(SOFLE_OLED_GAMES)"
endif
