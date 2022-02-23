# To add a new cell, type '# %%'
# To add a new markdown cell, type '# %% [markdown]'
# %%
from BitVector import *
import time, os


# %%
Sbox = (
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16,
)

InvSbox = (
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D,
)

Mixer = [
    [BitVector(hexstring="02"), BitVector(hexstring="03"), BitVector(hexstring="01"), BitVector(hexstring="01")],
    [BitVector(hexstring="01"), BitVector(hexstring="02"), BitVector(hexstring="03"), BitVector(hexstring="01")],
    [BitVector(hexstring="01"), BitVector(hexstring="01"), BitVector(hexstring="02"), BitVector(hexstring="03")],
    [BitVector(hexstring="03"), BitVector(hexstring="01"), BitVector(hexstring="01"), BitVector(hexstring="02")]
]

InvMixer = [
    [BitVector(hexstring="0E"), BitVector(hexstring="0B"), BitVector(hexstring="0D"), BitVector(hexstring="09")],
    [BitVector(hexstring="09"), BitVector(hexstring="0E"), BitVector(hexstring="0B"), BitVector(hexstring="0D")],
    [BitVector(hexstring="0D"), BitVector(hexstring="09"), BitVector(hexstring="0E"), BitVector(hexstring="0B")],
    [BitVector(hexstring="0B"), BitVector(hexstring="0D"), BitVector(hexstring="09"), BitVector(hexstring="0E")]
]


# %%
AES_modulus = BitVector(bitstring='100011011')


# %%
show_steps = False


# %%
mixer_multiplication = [[[None]*4 for i in range(4)] for j in range(256)]
invmixer_multiplication = [[[None]*4 for i in range(4)] for j in range(256)]
for i in range(256):
    bvi = BitVector(intVal=i,size=8)
    for j in range(4):
        for k in range(4):
            mixer_multiplication[i][j][k] = Mixer[j][k].gf_multiply_modular(bvi, AES_modulus,8)
            invmixer_multiplication[i][j][k] = InvMixer[j][k].gf_multiply_modular(bvi, AES_modulus,8)


# %%
def sub_byte(x):
    return BitVector(intVal=Sbox[x.intValue()],size=8)


# %%
def rot_sub_word(finp):
    x = finp.deep_copy()
    x << 8
    ret = BitVector(size=0)
    for i in range(4):
        ret += sub_byte(x[i*8:(i+1)*8])
    return ret


# %%
def print_matrix(x):
    for i in range(4):
        for j in range(4):
            print(x[i][j].get_bitvector_in_hex(),end=' ')
        print()


# %%
key_ascii = input("Key: ")
# key_ascii = "BUET CSE16 Batch" # "Thats my Kung Fu"
key_ascii = key_ascii.ljust(16, '0')[0:16]
print("Key:")
print(key_ascii)

rc = BitVector(intVal=1,size=8)
w = []
for i in range(4):
    w += [BitVector(textstring=key_ascii[i*4:(i+1)*4])]
for i in range(4,44):
    if i&3 == 0:
        rcon = rc.deep_copy()
        rcon.pad_from_right(24)
        w += [w[i-4] ^ rot_sub_word(w[i-1]) ^ rcon]
        rc = BitVector(intVal=2,size=8).gf_multiply_modular(rc, AES_modulus, 8)
    else:
        w += [w[i-4] ^ w[i-1]]

round_key = []
for i in range(11):
    rk = [[None]*4 for i in range(4)]
    for j in range(i*4,(i+1)*4):
        for k in range(4):
            rk[k][j-i*4] = w[j][k*8:(k+1)*8]
    if show_steps:
        print("round {} key:".format(i))
        print_matrix(rk)
    round_key += [rk]


# %%
def add_matrices(x,y):
    ret = [[None]*4 for i in range(4)]
    for i in range(4):
        for j in range(4):
            ret[i][j] = x[i][j] ^ y[i][j]
    return ret


# %%
def sub_bytes(x):
    x[:] = [[sub_byte(cell) for cell in row] for row in x]  # can't change the reference, must change in place, hence x[:] instead x


# %%
def shift_rows(x):
    for i in range(4):
        x[i] = x[i][i:]+x[i][:i]


# %%
def mix_cols(x):
    ret = [[None]*4 for i in range(4)]
    for i in range(4):
        for j in range(4):
            ret[i][j] = BitVector(size=8)
            for k in range(4):
                ret[i][j] ^= mixer_multiplication[x[k][j].intValue()][i][k]
    return ret


# %%
def ascii_to_matrix(text_ascii):
    assert(len(text_ascii) == 16)
    mat = [[None]*4 for i in range(4)]
    for i in range(16):
        mat[i&3][i>>2] = BitVector(textstring=text_ascii[i])
    return mat


# %%
def matrix_copy(x):
    mat = [[None]*4 for i in range(4)]
    for i in range(4):
        for j in range(4):
            mat[i][j] = x[i][j].deep_copy()
    return mat


# %%
def encrypt_matrix(textmat):
    ciphermat = matrix_copy(textmat)
    for i in range(11):
        if i != 0:
            sub_bytes(ciphermat)
            shift_rows(ciphermat)
            if i != 10:
                ciphermat = mix_cols(ciphermat)
        ciphermat = add_matrices(ciphermat,round_key[i])
        if show_steps:
            print("encryption round {} output:".format(i))
            print_matrix(ciphermat)
            print()
    return ciphermat


# %%
def invsub_byte(x):
    return BitVector(intVal=InvSbox[x.intValue()],size=8)


