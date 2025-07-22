#!/usr/bin/env python3
# generate-diceware-dict.py

def main():
    input_file = 'eff-large-wordlist.txt'
    output_header = 'diceware-dict.h'
    output_source = 'diceware-dict.c'

    # Read words
    with open(input_file, 'r') as f:
        words = [line.strip().split()[1] for line in f if line.strip()]

    # Prepare offsets
    offsets = []
    pos = 0
    for w in words:
        offsets.append(pos)
        pos += len(w) + 1  # +1 for '\0'

    # Write header file
    with open(output_header, 'w') as f:
        f.write('#pragma once\n\n')
        f.write('#include <stddef.h>\n\n')
        f.write(f'#define DICEWARE_DICT_SIZE {len(words)}\n\n')
        f.write('extern const char diceware_words_data[];\n')
        f.write('extern const size_t diceware_words_offsets[];\n\n')
        f.write('static inline const char* diceware_dict(size_t idx) {\n')
        f.write('    if (idx >= DICEWARE_DICT_SIZE) return NULL;\n')
        f.write('    return diceware_words_data + diceware_words_offsets[idx];\n')
        f.write('}\n')

    # Write source file
    with open(output_source, 'w') as f:
        f.write('#include "diceware_dict.h"\n\n')

        # Write concatenated string (chunked for readability)
        f.write('const char diceware_words_data[] =\n')
        line_len = 0
        f.write('    "')
        for i, w in enumerate(words):
            # escape any quotes or backslashes just in case
            w_escaped = w.replace('\\', '\\\\').replace('"', '\\"')
            f.write(w_escaped + '\\0')
            line_len += len(w) + 1
            if line_len > 60:
                f.write('"\n    "')
                line_len = 0
        f.write('";\n\n')

        # Write offsets array
        f.write('const size_t diceware_words_offsets[] = {\n    ')
        for i, off in enumerate(offsets):
            f.write(f'{off}, ')
            if (i + 1) % 10 == 0:
                f.write('\n    ')
        f.write('\n};\n')

if __name__ == '__main__':
    main()

