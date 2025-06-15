import struct

def display(f):
    print('f', f)
    u32 = struct.unpack('!I', struct.pack('!f', f))[0]
    print('bin: ', bin(u32))
    print('hex: ', hex(u32))
    print('s', (u32 >> 31) & 1,
          'e', (u32 >> 23) & 0xFF,
          'm', bin(u32 & 0x7FFFFF), hex(u32 & 0x7FFFFF))


def to_bin(f):
    sign = 1 if f < 0 else 0
    int_part = int(abs(f))
    float_part = f - int_part

    ihex = []
    while 1:
        lsb = int_part % 2
        ihex.insert(0, lsb)

        int_part //= 2
        if int_part == 0:
            break

    fhex = []
    for _ in range(23 - len(ihex) + 1):
        float_part *= 2
        msb = int(float_part)
        float_part -= msb
        fhex.append(msb)
    return sign, ihex, fhex


def to_ieee754(sign, ihex, fhex):
    assert len(ihex) > 0
    assert len(fhex) > 0
    assert ihex[0] == 1  # normalized float

    bias = 127
    exp = len(ihex) - 1 + bias
    mantissa = (ihex[1:] + fhex)[:23]
    return sign, exp, mantissa


def mantissa_to_hex(mantissa):
    # 补齐到4的倍数
    bits = [0] * ((4 - len(mantissa) % 4) % 4) + mantissa
    hex_str = ''
    for i in range(0, len(bits), 4):
        group = bits[i:i+4]
        value = 0
        for b in group:
            value = (value << 1) | b
        hex_str += format(value, 'x')
    return hex_str


def test():
    sign, ihex, fhex = to_bin(3.14)
    print(sign, ihex, '.', fhex)
    sign, exp, mantissa = to_ieee754(sign, ihex, fhex)
    print('s', sign, 'e', exp, 'm', ''.join(map(str, mantissa)))
    mantissa_hex = mantissa_to_hex(mantissa)
    print('mantissa hex:', mantissa_hex)
    display(3.14)


if __name__ == "__main__":
    test()