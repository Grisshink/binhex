#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>

#define BUF_CAP 256

typedef enum {
    MODE_UNKNOWN,
    MODE_ENCODE,
    MODE_DECODE,
} Mode;

typedef struct {
    size_t len;
    char* contents;
} String;

String string_new() {
    return (String){
        .len = 0,
        .contents = NULL,
    };
}

void string_push(String* str, char* c_str, size_t len) {
    str->len += len;
    str->contents = str->contents == NULL ? malloc(str->len) : realloc(str->contents, str->len);
    memcpy(str->contents + str->len - len, c_str, len);
}

void string_reset(String* str) {
    free(str->contents);
    str->contents = NULL;
    str->len = 0;
}

char char_to_hex(char hex_val) {
    if (hex_val >= '0' && hex_val <= '9') return hex_val - '0';
    hex_val = toupper(hex_val);
    if (hex_val >= 'A' && hex_val <= 'F') return hex_val - 'A' + 10;

    return 16;
}

int hex_to_uni(char* hex, size_t hexlen, short* uni) {
    for (size_t i = 0; i < hexlen; i += 4) {
        if (i + 4 > hexlen) return 1;

        uni[i / 4] = 0;
        for (int j = 0; j < 4; j++) {
            char hexval = char_to_hex(hex[i + 4-1 - j]);
            if (hexval == 16) return 2;

            uni[i / 4] |= hexval << j * 4;
        }
    }
    return 0;
}

int hex_to_uni_err(char* hex, size_t hexlen, short* uni) {
    int result = hex_to_uni(hex, hexlen, uni);
    if (!result) return 0;

    printf("Invalid input: ");
    if (result == 1) {
        printf("Input is not 2 byte aligned\n");
    } else if (result == 2) {
        printf("Input contains non hex chars\n");
    }
    return 1;
}

int read_to_string(String* input) {
    char buf[BUF_CAP + 1] = { 0 };
    size_t buf_len = 0;

    do {
        if (fgets(buf, BUF_CAP + 1, stdin) == NULL) return 1;
        buf_len = strlen(buf);
        if (buf[buf_len - 1] == '\n') --buf_len;
        string_push(input, buf, buf_len);
    } while(buf[buf_len] != '\n');

    return 0;
}

void encode_hex(String* input, short* key, size_t key_len) {
    string_push(input, "0", 2);

    mbstate_t state;
    memset(&state, 0, sizeof(state));
    wchar_t ch;
    size_t res;
    size_t i = 0, offset = 0;

    printf("> ");
    while ((res = mbrtowc(&ch, input->contents + offset, input->len, &state)) != 0) {
        if (res == (size_t) -2) {
            printf("Error parsing input: not enough length\n");
            return;
        } else if (res == (size_t) -1) {
            printf("Error parsing input: invalid character(s) in input\n");
            return;
        } else if (ch > 0xFFFF) {
            printf("Error parsing input: found non Unicode character(s)\n");
            return;
        }
        printf("%04hX", ((short)ch - 31) ^ key[i++ % key_len]);
        offset += res;
    }
    printf("\n");
}

void decode_hex(String* input, short* key, size_t key_len) {
    size_t input_uni_len = input->len / 4;
    short* input_uni = malloc(input_uni_len * sizeof(short));

    if (hex_to_uni_err(input->contents, input->len, input_uni)) return;
    
    printf("> ");
    for (size_t i = 0; i < input_uni_len - 1; i++) {
        printf("%lc", towlower((input_uni[i] ^ key[i % key_len]) + 31));
    } 
    printf("\n");

    free(input_uni);
}

Mode current_mode = MODE_UNKNOWN;

int main(int argc, char** argv) {
    setlocale(LC_ALL, "");
    if (argc < 3) {
        printf("Error: Not enough arguments\n");
        printf("Usage: binhex enc|dec <HEX_KEY>\n");
        return 1;
    }

    if (!strcmp(argv[1], "enc")) {
        current_mode = MODE_ENCODE;
    } else if (!strcmp(argv[1], "dec")) {
        current_mode = MODE_DECODE;
    } else {
        printf("Invalid argument: Unknown mode \"%s\"\n", argv[1]);
        printf("Usage: binhex enc|dec <HEX_KEY>\n");
        return 1;
    }

    char* key = argv[2];
    size_t key_size = strlen(key);
    short key_uni[key_size / 4];
    if (hex_to_uni_err(key, key_size, key_uni)) return 1;

    String input = string_new();
    while (1) {
        printf("Enter message: ");
        fflush(stdout);
        if (read_to_string(&input)) break;

        switch (current_mode) {
        case MODE_ENCODE:
            encode_hex(&input, key_uni, key_size / 4);
            break;
        case MODE_DECODE:
            decode_hex(&input, key_uni, key_size / 4);
            break;
        default:
            printf("Unreachable\n");
            break;
        }
        string_reset(&input);
    }
    printf("\n");
    return 0;
}
