#!/usr/bin/env python3
"""Preview QMK's standard OLED text output using the actual glcdfont bitmap."""

import argparse
import re
import struct
import zlib
from pathlib import Path


DISPLAY_WIDTH = 64
DISPLAY_HEIGHT = 128
FONT_WIDTH = 6
FONT_HEIGHT = 8
FONT_PATH = Path(__file__).resolve().parents[2] / "qmk_firmware/drivers/oled/glcdfont.c"
LAYERS = ("QWERTY", "LOWER", "RAISE", "ADJUST")


def load_font():
    source = FONT_PATH.read_text()
    pattern = re.compile(r"((?:0x[0-9A-Fa-f]{2},\s*){6})//\s*0x([0-9A-Fa-f]{2})")
    glyphs = {}
    for match in pattern.finditer(source):
        code = int(match.group(2), 16)
        if code < 128:
            glyphs[code] = [int(value, 16) for value in re.findall(r"0x([0-9A-Fa-f]{2})", match.group(1))]
    if len(glyphs) != 128:
        raise RuntimeError(f"Expected 128 ASCII glyphs in {FONT_PATH}, found {len(glyphs)}")
    return glyphs


def render_text(text, glyphs):
    pixels = bytearray(DISPLAY_WIDTH * DISPLAY_HEIGHT)
    col = 0
    row = 0

    def advance_line():
        nonlocal col, row
        col = 0
        row += 1
        if row >= DISPLAY_HEIGHT // FONT_HEIGHT:
            row = 0

    for character in text:
        if character == "\r":
            col = 0
            continue
        if character == "\n":
            advance_line()
            continue

        code = ord(character)
        if code > 127:
            raise ValueError(f"Non-ASCII character {character!r} is not in QMK's standard font")
        if col >= DISPLAY_WIDTH // FONT_WIDTH:
            advance_line()

        glyph = glyphs[code]
        for x, column_bits in enumerate(glyph):
            for y in range(FONT_HEIGHT):
                if column_bits & (1 << y):
                    px = col * FONT_WIDTH + x
                    py = row * FONT_HEIGHT + y
                    pixels[py * DISPLAY_WIDTH + px] = 1
        col += 1

    return pixels


def png_chunk(chunk_type, data):
    return struct.pack(">I", len(data)) + chunk_type + data + struct.pack(">I", zlib.crc32(chunk_type + data) & 0xFFFFFFFF)


def write_png(path, pixels, scale):
    width = DISPLAY_WIDTH * scale
    height = DISPLAY_HEIGHT * scale
    scanlines = bytearray()
    for y in range(height):
        scanlines.append(0)
        source_y = y // scale
        row_start = source_y * DISPLAY_WIDTH
        for x in range(width):
            color = 255 if pixels[row_start + x // scale] else 0
            scanlines.extend((color, color, color))

    png = bytearray(b"\x89PNG\r\n\x1a\n")
    png.extend(png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)))
    png.extend(png_chunk(b"IDAT", zlib.compress(bytes(scanlines))))
    png.extend(png_chunk(b"IEND", b""))
    path.write_bytes(png)


def split_c_arguments(source):
    arguments = []
    start = 0
    depth = 0
    for index, character in enumerate(source):
        if character == "(":
            depth += 1
        elif character == ")":
            depth -= 1
        elif character == "," and depth == 0:
            arguments.append(source[start:index].strip())
            start = index + 1
    final = source[start:].strip()
    if final:
        arguments.append(final)
    return arguments


def matching_parenthesis(source, opening_index):
    depth = 0
    for index in range(opening_index, len(source)):
        if source[index] == "(":
            depth += 1
        elif source[index] == ")":
            depth -= 1
            if depth == 0:
                return index
    raise ValueError("Unclosed parenthesis while reading keymap source")


