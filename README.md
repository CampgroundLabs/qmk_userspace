# QMK Userspace

Repository: https://github.com/CampgroundLabs/qmk_userspace

This external userspace contains your `personal` keymaps for Sofle Pico and
Corne, plus an alternate `personal_symbols` Sofle Pico keymap. The Sofle Pico
keymaps have a dim-white idle RGB Matrix and an orange keypress fade.

Configure QMK to use this userspace:

```sh
qmk config user.overlay_dir="$(realpath /path/to/keebs/qmk_userspace)"
```

Then build and flash with:

```sh
qmk compile -kb sofle_pico -km personal
qmk flash -kb sofle_pico -km personal

qmk compile -kb sofle_pico -km personal_symbols
qmk flash -kb sofle_pico -km personal_symbols

qmk compile -kb crkbd -km personal
qmk flash -kb crkbd -km personal

qmk compile -kb crkbd -km jcampo_miryoku
qmk flash -kb crkbd -km jcampo_miryoku
```

The Sofle Pico keymap gets RGB defaults from its `config.h`. On its Adjust
layer, `RM_VALU`/`RM_VALD` change brightness and `RM_SPDU`/`RM_SPDD` change
fade speed.

## Sofle Pico alternate symbols keymap

`personal_symbols` is a separate build target that preserves the existing
`personal` keymap and carries over its OLED keymap display, Game of Life, Snake,
RGB behavior, and optional Vial build mode. Its base layer remains QWERTY with
the number row, rather than adding a numpad layer. Hold Lower for a dedicated
symbols layer: shifted number-row symbols are grouped on the right half, with
arrows and navigation on the left, inspired by the Oryx Ergodox layout. Hold
Raise for a navigation layer with mirrored arrow clusters and Home/End/Page Up/
Page Down on both halves. Hold Lower and Raise together for the existing Adjust
layer.

The alternate target generates its own OLED preview images in
`.build/oled-preview/personal_symbols/`, separate from the `personal` previews.
To build it with Vial, use the same Vial-QMK checkout and add
`-e SOFLE_ENABLE_VIAL=yes`:

```sh
qmk compile -kb sofle_pico -km personal_symbols -e SOFLE_ENABLE_VIAL=yes
```

## Sofle Pico OLED games

The Game of Life and Snake are included by default in the `personal` keymap.
To exclude them from a one-off build, pass `-e SOFLE_OLED_GAMES=no` to
`qmk compile`.

```make
SOFLE_OLED_GAMES = no
```

Hold the Lower and Raise thumb keys together to enter Adjust. On its second
row, use the three keys immediately to the right of `QK_BOOT` to start Life,
start Snake, or exit the current game:

- While Snake is running, use Raise plus the arrow keys to steer.
- Use the game-exit key in Adjust to return to the keymap screen.
- The compiled keymap view is shown on both OLEDs by default. In Adjust, the key
  next to `CG_TOGG` toggles it off or back on. Each half displays its own keys
  for the active layer, with the letter columns toward the keyboard center and
  the outer column at the outside edge. Each matrix row has its own line; the
  outer key has a fixed three-character slot so the letter/digit columns line
  up across rows. There is a blank row below the title and above the thumb row.
  The thumb keys follow their physical stagger: `Os`, `Ctl`, and `Ent` on the
  first line, with `Alt` and `L1` between them on the second. The other half
  mirrors this arrangement. Special keys use mixed-case labels such
  as `Esc`, `Tab`, `Al`, `Ct`, `Sft`, and `Bsp`. The title shows `Layer 0`
  through `Layer 3`. Thumb layer keys show `L1`/`L2` on the base and Adjust
  layers, and both show `L4` on Lower or Raise. While Shift is held, letters
  display uppercase and shifted punctuation is shown (for example `1` becomes
  `!`). Transparent keys show the inherited key from a lower layer.

The build also generates previews of each OLED's compiled keymap automatically
whenever `SOFLE_OLED_KEYMAP=yes` (the default). The 16 PNGs are written to
`.build/oled-preview/` in this userspace: one image per layer, half, and Shift
state. For example, `layer-0-left.png` and `layer-0-right-shift.png` show the
base layer. The images use the actual QMK font and portrait 64x128 display
dimensions, so they can be opened in an image viewer before flashing.

To preview custom OLED text without flashing:

```sh
python3 tools/oled_text_preview.py --text 'L Layer 0\n\n`   12345\nEsc qwert\nTab asdfg\nSft zxcvb\n\nOs Ctl Ent\n  Alt L1' --output /tmp/sofle-oled.png
```

The previewer also accepts `--text-file` for iterative layout work. It uses
`qmk_firmware/drivers/oled/glcdfont.c`, shows the 10x16 character grid, and
wraps text the way QMK does.

Games are drawn on the USB master half's OLED. The keymap screen is drawn
independently on both halves. It shows the keymap compiled into firmware and
does not reflect later edits saved dynamically in Vial. The OLED is rotated
vertically on Sofle Pico, so the game board is laid out in portrait orientation.
Set `SOFLE_OLED_GAMES=no` or `SOFLE_OLED_KEYMAP=no` to exclude either feature.

The implementations use Conway's neighbor-counting rules and a bounded,
grid-based Snake game. The Game of Life was reviewed against the
[GPL-2.0-or-later QMK Conway example by Dakes](https://github.com/Dakes/qmk_config/blob/master/userspace/dakes/conway.c)
and [Krteq's CC0 OLED keyboard project](https://github.com/swift502/Krteq).
I also inspected [Luerl21's QMK OLED Snake example](https://github.com/Luerl21/qmk_synth_wave-old/blob/main/keymaps/via/oled/snake.c);
that repository declares no license, so none of its code was copied. The game
code in this userspace is a new implementation.

## Sofle Pico Vial

Vial is an optional build mode for the same `personal` keymap; it doesn't
require a duplicate keymap. Sofle Pico is already supported by the
[Vial-QMK fork](https://github.com/vial-kb/vial-qmk/tree/vial/keyboards/sofle_pico),
but Vial firmware must be built from that fork, not regular upstream QMK.
Clone the `vial` branch, then build from its checkout:

```sh
git clone --depth 1 --branch vial https://github.com/vial-kb/vial-qmk.git ~/qmk_vial
cd ~/qmk_vial
make git-submodule
qmk compile -kb sofle_pico -km personal -e SOFLE_ENABLE_VIAL=yes -e SOFLE_OLED_GAMES=yes
```

The QMK CLI's `user.overlay_dir` must point to this userspace so the external
`personal` keymap is visible. `SOFLE_ENABLE_VIAL=yes` enables VIA/Vial, the
secure unlock combo, and encoder support for this build only. Its Vial
keyboard definition is in `keyboards/sofle_pico/keymaps/personal/vial.json`.
The command above includes the OLED games by default; pass
`-e SOFLE_OLED_GAMES=no` for a Vial build without them.
