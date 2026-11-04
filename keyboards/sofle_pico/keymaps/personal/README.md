# Using Vial with Sofle Pico

This guide covers the `personal` keymap in this userspace. Vial edits the
keymap stored by the keyboard; it does not edit the source files in this
repository.

## Build and install Vial firmware

Vial support must be built from the Vial-QMK fork, not regular upstream QMK.
The userspace needs to be configured as QMK's overlay directory:

```sh
qmk config user.overlay_dir="/path/to/keebs/qmk_userspace"
```

Clone Vial-QMK if needed, initialize its submodules, and build the `personal`
keymap with Vial enabled:

```sh
git clone --depth 1 --branch vial https://github.com/vial-kb/vial-qmk.git ~/qmk_vial
cd ~/qmk_vial
make git-submodule
qmk compile -kb sofle_pico -km personal -e SOFLE_ENABLE_VIAL=yes
```

The build produces `sofle_pico_personal.uf2` in the userspace directory. Flash
that same Vial-enabled UF2 to **both halves**. Put each controller into its
RP2040 bootloader using its BOOT/RST buttons, then copy the UF2 to the
`RPI-RP2` drive. Repeat for the other half. Disconnect USB before inserting or
removing the TRRS cable.

For regular firmware updates, continue to build from the Vial-QMK checkout
with `SOFLE_ENABLE_VIAL=yes`; an upstream QMK build does not include Vial.

## Connect and edit

1. Download and open the [Vial desktop application](https://get.vial.today/).
2. Connect the keyboard over USB. Vial should identify the Sofle Pico and show
   its keymap. If it asks to unlock the keyboard, hold the four configured
   outer-column unlock keys together.
3. Use the keymap view to select a layer and change a key. The `personal`
   layout has four layers: QWERTY, Lower, Raise, and Adjust. Changes are saved
   on the keyboard and survive power loss.
4. Before experimenting or flashing, use Vial's layout save/export action to
   keep a backup. Use its load/import action to restore a saved layout.

The OLED keymap display reads the active keymap, so a Vial-enabled build can
show dynamic key assignments. The generated PNG previews are based on the
compiled source map and do not include later Vial edits.

## How Vial changes relate to firmware

The C keymap in `keymap.c` is the compiled default. When Vial's dynamic
keymap storage is initialized, QMK seeds it from that compiled layout. Once
you edit keys in Vial, the stored layout takes precedence and remains across
ordinary firmware updates; rebuilding firmware alone does not necessarily
replace those saved assignments.

To make a Vial-edited layout reproducible as the firmware default:

1. Save/export a backup from Vial.
2. Copy the desired key assignments into the corresponding `LAYOUT(...)`
   layers in `keymap.c`.
3. Build and flash the updated Vial firmware to both halves.
4. If you want the newly compiled layout to replace the keyboard's saved
   dynamic map, use Vial's EEPROM reset action. This also resets other
   EEPROM-backed settings, so save anything important first.

The compiled OLED preview and source remain useful as a record of the default
layout. Vial stores key assignments, not the OLED renderer, games, or RGB
effect source; those features still come from the firmware build.
