#include "huffman.h"
#include "codec.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Inicializa um buffer de bits
BitBuffer* init_bit_buffer(size_t initial_capacity) {
    BitBuffer* buffer = (BitBuffer*)malloc(sizeof(BitBuffer));
    if (!buffer) return NULL; // erro em alocar memória
    
    buffer->data = (uint8_t*)calloc(initial_capacity, sizeof(uint8_t));
    if (!buffer->data) {
        free(buffer);
        return NULL;
    }
    
    buffer->capacity = initial_capacity;
    buffer->byte_position = 0;
    buffer->bit_position = 0;
    
    return buffer;
}

// Libera um buffer de bits
void free_bit_buffer(BitBuffer* buffer) {
    if (buffer) {
        if (buffer->data) free(buffer->data);
        free(buffer); // libera a estrutura do buffer
    }
}

// Garante que o buffer tem capacidade suficiente
static int ensure_capacity(BitBuffer* buffer, size_t additional_bits) {
    size_t required_bytes = buffer->byte_position + 
                           (buffer->bit_position + additional_bits + 7) / 8;
                           
    if (required_bytes >= buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2;
        if (new_capacity < required_bytes) new_capacity = required_bytes + 64;
        
        uint8_t* new_data = (uint8_t*)realloc(buffer->data, new_capacity);
        if (!new_data) return 0;
        
        // Zera os novos bytes
        memset(new_data + buffer->capacity, 0, new_capacity - buffer->capacity);
        
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }
    
    return 1;
}

// Escreve bits no buffer
int write_bits(BitBuffer* buffer, int value, int num_bits) {
    if (!buffer || num_bits <= 0) return 0;
    
    // Garante que ha espaço suficiente
    if (!ensure_capacity(buffer, num_bits)) return 0;
    
    // Escreve os bits um a um, começando pelo MSB
    for (int i = num_bits - 1; i >= 0; i--) {
        // Extrai o bit na posição i
        int bit = (value >> i) & 1;
        
        // Define o bit na posição atual do buffer
        if (bit) {
            buffer->data[buffer->byte_position] |= (1 << (7 - buffer->bit_position));
        }
        
        // Avanca para o proximo bit
        buffer->bit_position++;
        
        // Se completou um byte, avança para o proximo
        if (buffer->bit_position == 8) {
            buffer->byte_position++;
            buffer->bit_position = 0;
        }
    }
    
    return 1;
}

// Escreve um coeficiente DC no buffer
int write_dc_coefficient(BitBuffer* buffer, int dc_diff) {
    // Determina a categoria do coeficiente
    int category = get_coefficient_category(dc_diff);
    if (category > 10) category = 10;  // Limita a tabela disponível
    
    // Obtém o código Huffman para esta categoria
    const HuffmanEntry* entry = &JPEG_DC_LUMINANCE_TABLE[category];
    
    // Escreve o prefixo Huffman para a categoria
    if (!write_bits(buffer, entry->code_value, entry->code_length)) {
        return 0;
    }
    
    // Se categoria não for zero, precisa escrever o valor
    if (category > 0) {
        // Codifica o valor de acordo com a categoria
        int encoded_value = get_coefficient_code(dc_diff, category);
        
        // Escreve o valor codificado
        if (!write_bits(buffer, encoded_value, category)) { // Retorna 0 se falhar
            return 0;
        }
    }
    
    return 1;
}

// Escreve um coeficiente AC no buffer
int write_ac_coefficient(BitBuffer* buffer, int run_length, int ac_value) {
    // EOB - End of Block (0,0)
    if (run_length == 0 && ac_value == 0) {
        const HuffmanEntry* entry = &JPEG_AC_LUMINANCE_MATRIX[0][0];
        return write_bits(buffer, entry->code_value, entry->code_length);
    }
    
    // Trata sequências longas de zeros (>15) usando ZRL
    while (run_length > 15) {
        // ZRL - Zero Run Length (15,0)
        const HuffmanEntry* zrl = &JPEG_AC_LUMINANCE_MATRIX[15][0];
        if (!write_bits(buffer, zrl->code_value, zrl->code_length)) {
            return 0;
        }
        run_length -= 16;
    }
    
    // Categoria do valor AC
    int category = get_coefficient_category(ac_value);
    if (category > 10) category = 10;  // Limita a tabela disponível
    
    // Obtem o código Huffman para o par (run, category)
    const HuffmanEntry* entry = &JPEG_AC_LUMINANCE_MATRIX[run_length][category];
    
    // Verifica se este par tem entrada na tabela
    if (entry->code_length == 0) {
        return 0;  // Combinação inválida
    }
    
    // Escreve o prefixo Huffman para o par (run, category)
    if (!write_bits(buffer, entry->code_value, entry->code_length)) {
        return 0;
    }
    
    // Codifica o valor AC dentro da categoria
    int encoded_value = get_coefficient_code(ac_value, category);
    
    // Escreve o valor codificado
    if (!write_bits(buffer, encoded_value, category)) {
        return 0;
    }
    
    return 1;
}

// Codifica um bloco RLE usando Huffman
int huffman_encode_block(BitBuffer* buffer, BLOCO_RLE_DIFERENCIAL* block) {
    // Codifica o coeficiente DC
    if (!write_dc_coefficient(buffer, block->coeficiente_dc)) {
        return 0;
    }
    
    // Codifica os pares AC (zeros, valor)
    for (int i = 0; i < block->quantidade; i++) {
        int zeros = block->pares[i].zeros;
        int valor = (int)round(block->pares[i].valor);
        
        // Se valor é zero e estamos no final, é EOB
        if (valor == 0 && i == block->quantidade - 1) {
            return write_ac_coefficient(buffer, 0, 0);
        }
        
        // Codifica o par AC
        if (!write_ac_coefficient(buffer, zeros, valor)) {
            return 0;
        }
    }
    
    // Se não terminou com EOB explícito, adiciona um
    if (block->quantidade == 0 || 
        !(block->pares[block->quantidade-1].zeros == 0 && 
          block->pares[block->quantidade-1].valor == 0.0f)) {
        return write_ac_coefficient(buffer, 0, 0);
    }
    
    return 1;
}

// Codifica um macrobloco completo
BitBuffer* huffman_encode_macroblock(MACROBLOCO_RLE_DIFERENCIAL* macroblock) {
    // Inicializa o buffer com um tamanho razoável
    BitBuffer* buffer = init_bit_buffer(1024);
    if (!buffer) return NULL;
    
    // Codifica os blocos Y (luminância)
    for (int i = 0; i < 4; i++) {
        if (!huffman_encode_block(buffer, &macroblock->Y_vetor[i])) {
            free_bit_buffer(buffer);
            return NULL;
        }
    }
    
    // Codifica o bloco Cb (crominância azul)
    if (!huffman_encode_block(buffer, &macroblock->Cb_vetor)) {
        free_bit_buffer(buffer);
        return NULL;
    }
    
    // Codifica o bloco Cr (crominância vermelha)
    if (!huffman_encode_block(buffer, &macroblock->Cr_vetor)) {
        free_bit_buffer(buffer);
        return NULL;
    }
    
    return buffer;
}

// Função para obter o tamanho final do buffer comprimido em bytes
size_t get_huffman_buffer_size(BitBuffer* buffer) {
    if (!buffer) return 0;
    return buffer->byte_position + (buffer->bit_position > 0 ? 1 : 0);
}