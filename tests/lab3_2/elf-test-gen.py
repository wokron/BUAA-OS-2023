#!/usr/bin/python3

import sys
from posixpath import basename, splitext
from elftools.elf.elffile import ELFFile


if __name__ == '__main__':
    file = sys.argv[1]
    case = splitext(basename(file))[0]

    segs = []
    with open(file, 'rb') as fp:
        elf = ELFFile(fp)
        for seg in elf.iter_segments():
            h = seg.header
            if h.p_type == 'PT_LOAD':
                data = seg.data()
                segs.append((h.p_vaddr, data, h))

    for i, (va, data, _) in enumerate(segs):
        print(f'const char {case}_{va:x}[] = {{')
        for b in data:
            print(f'0x{b:x},', end='', sep='')
        print('\n};\n')

    print(f'''void load_icode_check() {{
    printk("testing load_icode for {case}\\n");
    struct Env *e = ENV_CREATE(test_{case});\
''')

    for i, (va, data, h) in enumerate(segs):
        n = len(data)
        std = f'{case}_{va:x}'
        print(f'''    // Segment at 0x{va:x}, memsz={h.p_memsz}, filesz={h.p_filesz}
    seg_check(e->env_pgdir, 0x{va:x}, {std}, sizeof {std});''')
        if h.p_memsz != n:
            print(f'    seg_check(e->env_pgdir, 0x{va + n:x}, NULL, {h.p_memsz - n});')
    print(f'''    printk("load_icode test for {case} passed!\\n");
}}''')