def enabled_preprocessor_lines(source, enabled_defines):
    output = []
    stack = []
    active = True
    for line in source.splitlines():
        directive = re.match(r"\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(?:\s+(.*?))?\s*$", line)
        if not directive:
            if active:
                output.append(line.split("//", 1)[0])
            continue
        command, expression = directive.groups()

        def evaluate(condition_expression):
            if command in ("ifdef", "ifndef"):
                result = condition_expression.strip() in enabled_defines
                return result if command == "ifdef" else not result
            expression_text = re.sub(
                r"defined\s*\(\s*(\w+)\s*\)",
                lambda match: "True" if match.group(1) in enabled_defines else "False",
                condition_expression or "",
            )
            expression_text = re.sub(
                r"defined\s+(\w+)",
                lambda match: "True" if match.group(1) in enabled_defines else "False",
                expression_text,
            )
            expression_text = expression_text.replace("||", " or ").replace("&&", " and ")
            expression_text = re.sub(r"!(?!=)", " not ", expression_text)
            if not re.fullmatch(r"[\w\s()]+", expression_text):
                raise ValueError(f"Unsupported preprocessor condition: {condition_expression}")
            return bool(eval(expression_text, {"__builtins__": {}}, {}))

        if command in ("ifdef", "ifndef", "if"):
            condition = evaluate(expression)
            stack.append([active, condition])
            active = active and condition
        elif command == "elif":
            if not stack:
                raise ValueError("Unmatched #elif in keymap source")
            parent_active, branch_taken = stack[-1]
            condition = False if branch_taken else evaluate(expression)
            active = parent_active and condition
            stack[-1][1] = branch_taken or condition
        elif command == "else":
            if not stack:
                raise ValueError("Unmatched #else in keymap source")
            parent_active, branch_taken = stack[-1]
            active = parent_active and not branch_taken
            stack[-1][1] = True
        else:
            if not stack:
                raise ValueError("Unmatched #endif in keymap source")
            parent_active, _ = stack.pop()
            active = parent_active
    if stack:
        raise ValueError("Unclosed #if directive in keymap source")
    return "\n".join(output)


def parse_layout_matrix(layout_header):
    source = layout_header.read_text()
    match = re.search(r"#define\s+LAYOUT\s*\(", source)
    if not match:
        raise ValueError(f"Could not find LAYOUT macro in {layout_header}")
    params_open = source.index("(", match.start())
    params_close = matching_parenthesis(source, params_open)
    parameters = split_c_arguments(source[params_open + 1 : params_close])
    body = source[params_close + 1 :]
    macro_body = []
    for line in body.splitlines():
        macro_body.append(line.split("//", 1)[0].rstrip().rstrip("\\").strip())
        if not line.rstrip().endswith("\\"):
            break
    rows = re.findall(r"\{([^{}]+)\}", "\n".join(macro_body))
    mapping = {}
    for row_index, row_text in enumerate(rows):
        for col_index, parameter in enumerate(split_c_arguments(row_text)):
            if parameter not in parameters:
                raise ValueError(f"Unknown LAYOUT parameter {parameter!r} in matrix row")
            mapping[(row_index, col_index)] = parameters.index(parameter)
    return mapping


def parse_keymap(keymap_path, layout_header, enabled_defines):
    source = enabled_preprocessor_lines(keymap_path.read_text(), enabled_defines)
    layout_mapping = parse_layout_matrix(layout_header)
    layers = {}
    cursor = 0
    layer_pattern = re.compile(r"\[_([A-Z]+)\]\s*=\s*LAYOUT\s*\(")
    while match := layer_pattern.search(source, cursor):
        opening = source.index("(", match.start())
        closing = matching_parenthesis(source, opening)
        arguments = split_c_arguments(source[opening + 1 : closing])
        matrix = [["KC_NO" for _ in range(6)] for _ in range(10)]
        for (row, col), argument_index in layout_mapping.items():
            if argument_index >= len(arguments):
                raise ValueError(f"LAYOUT has too few arguments in layer {match.group(1)}")
            matrix[row][col] = arguments[argument_index].strip()
        layers[match.group(1)] = matrix
        cursor = closing + 1
    if not layers:
        raise ValueError(f"No LAYOUT layers found in {keymap_path}")
    return layers


