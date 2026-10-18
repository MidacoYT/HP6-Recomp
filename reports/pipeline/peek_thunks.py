import struct

EXE = r'D:\HP\hp6ss\build\sdk-lab\pc\hp6.exe'
data = open(EXE, 'rb').read()
pe = struct.unpack_from('<I', data, 0x3c)[0]
nsec = struct.unpack_from('<H', data, pe + 6)[0]
optsize = struct.unpack_from('<H', data, pe + 20)[0]
imagebase = struct.unpack_from('<I', data, pe + 24 + 28)[0]
stable = pe + 24 + optsize
sections = []
for i in range(nsec):
    h = stable + i * 40
    vsz, vaddr, rawsz, rawaddr, char = struct.unpack_from('<IIIII', data, h + 8)
    sections.append((vaddr, vsz, rawaddr, rawsz))

def va_to_file(va):
    rva = va - imagebase
    for vaddr, vsz, rawaddr, rawsz in sections:
        if vaddr <= rva < vaddr + max(vsz, rawsz):
            return rawaddr + (rva - vaddr)
    return None

# Verify with the known strdup thunk bytes: 006b92bc PUSH EBP.
chk = va_to_file(0x006b92bc)
print('check 6b92bc:', data[chk:chk + 7].hex())

def dump(start, length):
    off = va_to_file(start)
    chunk = data[off:off + length]
    print(f'---- VA {start:08x} (file {off:06x})')
    for i in range(0, len(chunk), 16):
        va = start + i
        print(f'{va:08x}: ' + ' '.join(f'{b:02x}' for b in chunk[i:i + 16]))

from disas import *  if False else None
dump(0x006b4530, 0xa0)
dump(0x006b54a0, 0x30)
