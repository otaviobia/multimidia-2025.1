#ifndef HUFFMAN_H
    #define HUFFMAN_H

    #include <stdio.h>
    #include <stdint.h>
    #include "codec.h"

    // Tamanho máximo do código Huffman no padrão JPEG (16 bits)
    #define MAX_HUFFMAN_CODE_LENGTH 16
    
    // Estrutura para cabeçalho de arquivo BMP
    typedef struct {
        BITMAPFILEHEADER file_header;
        BITMAPINFOHEADER info_header;
        float quality;
        uint32_t macroblock_count;
    } COMPRESSED_HEADER;

    // Estrutura para entrada da tabela Huffman
    typedef struct {
        char code[MAX_HUFFMAN_CODE_LENGTH + 1]; // Código binário como string
        int code_length;                        // Comprimento do código em bits
        int code_value;                    // Valor do código como inteiro
    } HuffmanEntry;

    // Estrutura para estatísticas de símbolos
    typedef struct {
        int symbol;
        unsigned int frequency;
    } SymbolFrequency;

    // Estrutura para buffer de bits
    typedef struct {
        uint8_t *data;               // Buffer de dados
        size_t capacity;             // Capacidade total em bytes
        size_t byte_position;        // Posição atual em bytes
        int bit_position;            // Posição atual em bits (0-7)
    } BitBuffer;

    // Funções de manipulação de buffer
    BitBuffer* init_bit_buffer(size_t initial_capacity);
    void free_bit_buffer(BitBuffer* buffer);
    int write_bits(BitBuffer* buffer, int value, int num_bits);
    int read_bits(BitBuffer* buffer, int num_bits);
    size_t get_huffman_buffer_size(BitBuffer* buffer);

    // Funções de codificação Huffman
    int write_dc_coefficient(BitBuffer* buffer, int dc_diff);
    int write_ac_coefficient(BitBuffer* buffer, int run_length, int ac_value);
    int huffman_encode_block(BitBuffer* buffer, BLOCO_RLE_DIFERENCIAL* block);
    BitBuffer* huffman_encode_macroblock(MACROBLOCO_RLE_DIFERENCIAL* macroblock);

    // Funções de decodificação Huffman
    int decode_dc_huffman(BitBuffer* buffer);
    int decode_dc_coefficient(int* result_val, BitBuffer* buffer);
    int decode_ac_huffman(BitBuffer* buffer, int* run_length, int* category);
    int decode_ac_coefficient(BitBuffer* buffer, int* run_length, int* value);
    int huffman_decode_block(BitBuffer* buffer, BLOCO_RLE_DIFERENCIAL* block);
    MACROBLOCO_RLE_DIFERENCIAL* huffman_decode_macroblock(BitBuffer* buffer);

    // Funções de leitura e escrita de macroblocos
    void write_macroblocks_huffman(const char *output_filename, MACROBLOCO_RLE_DIFERENCIAL *rle_macroblocks, int macroblock_count, BITMAPFILEHEADER file_header, BITMAPINFOHEADER info_header, float quality);
    int read_macroblocks_huffman(const char *input_filename, MACROBLOCO_RLE_DIFERENCIAL **blocos_lidos, int *count_lido, BITMAPFILEHEADER *fhead, BITMAPINFOHEADER *ihead, float *quality_lida);
    int huffman_decode_macroblock_to_dest(BitBuffer* buffer, MACROBLOCO_RLE_DIFERENCIAL* dest_macroblock);

    // Tabela DC - Fornecida
    static const HuffmanEntry JPEG_DC_LUMINANCE_TABLE[11] = {
        // binario  | comprimento | valor(binario em hexadecimal)
        // Qtd. bits da mantissa é o mesmo da categoria
        // (linha) - (categoria)

        {"010", 3, 0x2},           // Categoria 0
        {"011", 3, 0x3},           // Categoria 1
        {"100", 3, 0x4},           // Categoria 2
        {"00", 2, 0x0},            // Categoria 3
        {"101", 3, 0x5},           // Categoria 4
        {"110", 3, 0x6},           // Categoria 5
        {"1110", 4, 0xE},         // Categoria 6
        {"11110", 5, 0x1E},       // Categoria 7
        {"111110", 6, 0x3E},      // Categoria 8
        {"1111110", 7, 0x7E},     // Categoria 9
        {"11111110", 8, 0xFE}     // Categoria A
    };
    
    // Tabela AC - Fornecida
    static const HuffmanEntry JPEG_AC_LUMINANCE_MATRIX[16][11] = {
        // binario  | comprimento | valor(binario em hexadecimal)
        // (linha, coluna) - (zeros, valor)

      {
        {"1010", 4, 0xA}, // (0,0) - EOB
        {"00", 2, 0x0}, // (0,1)
        {"01", 2, 0x1}, // (0,2)
        {"100", 3, 0x4}, // (0,3)
        {"1011", 4, 0xB}, // (0,4)
        {"11010", 5, 0x1A}, // (0,5)
        {"111000", 6, 0x38}, // (0,6)
        {"1111000", 7, 0x78}, // (0,7)
        {"1111110110", 10, 0x3F6}, // (0,8)
        {"1111111110000010", 16, 0xFF82}, // (0,9)
        {"1111111110000011", 16, 0xFF83}, // (0,A)
    },
    {
        {"", 0, 0x0}, // (1,0)
        {"1100", 4, 0xC}, // (1,1)
        {"111001", 6, 0x39}, // (1,2)
        {"1111001", 7, 0x79}, // (1,3)
        {"111110110", 9, 0x1F6}, // (1,4)
        {"11111110110", 11, 0x7F6}, // (1,5)
        {"1111111110000100", 16, 0xFF84}, // (1,6)
        {"1111111110000101", 16, 0xFF85}, // (1,7)
        {"1111111110000110", 16, 0xFF86}, // (1,8)
        {"1111111110000111", 16, 0xFF87}, // (1,9)
        {"1111111110001000", 16, 0xFF88}, // (1,A)
    },
    {
        {"", 0, 0x0}, // (2,0)
        {"11011", 5, 0x1B}, // (2,1)
        {"11111000", 8, 0xF8}, // (2,2)
        {"1111110111", 10, 0x3F7}, // (2,3)
        {"1111111110001001", 16, 0xFF89}, // (2,4)
        {"1111111110001010", 16, 0xFF8A}, // (2,5)
        {"1111111110001011", 16, 0xFF8B}, // (2,6)
        {"1111111110001100", 16, 0xFF8C}, // (2,7)
        {"1111111110001101", 16, 0xFF8D}, // (2,8)
        {"1111111110001110", 16, 0xFF8E}, // (2,9)
        {"1111111110001111", 16, 0xFF8F}, // (2,A)
    },
    {
        {"", 0, 0x0}, // (3,0)
        {"111010", 6, 0x3A}, // (3,1)
        {"111110111", 9, 0x1F7}, // (3,2)
        {"11111110111", 11, 0x7F7}, // (3,3)
        {"1111111110010000", 16, 0xFF90}, // (3,4)
        {"1111111110010001", 16, 0xFF91}, // (3,5)
        {"1111111110010010", 16, 0xFF92}, // (3,6)
        {"1111111110010011", 16, 0xFF93}, // (3,7)
        {"1111111110010100", 16, 0xFF94}, // (3,8)
        {"1111111110010101", 16, 0xFF95}, // (3,9)
        {"1111111110010110", 16, 0xFF96}, // (3,A)
    },
    {
        {"", 0, 0x0}, // (4,0)
        {"111011", 6, 0x3B}, // (4,1)
        {"1111111000", 10, 0x3F8}, // (4,2)
        {"1111111110010111", 16, 0xFF97}, // (4,3)
        {"1111111110011000", 16, 0xFF98}, // (4,4)
        {"1111111110011001", 16, 0xFF99}, // (4,5)
        {"1111111110011010", 16, 0xFF9A}, // (4,6)
        {"1111111110011011", 16, 0xFF9B}, // (4,7)
        {"1111111110011100", 16, 0xFF9C}, // (4,8)
        {"1111111110011101", 16, 0xFF9D}, // (4,9)
        {"1111111110011110", 16, 0xFF9E}, // (4,A)
    },
    {
        {"", 0, 0x0}, // (5,0)
        {"1111010", 7, 0x7A}, // (5,1)
        {"1111111001", 10, 0x3F9}, // (5,2)
        {"1111111110011111", 16, 0xFF9F}, // (5,3)
        {"1111111110100000", 16, 0xFFA0}, // (5,4)
        {"1111111110100001", 16, 0xFFA1}, // (5,5)
        {"1111111110100010", 16, 0xFFA2}, // (5,6)
        {"1111111110100011", 16, 0xFFA3}, // (5,7)
        {"1111111110100100", 16, 0xFFA4}, // (5,8)
        {"1111111110100101", 16, 0xFFA5}, // (5,9)
        {"1111111110100110", 16, 0xFFA6}, // (5,A)
    },
    {
        {"", 0, 0x0}, // (6,0)
        {"1111011", 7, 0x7B}, // (6,1)
        {"11111111000", 11, 0x7F8}, // (6,2)
        {"1111111110100111", 16, 0xFFA7}, // (6,3)
        {"1111111110101000", 16, 0xFFA8}, // (6,4)
        {"1111111110101001", 16, 0xFFA9}, // (6,5)
        {"1111111110101010", 16, 0xFFAA}, // (6,6)
        {"1111111110101011", 16, 0xFFAB}, // (6,7)
        {"1111111110101100", 16, 0xFFAC}, // (6,8)
        {"1111111110101101", 16, 0xFFAD}, // (6,9)
        {"1111111110101110", 16, 0xFFAE}, // (6,A)
    },
    {
        {"", 0, 0x0}, // (7,0)
        {"11111001", 8, 0xF9}, // (7,1)
        {"11111111001", 11, 0x7F9}, // (7,2)
        {"1111111110101111", 16, 0xFFAF}, // (7,3)
        {"1111111110110000", 16, 0xFFB0}, // (7,4)
        {"1111111110110001", 16, 0xFFB1}, // (7,5)
        {"1111111110110010", 16, 0xFFB2}, // (7,6)
        {"1111111110110011", 16, 0xFFB3}, // (7,7)
        {"1111111110110100", 16, 0xFFB4}, // (7,8)
        {"1111111110110101", 16, 0xFFB5}, // (7,9)
        {"1111111110110110", 16, 0xFFB6}, // (7,A)
    },
    {
        {"", 0, 0x0}, // (8,0)
        {"11111010", 8, 0xFA}, // (8,1)
        {"1111111110000000", 16, 0xFFC0}, // (8,2)
        {"1111111110110111", 16, 0xFFB7}, // (8,3)
        {"1111111110111000", 16, 0xFFB8}, // (8,4)
        {"1111111110111001", 16, 0xFFB9}, // (8,5)
        {"1111111110111010", 16, 0xFFBA}, // (8,6)
        {"1111111110111011", 16, 0xFFBB}, // (8,7)
        {"1111111110111100", 16, 0xFFBC}, // (8,8)
        {"1111111110111101", 16, 0xFFBD}, // (8,9)
        {"1111111110111110", 16, 0xFFBE}, // (8,A)
    },
    {
        {"", 0, 0x0}, // (9,0)
        {"111111000", 9, 0x1F8}, // (9,1)
        {"1111111110111111", 16, 0xFFBF}, // (9,2)
        {"1111111111000000", 16, 0xFFC0}, // (9,3)
        {"1111111111000001", 16, 0xFFC1}, // (9,4)
        {"1111111111000010", 16, 0xFFC2}, // (9,5)
        {"1111111111000011", 16, 0xFFC3}, // (9,6)
        {"1111111111000100", 16, 0xFFC4}, // (9,7)
        {"1111111111000101", 16, 0xFFC5}, // (9,8)
        {"1111111111000110", 16, 0xFFC6}, // (9,9)
        {"1111111111000111", 16, 0xFFC7}, // (9,A)
    },
    {
        {"", 0, 0x0}, // (A,0)
        {"111111001", 9, 0x1F9}, // (A,1)
        {"1111111111001000", 16, 0xFFC8}, // (A,2)
        {"1111111111001001", 16, 0xFFC9}, // (A,3)
        {"1111111111001010", 16, 0xFFCA}, // (A,4)
        {"1111111111001011", 16, 0xFFCB}, // (A,5)
        {"1111111111001100", 16, 0xFFCC}, // (A,6)
        {"1111111111001101", 16, 0xFFCD}, // (A,7)
        {"1111111111001110", 16, 0xFFCE}, // (A,8)
        {"1111111111001111", 16, 0xFFCF}, // (A,9)
        {"1111111111010000", 16, 0xFFD0}, // (A,A)
    },
    {
        {"", 0, 0x0}, // (B,0)
        {"111111010", 9, 0x1FA}, // (B,1)
        {"1111111111010001", 16, 0xFFD1}, // (B,2)
        {"1111111111010010", 16, 0xFFD2}, // (B,3)
        {"1111111111010011", 16, 0xFFD3}, // (B,4)
        {"1111111111010100", 16, 0xFFD4}, // (B,5)
        {"1111111111010101", 16, 0xFFD5}, // (B,6)
        {"1111111111010110", 16, 0xFFD6}, // (B,7)
        {"1111111111010111", 16, 0xFFD7}, // (B,8)
        {"1111111111011000", 16, 0xFFD8}, // (B,9)
        {"1111111111011001", 16, 0xFFD9}, // (B,A)
    },
    {
        {"", 0, 0x0}, // (C,0)
        {"1111111010", 10, 0x3FA}, // (C,1)
        {"1111111111011010", 16, 0xFFDA}, // (C,2)
        {"1111111111011011", 16, 0xFFDB}, // (C,3)
        {"1111111111011100", 16, 0xFFDC}, // (C,4)
        {"1111111111011101", 16, 0xFFDD}, // (C,5)
        {"1111111111011110", 16, 0xFFDE}, // (C,6)
        {"1111111111011111", 16, 0xFFDF}, // (C,7)
        {"1111111111100000", 16, 0xFFE0}, // (C,8)
        {"1111111111100001", 16, 0xFFE1}, // (C,9)
        {"1111111111100010", 16, 0xFFE2}, // (C,A)
    },
    {
        {"", 0, 0x0}, // (D,0)
        {"11111111010", 11, 0x7FA}, // (D,1)
        {"1111111111100011", 16, 0xFFE3}, // (D,2)
        {"1111111111100100", 16, 0xFFE4}, // (D,3)
        {"1111111111100101", 16, 0xFFE5}, // (D,4)
        {"1111111111100110", 16, 0xFFE6}, // (D,5)
        {"1111111111100111", 16, 0xFFE7}, // (D,6)
        {"1111111111101000", 16, 0xFFE8}, // (D,7)
        {"1111111111101001", 16, 0xFFE9}, // (D,8)
        {"1111111111101010", 16, 0xFFEA}, // (D,9)
        {"1111111111101011", 16, 0xFFEB}, // (D,A)
    },
    {
        {"", 0, 0x0}, // (E,0)
        {"111111110110", 12, 0xFF6}, // (E,1)
        {"1111111111101100", 16, 0xFFEC}, // (E,2)
        {"1111111111101101", 16, 0xFFED}, // (E,3)
        {"1111111111101110", 16, 0xFFEE}, // (E,4)
        {"1111111111101111", 16, 0xFFEF}, // (E,5)
        {"1111111111110000", 16, 0xFFF0}, // (E,6)
        {"1111111111110001", 16, 0xFFF1}, // (E,7)
        {"1111111111110010", 16, 0xFFF2}, // (E,8)
        {"1111111111110011", 16, 0xFFF3}, // (E,9)
        {"1111111111110100", 16, 0xFFF4}, // (E,A)
    },
    {
        {"111111110111", 12, 0xFF7}, // (F,0) - ZRL
        {"1111111111110101", 16, 0xFFF5}, // (F,1)
        {"1111111111110110", 16, 0xFFF6}, // (F,2)
        {"1111111111110111", 16, 0xFFF7}, // (F,3)
        {"1111111111111000", 16, 0xFFF8}, // (F,4)
        {"1111111111111001", 16, 0xFFF9}, // (F,5)
        {"1111111111111010", 16, 0xFFFA}, // (F,6)
        {"1111111111111011", 16, 0xFFFB}, // (F,7)
        {"1111111111111100", 16, 0xFFFC}, // (F,8)
        {"1111111111111101", 16, 0xFFFD}, // (F,9)
        {"1111111111111110", 16, 0xFFFE}, // (F,A)
    }
    };

#endif