def key_label(keycode, layer_index, shifted, full=False, outer=False):
    if shifted:
        keycode = {
            "KC_BSPC": "KC_DEL",
            "KC_SLSH": "KC_PIPE",
        }.get(keycode, keycode)

    if keycode in ("_______", "KC_TRNS"):
        return "..."
    if keycode in ("XXXXXXX", "KC_NO"):
        return "---"
    if keycode in ("TL_LOWR", "TL_UPPR"):
        return "L4" if layer_index in (1, 2) else ("L2" if keycode == "TL_LOWR" else "L3")

    match = re.fullmatch(r"C\((KC_[A-Z0-9]+)\)", keycode)
    if match:
        return f"C-{match.group(1)[3:]}" if full else match.group(1)[-1].lower()
    if keycode.startswith("KC_") and len(keycode) == 4 and keycode[-1].isalpha():
        letter = keycode[-1]
        return letter if shifted else letter.lower()
    if re.fullmatch(r"KC_[0-9]", keycode):
        digit = keycode[-1]
        return "!@#$%^&*()"[int(digit) - 1] if shifted and digit != "0" else (")" if shifted else "0") if digit == "0" else digit

    aliases = {
        "KC_LCTL": "Ctl", "KC_RCTL": "Ctl", "KC_LALT": "Alt", "KC_RALT": "Alt",
        "KC_LGUI": "Gui", "KC_RGUI": "Gui", "KC_ENT": "Ent", "KC_SPC": "Spc",
        "KC_BSPC": "Bsp", "KC_TAB": "Tab", "KC_ESC": "Esc", "KC_LSFT": "Sft",
        "KC_RSFT": "Sft", "KC_UP": "Up", "KC_DOWN": "Dn", "KC_LEFT": "Lt",
        "KC_RIGHT": "Rt", "KC_INS": "Ins", "KC_PSCR": "Psc", "KC_APP": "App",
        "KC_PGUP": "Pup", "KC_PGDN": "Pdn", "KC_HOME": "Hom", "KC_END": "End",
        "KC_DEL": "Del", "KC_CAPS": "Cap", "KC_RGHT": "Rt",
        "QK_BOOT": "Bot", "CG_TOGG": "Cap", "RM_TOGG": "Rgb", "RM_VALU": "V+",
        "RM_VALD": "V-", "RM_SPDU": "S+", "RM_SPDD": "S-", "RM_NEXT": "Nxt",
        "RM_PREV": "Prv", "KC_VOLD": "V-", "KC_VOLU": "V+", "KC_MUTE": "Mut",
        "KC_MPRV": "Prv", "KC_MPLY": "Ply", "KC_MNXT": "Nxt",
        "KC_EXLM": "!", "KC_AT": "@", "KC_HASH": "#", "KC_DLR": "$",
        "KC_PERC": "%", "KC_CIRC": "^", "KC_AMPR": "&", "KC_ASTR": "*",
        "KC_LPRN": "(", "KC_RPRN": ")", "KC_PIPE": "|", "KC_PLUS": "+",
        "KC_LCBR": "{", "KC_RCBR": "}", "KC_COLN": ":",
        "KC_TILD": "~", "KC_UNDS": "_",
        "KC_GRV": "~" if shifted else "`", "KC_MINS": "_" if shifted else "-",
        "KC_EQL": "+" if shifted else "=", "KC_LBRC": "{" if shifted else "[",
        "KC_RBRC": "}" if shifted else "]", "KC_BSLS": "|" if shifted else "\\",
        "KC_SCLN": ":" if shifted else ";", "KC_QUOT": '"' if shifted else "'",
        "KC_COMM": "<" if shifted else ",", "KC_DOT": ">" if shifted else ".",
        "KC_SLSH": "?" if shifted else "/",
    }
    if keycode in aliases:
        value = aliases[keycode]
        if full and len(value) > 1:
            return value[:3]
        if outer and len(value) > 1:
            return value[:3]
        return value[0]
    if keycode.startswith("KC_F"):
        return keycode[3:6].title()
    if keycode.startswith("OLED_GAME_"):
        return {"OLED_GAME_LIFE": "Lif", "OLED_GAME_SNAKE": "Snk", "OLED_GAME_EXIT": "Ext"}.get(keycode, "???")
    if keycode == "OLED_KEYMAP_TOGGLE":
        return "Map"
    return "?"


def effective_keycode(layers, layer_name, row, col):
    fallback_order = {
        "LOWER": ("LOWER", "QWERTY"),
        "RAISE": ("RAISE", "QWERTY"),
        "ADJUST": ("ADJUST", "RAISE", "LOWER", "QWERTY"),
    }.get(layer_name, (layer_name,))
    for candidate in fallback_order:
        if candidate not in layers:
            continue
        code = layers[candidate][row][col]
        if code not in ("_______", "KC_TRNS"):
            return code
    return layers.get(layer_name, [["KC_NO"]])[row][col]


def thumb_key_label(keycode, layer_index, shifted):
    if keycode in ("KC_LGUI", "KC_RGUI"):
        return "Cmd"
    if keycode in ("KC_LCTL", "KC_RCTL"):
        return "Ct"
    if keycode == "TL_LOWR":
        return "L4" if layer_index in (1, 2) else "L1"
    if keycode == "TL_UPPR":
        return "L4" if layer_index in (1, 2) else "L2"
    return key_label(keycode, layer_index, shifted, full=True)


def place_labels(labels, positions):
    row = [" "] * 10
    for label, position in zip(labels, positions):
        if position + len(label) > len(row):
            raise ValueError(f"Thumb label {label!r} does not fit on the OLED row")
        row[position : position + len(label)] = label
    return "".join(row).rstrip()


