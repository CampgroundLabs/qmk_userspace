# QMK Userspace

Repository: https://github.com/CampgroundLabs/qmk_userspace

This external userspace contains your `personal` keymaps for Sofle Pico and
Corne. The Sofle Pico keymap has a dim-white idle RGB Matrix and an orange
keypress fade.

Configure QMK to use this userspace:

```sh
qmk config user.overlay_dir="$(realpath /path/to/keebs/qmk_userspace)"
```

Then build and flash with:

```sh
qmk compile -kb sofle_pico -km personal
qmk flash -kb sofle_pico -km personal

qmk compile -kb crkbd -km personal
qmk flash -kb crkbd -km personal

qmk compile -kb crkbd -km jcampo_miryoku
qmk flash -kb crkbd -km jcampo_miryoku
```

The Sofle Pico keymap gets RGB defaults from its `config.h`. On its Adjust
layer, `RM_VALU`/`RM_VALD` change brightness and `RM_SPDU`/`RM_SPDD` change
fade speed.

## Sofle Pico OLED games

The Game of Life and Snake are optional in the `personal` keymap. Enable them
by changing `SOFLE_OLED_GAMES ?= no` to `SOFLE_OLED_GAMES = yes` in
`keyboards/sofle_pico/keymaps/personal/rules.mk`, then build as usual. For a
one-off build, pass `-e SOFLE_OLED_GAMES=yes` to `qmk compile`.

```make
SOFLE_OLED_GAMES = yes
```

Hold the Lower and Raise thumb keys together to enter Adjust. On its second
row, use the three keys immediately to the right of `QK_BOOT` to start Life,
start Snake, or exit the current game:

- While Snake is running, use Raise plus the arrow keys to steer.

Only the master half displays the active game; the other OLED keeps the Sofle
logo. The game code is included only when `SOFLE_OLED_GAMES` is enabled.

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
The command above also opts into the OLED games; omit
`-e SOFLE_OLED_GAMES=yes` if you want a Vial build without them.
