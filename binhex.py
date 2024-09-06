from itertools import cycle, batched
def decrypt_hex(msg, key):
    def strhex(inp): return (int(''.join(block), 16) for block in batched(inp, 4))
    dec = (left ^ right for left, right in zip(strhex(msg), cycle(strhex(key))))
    return ''.join(chr(block + 31) for block in dec)[:-1]

def encrypt_hex(msg, key):
    enc = ((ord(char) - 31) ^ int(''.join(block), 16) for char, block in zip(msg + '0', cycle(batched(key, 4))))
    return ''.join(f'{hex(val)[2:]:>04}' for val in enc)

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 3: 
        print('Error: Not enough arguments\n'
              'Usage: binhex.py Enc|Dec <HEX_KEY>')
    else:
        while True:
            print('> %s' % (decrypt_hex(input('Enter message: '), sys.argv[2]).lower()
                           if sys.argv[1] == 'Dec' else
                           encrypt_hex(input('Enter message: '), sys.argv[2]).upper()))