def render_keymap_previews(keymap_path, layout_header, output_dir, enabled_defines, scale):
    layers = parse_keymap(keymap_path, layout_header, enabled_defines)
    glyphs = load_font()
    output_dir.mkdir(parents=True, exist_ok=True)
    render_count = 0

    for layer_index, layer_name in enumerate(LAYERS):
        if layer_name not in layers:
            continue
        for side, is_left in (("left", True), ("right", False)):
            for shifted in (False, True):
                lines = [f"{'L' if is_left else 'R'} Layer {layer_index}", ""]
                row_offset = 0 if is_left else 5
                for row in range(4):
                    if is_left:
                        outer = key_label(effective_keycode(layers, layer_name, row_offset + row, 0), layer_index, shifted, outer=True)
                        inner = "".join(
                            key_label(effective_keycode(layers, layer_name, row_offset + row, col), layer_index, shifted)
                            for col in range(1, 6)
                        )
                        lines.append(f"{outer:3} {inner}")
                    else:
                        inner = "".join(
                            key_label(effective_keycode(layers, layer_name, row_offset + row, col), layer_index, shifted)
                            for col in range(5, 0, -1)
                        )
                        outer = key_label(effective_keycode(layers, layer_name, row_offset + row, 0), layer_index, shifted, outer=True)
                        lines.append(f"{inner} {outer:>3}")

                thumb_row = 4 if is_left else 9
                first_line_cols = (0, 2, 4) if is_left else (4, 2, 0)
                second_line_cols = (1, 3) if is_left else (3, 1)
                first_line_labels = [
                    thumb_key_label(effective_keycode(layers, layer_name, thumb_row, col), layer_index, shifted)
                    for col in first_line_cols
                ]
                second_line_labels = [
                    thumb_key_label(effective_keycode(layers, layer_name, thumb_row, col), layer_index, shifted)
                    for col in second_line_cols
                ]
                first_line_positions = (0, 4, 7)
                lines.extend((
                    "",
                    place_labels(first_line_labels, first_line_positions),
                    place_labels(second_line_labels, (2, 6)),
                ))

                text = "\n".join(lines)
                pixels = render_text(text, glyphs)
                path = output_dir / f"layer-{layer_index}-{side}{'-shift' if shifted else ''}.png"
                write_png(path, pixels, scale)
                render_count += 1
    print(f"Rendered {render_count} OLED previews in {output_dir}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group()
    source.add_argument("--text", help="Text to display; use literal \\n between OLED rows")
    source.add_argument("--text-file", type=Path, help="UTF-8 file containing the display text")
    parser.add_argument("--keymap", type=Path, help="Render all layers from a QMK keymap.c")
    parser.add_argument("--layout-header", type=Path, help="Generated header defining the QMK LAYOUT macro")
    parser.add_argument("--output-dir", type=Path, help="Directory for all keymap preview PNGs")
    parser.add_argument("--games", choices=("yes", "no"), default="yes", help="Whether game keycodes are enabled")
    parser.add_argument("--output", type=Path, default=Path("/tmp/sofle-oled-preview.png"))
    parser.add_argument("--scale", type=int, default=4, help="Integer pixel scale for the PNG (default: 4)")
    args = parser.parse_args()

    if args.scale < 1:
        parser.error("--scale must be at least 1")
    if args.keymap:
        if not args.layout_header or not args.output_dir:
            parser.error("--keymap requires --layout-header and --output-dir")
        enabled_defines = {"SOFLE_OLED_KEYMAP"}
        if args.games == "yes":
            enabled_defines.add("SOFLE_OLED_GAMES")
        render_keymap_previews(args.keymap, args.layout_header, args.output_dir, enabled_defines, args.scale)
        return
    if bool(args.text) == bool(args.text_file):
        parser.error("provide exactly one of --text, --text-file, or --keymap")
    text = args.text.replace("\\n", "\n") if args.text is not None else args.text_file.read_text()
    pixels = render_text(text, load_font())
    write_png(args.output, pixels, args.scale)
    print(f"Saved {DISPLAY_WIDTH}x{DISPLAY_HEIGHT} OLED preview to {args.output}")
    print(f"Character grid: {DISPLAY_WIDTH // FONT_WIDTH} columns x {DISPLAY_HEIGHT // FONT_HEIGHT} rows")
    print("Newlines advance one display row; text wraps at the OLED's 10-character limit.")
    for row_number, row_text in enumerate(text.splitlines()):
        if len(row_text) > DISPLAY_WIDTH // FONT_WIDTH:
            print(f"Warning: input row {row_number + 1} is {len(row_text)} characters and will wrap")


if __name__ == "__main__":
    main()