# %%
def invsub_bytes(x):
    x[:] = [[invsub_byte(cell) for cell in row] for row in x]  # can't change the reference, must change in place, hence x[:] instead x


# %%
def invshift_rows(x):
    for i in range(4):
        x[i] = x[i][4-i:]+x[i][:4-i]


# %%
def invmix_cols(x):
    ret = [[None]*4 for i in range(4)]
    for i in range(4):
        for j in range(4):
            ret[i][j] = BitVector(size=8)
            for k in range(4):
                ret[i][j] ^= invmixer_multiplication[x[k][j].intValue()][i][k]
    return ret


# %%
def matrix_to_ascii(mat):
    ret = ""
    for i in range(16):
        ret += mat[i&3][i>>2].get_bitvector_in_ascii()
    return ret


# %%
def decrypt_matrix(ciphermat):
    deciphermat = matrix_copy(ciphermat)
    for i in range(11):
        if i != 0:
            invshift_rows(deciphermat)
            invsub_bytes(deciphermat)
        deciphermat = add_matrices(deciphermat,round_key[10-i])
        if i != 0 and i != 10:
            deciphermat = invmix_cols(deciphermat)
        if show_steps:
            print("decryption round {} output:".format(i))
            print_matrix(deciphermat)
            print()
    return deciphermat


# %%
def encrypt_ascii(ascii_large):
    l = len(ascii_large)
    ascii_large += "".rjust(16, ' ')
    ret = []
    for i in range(0,l,16):
        ret += [encrypt_matrix(ascii_to_matrix(ascii_large[i:i+16]))]
    return ret


# %%
def decrypt_ciphers(ciphers):
    ret = ""
    for cipher in ciphers:
        ret += matrix_to_ascii(decrypt_matrix(cipher))
    return ret


# %%
text_ascii = input("Plain text: ")
# text_ascii = "WillGraduateSoon" # "Two One Nine Two"
print("Plain Text:")
print(text_ascii)
start_time = time.process_time()
ciphers = encrypt_ascii(text_ascii)
end_time = time.process_time()
print("Cipher text:")
for cipher in ciphers:
    print_matrix(cipher)
print("Encryption time: {}".format(end_time - start_time))
print("Deciphered text:")
start_time = time.process_time()
print(decrypt_ciphers(ciphers))
end_time = time.process_time()
print("Decryption time: {}".format(end_time - start_time))


# %%
def bytes_from_file(filename, chunksize=8192):
    with open(filename, "rb") as f:
        while True:
            chunk = f.read(chunksize)
            if chunk:
                yield from chunk
            else:
                break


# %%
inpfile = input("Enter filepath: ")
# inpfile = "../sample.pdf"
filename, file_extension = os.path.splitext(inpfile)
input_byteara = None
with open(inpfile, "rb") as f:
    input_byteara = bytearray(f.read())
print(len(input_byteara))


# %%
def matrix_to_bytearray(x):
    ret = bytearray()
    for j in range(4):
        for i in range(4):
            ret.append(x[i][j].intValue())
    return ret


# %%
def encrypt_byteara(x):
    ret = bytearray()
    mat = [[None]*4 for i in range(4)]
    curbyte = 0
    for b in x:
        mat[curbyte&3][curbyte>>2] = BitVector(intVal=b)
        if curbyte == 15:
            ret += matrix_to_bytearray(encrypt_matrix(mat))
            curbyte = 0
        else:
            curbyte += 1
    if curbyte != 0:
        ret += matrix_to_bytearray(encrypt_matrix(mat))
    ret += len(x).to_bytes(16, byteorder='big')
    return ret


# %%
encrypted_byteara = encrypt_byteara(input_byteara)
with open("encrypted.bin", "wb") as f:
    f.write(encrypted_byteara)
print(len(encrypted_byteara))


# %%
def decrypt_byteara(x):
    filesize = int.from_bytes(x[-16:], byteorder='big')
    ret = bytearray()
    mat = [[None]*4 for i in range(4)]
    curbyte = 0
    for b in x:
        mat[curbyte&3][curbyte>>2] = BitVector(intVal=b)
        if curbyte == 15:
            ret += matrix_to_bytearray(decrypt_matrix(mat))
            curbyte = 0
        else:
            curbyte += 1
    assert(curbyte == 0)
    print(filesize)
    ret = ret[:filesize]
    return ret

decrypted_byteara = decrypt_byteara(encrypted_byteara)


# %%
assert(input_byteara == decrypted_byteara)
with open("decrypted"+file_extension, "wb") as f:
    f.write(decrypted_byteara)


# %%
sbox = [None]*256
invsbox = [None]*256
for i in range(256):
    sbox[i] = BitVector(intVal=i, size=8)
    if i != 0:
        sbox[i] = sbox[i].gf_MI(AES_modulus, 8)
    sbox[i] = sbox[i].gf_multiply_modular(BitVector(intVal=31, size=8), BitVector(intVal=257, size=9), 8) ^ BitVector(intVal=99, size=8)
    invsbox[sbox[i].intValue()] = BitVector(intVal=i,size=8)

print("\nSbox:")
for i in range(256):
    assert(sbox[i].intValue() == Sbox[i])
    print(sbox[i].get_bitvector_in_hex(),end='\n' if i%16 == 15 else ' ')
print("\nInvSbox:")
for i in range(256):
    assert(invsbox[i].intValue() == InvSbox[i])
    print(invsbox[i].get_bitvector_in_hex(),end='\n' if i%16 == 15 else ' ')

