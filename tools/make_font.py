import os
from PIL import ImageFont

def generate_header():
    lines = []
    lines.append('// Auto-generated Fredoka Bold GFX Fonts matching Dasai Mochi theme')
    lines.append('#pragma once')
    lines.append('#include <LovyanGFX.hpp>')
    lines.append('')
    
    fonts_to_gen = [
        ('FredokaBold_48pt', 48),
        ('FredokaBold_16pt', 16)
    ]
    
    for font_name, pt in fonts_to_gen:
        font = ImageFont.truetype('fredoka.ttf', pt)
        ascent, descent = font.getmetrics()
        y_advance = ascent + descent
        
        bitmaps = []
        glyphs = []
        offset = 0
        first = 32
        last = 126
        
        for c in range(first, last + 1):
            ch = chr(c)
            bbox = font.getbbox(ch)
            adv = int(round(font.getlength(ch)))
            
            if bbox is None or bbox[2] <= bbox[0] or bbox[3] <= bbox[1]:
                glyphs.append((offset, 0, 0, adv, 0, 0, c))
                continue
                
            w = bbox[2] - bbox[0]
            h = bbox[3] - bbox[1]
            x_off = bbox[0]
            y_off = bbox[1] - ascent
            
            mask = font.getmask(ch)
            # pack 1-bit MSB first
            cur = 0
            bits = 0
            char_bytes = []
            for y in range(h):
                for x in range(w):
                    val = 1 if mask.getpixel((x, y)) > 80 else 0
                    cur = (cur << 1) | val
                    bits += 1
                    if bits == 8:
                        char_bytes.append(cur)
                        cur = 0
                        bits = 0
            if bits > 0:
                cur <<= (8 - bits)
                char_bytes.append(cur)
                
            glyphs.append((offset, w, h, adv, x_off, y_off, c))
            bitmaps.extend(char_bytes)
            offset += len(char_bytes)
            
        # Write bitmaps
        lines.append(f'const uint8_t {font_name}Bitmaps[] PROGMEM = {{')
        row = []
        for i, b in enumerate(bitmaps):
            row.append(f'0x{b:02X}')
            if len(row) == 16 or i == len(bitmaps) - 1:
                lines.append('  ' + ', '.join(row) + ',')
                row = []
        lines.append('};')
        lines.append('')
        
        # Write glyphs
        lines.append(f'const lgfx::GFXglyph {font_name}Glyphs[] PROGMEM = {{')
        for off, w, h, adv, xo, yo, c in glyphs:
            lines.append(f'  {{ {off:5d}, {w:3d}, {h:3d}, {adv:3d}, {xo:3d}, {yo:3d} }}, // 0x{c:02X}')
        lines.append('};')
        lines.append('')
        
        # Write GFXfont struct
        lines.append(f'const lgfx::GFXfont {font_name} = {{')
        lines.append(f'  (uint8_t*){font_name}Bitmaps,')
        lines.append(f'  (lgfx::GFXglyph*){font_name}Glyphs,')
        lines.append(f'  {first}, {last}, {y_advance}')
        lines.append('};')
        lines.append('')
        
    with open('include/fredoka_font.h', 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))
    print('include/fredoka_font.h generated successfully!')

if __name__ == '__main__':
    generate_header()
