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
                           
    if (required_bytes > buffer->capacity) {
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
    if (!buffer || num_bits <= 0) return 0; // Parâmetros inválidos
    
    // Garante que ha espaço suficiente
    if (!ensure_capacity(buffer, num_bits)) return 0;
    
    // Escreve os bits um a um, começando pelo MSB (bit mais significativo)
    for (int i = num_bits - 1; i >= 0; i--) {
        // Extrai o bit na posição i
        int bit = (value >> i) & 1;
        /* 
         * Para extrair o bit na posição i:
         * 1. (value >> i) - Desloca para direita até o bit i ficar na posição 0
         * 2. & 1 - Mascara todos os bits exceto o menos significativo
         * 
         * Ex: Para value=13 (1101 binário) e i=2:
         *     1101 >> 2 = 11 (o bit na posição 2 está agora na posição 0)
         *     11 & 1 = 1 (extraímos o bit)
         */
        
        // Define o bit na posição atual do buffer
        if (bit) {
            buffer->data[buffer->byte_position] |= (1 << (7 - buffer->bit_position));
            /*
             * Aqui estamos fazendo o inverso da leitura:
             * 1. Criamos uma máscara com 1 bit na posição correta: (1 << (7 - bit_position))
             * 2. Aplicamos OR para ligar esse bit específico sem afetar os outros
             * 
             * Exemplo
             * 
             * Escrever o bit 1 na posição bit_position = 3 do byte que atualmente tem o valor 01000010
             * 
             * 1. Criamos a máscara:
             *    (1 << (7 - 3)) = (1 << 4) = 00010000 
             * 
             * 2. Aplicamos OR entre o byte atual e a máscara:
             *    01000010 (byte atual) | 00010000 (máscara)
             *  = 01010010
             * 
             * 3. O bit específico na posição 3 foi ligado sem modificar os outros bits.
             *    
             *    Antes:    0 1 0  0  0 0 1 0
             *    Depois:   0 1 0 (1) 0 0 1 0
             */
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
    
    // ZRL - Zero Run Length (15,0)
    if (run_length == 15 && ac_value == 0) {
        const HuffmanEntry* zrl = &JPEG_AC_LUMINANCE_MATRIX[15][0];
        return write_bits(buffer, zrl->code_value, zrl->code_length);
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

// Lê bits do buffer
int read_bits(BitBuffer* buffer, int num_bits) {
    if (!buffer || num_bits <= 0) return -1; // Erro de parâmetros
    
    int value = 0;
    for (int i = 0; i < num_bits; i++) {
        // Verifica se chegamos ao fim do buffer
        if (buffer->byte_position >= buffer->capacity) {
            return -1; // Fim do buffer
        }
        
        value <<= 1;
        /* 
         * Esta operação multiplica 'value' por 2 (shift left por 1),
         * movendo todos os bits para a esquerda e deixando um 0 no bit 
         * menos significativo, que será preenchido pelo bit que vamos ler
         */
        
        // Lê o bit atual
        if (buffer->data[buffer->byte_position] & (1 << (7 - buffer->bit_position))) {
            value |= 1;
        }
        /* 
         * Explicação detalhada:
         * 1. (1 << (7 - buffer->bit_position)) cria uma máscara com apenas um bit ligado
         *    Ex: se bit_position = 3:
         *        7 - 3 = 4
         *        1 << 4 = 00010000 (bit 4 ligado, contando da direita começando em 0)
         * 
         * 2. Os bytes são organizados com o MSB no bit 7 e o LSB no bit 0:
         *    Posição:  7 6 5 4 3 2 1 0
         *    Valor:    1 1 0 1 0 0 1 1  (exemplo)
         * 
         * 3. O AND bit-a-bit (&) com a máscara verifica se o bit específico está ligado
         *    No exemplo acima, com bit_position=3:
         *    11010011 & 00010000 = 00010000 (resultado não-zero)
         * 
         * 4. Se o resultado for não-zero, o bit está ligado, então
         *    adicionamos 1 ao value usando OR bit-a-bit (value |= 1)
         */
        
        // Avança para o próximo bit
        buffer->bit_position++;
        if (buffer->bit_position == 8) {
            buffer->byte_position++;  // Avança para o próximo byte
            buffer->bit_position = 0; // Recomeça do bit mais significativo
        }
    }
    
    return value;
}

// Decodifica um símbolo Huffman - usando tabela DC
int decode_dc_huffman(BitBuffer* buffer) {
    int bits_read = 0;
    int current_code = 0;
    
    // Lê bits um por um até encontrar um código válido
    while (bits_read < MAX_HUFFMAN_CODE_LENGTH) {
        // Lê um bit
        int bit = read_bits(buffer, 1);
        if (bit < 0) return -1; // Erro de leitura ou fim do buffer
        
        current_code = (current_code << 1) | bit;
        /* 
         * Construindo o código Huffman bit a bit:
         * 1. (current_code << 1) - Desloca os bits já lidos para a esquerda
         *    Ex: se current_code=101 -> após shift: 1010
         * 
         * 2. | bit - Adiciona o novo bit na posição menos significativa usando OR
         *    Ex: se bit=1 -> 1010|1 = 1011
         * 
         * Assim vamos acumulando bits sequencialmente: 1 -> 10 -> 101 -> 1011...
         */
        bits_read++;
        
        // Verifica se este código corresponde a uma categoria DC
        for (int i = 0; i < 11; i++) {
            const HuffmanEntry* entry = &JPEG_DC_LUMINANCE_TABLE[i];
            if (entry->code_length == bits_read && 
                entry->code_value == current_code) {
                return i; // Retorna a categoria ao encontrar correspondência exata
            }
        }
    }
    
    return -1; // Código inválido - não encontrou nas tabelas ou excedeu comprimento máximo
}

// Decodifica um coeficiente DC
int decode_dc_coefficient(BitBuffer* buffer) {
    // Decodifica o símbolo Huffman para obter a categoria
    int category = decode_dc_huffman(buffer);
    if (category < 0) return -1; // Erro
    
    // Se for categoria 0, o valor é 0
    if (category == 0) return 0;
    
    // Lê os bits adicionais que representam o valor
    int additional_bits = read_bits(buffer, category);
    if (additional_bits < 0) return -1; // Erro
    
    // Converte o código para o valor real
    return decode_coefficient_from_category(category, additional_bits);
}

// Decodifica um par AC (run, category)
int decode_ac_huffman(BitBuffer* buffer, int* run_length, int* category) {
    int bits_read = 0;
    int current_code = 0;
    
    // Lê bits um por um até encontrar um código válido
    while (bits_read < MAX_HUFFMAN_CODE_LENGTH) {
        int bit = read_bits(buffer, 1);
        if (bit < 0) return -1; // Erro de leitura
        
        current_code = (current_code << 1) | bit;
        bits_read++;
        
        // Procura na tabela AC
        for (int run = 0; run < 16; run++) {
            for (int cat = 0; cat < 11; cat++) {
                const HuffmanEntry* entry = &JPEG_AC_LUMINANCE_MATRIX[run][cat];
                if (entry->code_length == bits_read && 
                    entry->code_value == current_code) {
                    *run_length = run;
                    *category = cat;
                    return 1; // Sucesso
                }
            }
        }
    }
    
    return 0; // Não encontrou
}

// Decodifica um coeficiente AC
int decode_ac_coefficient(BitBuffer* buffer, int* run_length, int* value) {
    int category;
    if (!decode_ac_huffman(buffer, run_length, &category)) {
        return 0; // Não encontrou código válido
    }
    
    // EOB - End of Block
    if (*run_length == 0 && category == 0) {
        *value = 0;
        return 2; // Indica EOB
    }
    
    // ZRL - Zero Run Length
    if (*run_length == 15 && category == 0) {
        *value = 0;
        return 3; // Indica ZRL
    }
    
    // Lê os bits adicionais
    if (category > 0) {
        int additional_bits = read_bits(buffer, category);
        if (additional_bits < 0) return 0;
        
        // Decodifica o valor real
        *value = decode_coefficient_from_category(category, additional_bits);
    } else {
        *value = 0;
    }
    
    return 1; // Sucesso
}

// Decodifica um bloco inteiro
int huffman_decode_block(BitBuffer* buffer, BLOCO_RLE_DIFERENCIAL* block) {
    // Decodifica o coeficiente DC
    int dc = decode_dc_coefficient(buffer);
    if (dc < 0) return 0;
    
    block->coeficiente_dc = dc;
    block->quantidade = 0;
    
    // Decodifica coeficientes AC até encontrar EOB
    int pos = 0;
    while (pos < 63) { // Máximo de 63 coeficientes AC
        int run_length, value;
        int result = decode_ac_coefficient(buffer, &run_length, &value);
        
        if (result == 0) return 0; // Erro
        if (result == 2) break; // EOB
        
        // Para ZRL, apenas avançamos a posição
        if (result == 3) {
            pos += 16;
            continue;
        }
        
        // Avança a posição pelo número de zeros
        pos += run_length;
        if (pos >= 63) break;
        
        // Adiciona o par (run_length, value) ao bloco
        block->pares[block->quantidade].zeros = run_length;
        block->pares[block->quantidade].valor = (float)value;
        block->quantidade++;
    }
    
    return 1;
}

// Decodifica um macrobloco completo
MACROBLOCO_RLE_DIFERENCIAL* huffman_decode_macroblock(BitBuffer* buffer) {
    MACROBLOCO_RLE_DIFERENCIAL* macroblock = malloc(sizeof(MACROBLOCO_RLE_DIFERENCIAL));
    if (!macroblock) return NULL;
    
    // Decodifica os blocos Y (luminância)
    for (int i = 0; i < 4; i++) {
        if (!huffman_decode_block(buffer, &macroblock->Y_vetor[i])) {
            free(macroblock);
            return NULL;
        }
    }
    
    // Decodifica o bloco Cb (crominância azul)
    if (!huffman_decode_block(buffer, &macroblock->Cb_vetor)) {
        free(macroblock);
        return NULL;
    }
    
    // Decodifica o bloco Cr (crominância vermelha)
    if (!huffman_decode_block(buffer, &macroblock->Cr_vetor)) {
        free(macroblock);
        return NULL;
    }
    
    return macroblock;
}
