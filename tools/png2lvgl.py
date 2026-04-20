#!/usr/bin/env python3
"""Convert PNG to LVGL 9 lv_image_dsc_t C array (RGB565, little-endian)."""
from PIL import Image
import sys, os

def convert(src, var_name, max_w=160, max_h=160):
    img = Image.open(src).convert('RGB')
    img.thumbnail((max_w, max_h), Image.LANCZOS)
    w, h = img.size

    data = []
    for y in range(h):
        for x in range(w):
            r, g, b = img.getpixel((x, y))
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            data.append(rgb565 & 0xFF)         # low byte first (ARM LE)
            data.append((rgb565 >> 8) & 0xFF)  # high byte

    lines = [
        '#include "lvgl.h"',
        '',
        f'static const uint8_t _{var_name}_map[] = {{',
    ]
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        lines.append('    ' + ', '.join(f'0x{b:02X}' for b in chunk) + ',')
    lines += [
        '};',
        '',
        f'const lv_image_dsc_t {var_name} = {{',
        '    .header = {',
        '        .cf     = LV_COLOR_FORMAT_RGB565,',
        f'        .w      = {w},',
        f'        .h      = {h},',
        f'        .stride = {w * 2},',
        '    },',
        f'    .data_size = {w * h * 2},',
        f'    .data      = _{var_name}_map,',
        '};',
    ]

    out_dir = os.path.dirname(os.path.abspath(src))
    c_path = os.path.join(out_dir, f'{var_name}.c')
    h_path = os.path.join(out_dir, f'{var_name}.h')

    with open(c_path, 'w') as f:
        f.write('\n'.join(lines) + '\n')
    with open(h_path, 'w') as f:
        f.write(f'#pragma once\n#include "lvgl.h"\nextern const lv_image_dsc_t {var_name};\n')

    print(f'OK: {w}x{h} px  →  {c_path}')
    print(f'              →  {h_path}')

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: png2lvgl.py <input.png> <var_name> [max_w] [max_h]')
        sys.exit(1)
    convert(sys.argv[1], sys.argv[2],
            int(sys.argv[3]) if len(sys.argv) > 3 else 160,
            int(sys.argv[4]) if len(sys.argv) > 4 else 160)
