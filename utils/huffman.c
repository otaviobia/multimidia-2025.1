/* Esse arquivo é responsável por implementar a codificação de Huffman e manipulação de buffers de bits. 
*/
#include "huffman.h"
#include "codec.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

BitBuffer* init_bit_buffer(size_t initial_capacity) {
    /* Inicializa um buffer de bits com uma capacidade inicial.
     * Retorna um ponteiro para o buffer ou NULL em caso de erro.
     *
     * Parâmetros:
     * initial_capacity: capacidade inicial do buffer em bytes
    */
    BitBuffer* buffer = (BitBuffer*)calloc(1, sizeof(BitBuffer));
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

void free_bit_buffer(BitBuffer* buffer) {
    /* Libera a memória alocada para o buffer de bits.
     * Se o buffer for NULL, não faz nada.
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer a ser liberado
    */
    if (buffer) {
        if (buffer->data) free(buffer->data);
        free(buffer); // libera a estrutura do buffer
    }
}

static int ensure_capacity(BitBuffer* buffer, size_t additional_bits) {
    /* Garante que o buffer tem espaço suficiente para escrever mais bits.
     * Se necessário, aumenta a capacidade do buffer.
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits
     * additional_bits: número de bits adicionais que serão escritos
     *
     * Retorna 1 se o espaço foi garantido, 0 em caso de erro.
    */
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

int write_bits(BitBuffer* buffer, int value, int num_bits) {
    /* Escreve um valor de bits no buffer, começando pelo bit mais significativo (MSB).
     * Se o buffer não tiver espaço suficiente, aumenta a capacidade.
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits
     * value: valor a ser escrito (deve estar dentro do intervalo permitido)
     * num_bits: número de bits a serem escritos
     *
     * Retorna 1 se a escrita foi bem-sucedida, 0 em caso de erro.
    */
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

int write_dc_coefficient(BitBuffer* buffer, int dc_diff) {
    /* Escreve um coeficiente DC no buffer usando codificação Huffman.
     * O coeficiente é a diferença entre o valor atual e o valor anterior.
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits
     * dc_diff: diferença do coeficiente DC a ser escrito
     *
     * Retorna 1 se a escrita foi bem-sucedida, 0 em caso de erro.
    */
    // Clamp DC value to category 12 range if it exceeds it
    int original_dc = dc_diff;
    if (dc_diff > 4095) {
        dc_diff = 4095;
        printf("AVISO: DC coeficiente %d clamped para %d (limite categoria 12)\n", original_dc, dc_diff);
    } else if (dc_diff < -4095) {
        dc_diff = -4095;
        printf("AVISO: DC coeficiente %d clamped para %d (limite categoria 12)\n", original_dc, dc_diff);
    }
    
    // Determina a categoria do coeficiente
    int category = get_coefficient_category(dc_diff);
    
    // Debug: verifica se a categoria está dentro dos limites
    if (category > 12) {
        printf("ERRO: DC coeficiente %d ainda excede categoria 12 após clamp (categoria calculada: %d)\n", dc_diff, category);
        return 0;  // Erro crítico
    }
    
    // Obtém o código Huffman para esta categoria
    const HuffmanEntry* entry = &JPEG_DC_LUMINANCE_TABLE[category];
    
    // Escreve o prefixo Huffman para a categoria
    if (!write_bits(buffer, entry->code_value, entry->code_length)) {
        printf("Erro ao escrever prefixo Huffman para DC categoria %d\n", category);
        return 0;
    }
    
    // Se categoria não for zero, precisa escrever o valor
    if (category > 0) {
        // Codifica o valor de acordo com a categoria
        int encoded_value = get_coefficient_code(dc_diff, category);
        
        // Escreve o valor codificado
        if (!write_bits(buffer, encoded_value, category)) { // Retorna 0 se falhar
            printf("Erro ao escrever valor codificado %d para DC categoria %d\n", encoded_value, category);
            return 0;
        }
    }
    
    return 1;
}

int write_ac_coefficient(BitBuffer* buffer, int run_length, int ac_value) {
    /* Escreve um coeficiente AC no buffer usando codificação Huffman.
     * O coeficiente é representado por um par (run_length, ac_value).
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits
     * run_length: número de zeros antes do valor AC
     * ac_value: valor do coeficiente AC a ser escrito
     *
     * Retorna 1 se a escrita foi bem-sucedida, 0 em caso de erro.
    */
    // EOB - End of Block (0,0)
    if (run_length == 0 && ac_value == 0) {
        const HuffmanEntry* entry = &JPEG_AC_LUMINANCE_MATRIX[0][0];
        if (!write_bits(buffer, entry->code_value, entry->code_length)) {
            printf("Erro ao escrever EOB\n");
            return 0;
        }
        return 1;
    }
    
    // ZRL - Zero Run Length (15,0)
    if (run_length == 15 && ac_value == 0) {
        const HuffmanEntry* zrl = &JPEG_AC_LUMINANCE_MATRIX[15][0];
        if (!write_bits(buffer, zrl->code_value, zrl->code_length)) {
            printf("Erro ao escrever ZRL\n");
            return 0;
        }
        return 1;
    }
    
    // Se o ac_value é zero, mas não é EOB ou ZRL, é um erro/n existe na tabela fornecida
    if (ac_value == 0) {
        printf("Erro: Combinação inválida em JPEG RLE - run_length=%d, ac_value=0\n", run_length);
        return 0;  // Combinação inválida em JPEG RLE
    }
    
    // Trata sequências longas de zeros (>15) usando ZRL
    while (run_length > 15) {
        // ZRL - Zero Run Length (15,0)
        const HuffmanEntry* zrl = &JPEG_AC_LUMINANCE_MATRIX[15][0];
        if (!write_bits(buffer, zrl->code_value, zrl->code_length)) {
            printf("Erro ao escrever ZRL durante run_length > 15\n");
            return 0;
        }
        run_length -= 16;
    }    // Clamp AC value to prevent excessive categories
    int original_ac = ac_value;
    if (ac_value > 1023) {
        ac_value = 1023;  // Max for category 10
        printf("AVISO: AC coeficiente %d clamped para %d (limite categoria 10)\n", original_ac, ac_value);
    } else if (ac_value < -1023) {
        ac_value = -1023;  // Min for category 10
        printf("AVISO: AC coeficiente %d clamped para %d (limite categoria 10)\n", original_ac, ac_value);
    }
    
    // Categoria do valor AC
    int category = get_coefficient_category(ac_value);
    if (category > 10) {
        printf("AVISO: AC coeficiente %d excede categoria 10 (categoria calculada: %d). Limitando.\n", ac_value, category);
        category = 10;  // Limita a tabela disponível
    }
    
    // Verifica se run_length está dentro dos limites da tabela
    if (run_length > 15) {
        printf("Erro: run_length %d excede limite da tabela AC (máximo 15)\n", run_length);
        return 0;
    }
    
    // Obtem o código Huffman para o par (run, category)
    const HuffmanEntry* entry = &JPEG_AC_LUMINANCE_MATRIX[run_length][category];
    
    // Verifica se este par tem entrada na tabela
    if (entry->code_length == 0) {
        // Se categoria 11 não tem entrada para este run_length, tenta categoria 10
        if (category == 11) {
            printf("AVISO: Entrada AC (run=%d, cat=11) não disponível. Usando categoria 10 para AC valor %d.\n", run_length, ac_value);
            category = 10;
            entry = &JPEG_AC_LUMINANCE_MATRIX[run_length][category];
            if (entry->code_length == 0) {
                printf("Erro: Combinação inválida na tabela AC - run_length=%d, category=%d\n", run_length, category);
                return 0;  // Combinação inválida
            }
        } else {
            printf("Erro: Combinação inválida na tabela AC - run_length=%d, category=%d\n", run_length, category);
            return 0;  // Combinação inválida
        }
    }
    
    // Escreve o prefixo Huffman para o par (run, category)
    if (!write_bits(buffer, entry->code_value, entry->code_length)) {
        printf("Erro ao escrever prefixo Huffman AC (run=%d, cat=%d)\n", run_length, category);
        return 0;
    }
    
    // Codifica o valor AC dentro da categoria
    int encoded_value = get_coefficient_code(ac_value, category);
    
    // Escreve o valor codificado
    if (!write_bits(buffer, encoded_value, category)) {
        printf("Erro ao escrever valor AC codificado %d (categoria %d)\n", encoded_value, category);
        return 0;
    }
    
    return 1;
}

int huffman_encode_block(BitBuffer* buffer, BLOCO_RLE_DIFERENCIAL* block) {
    /* Codifica um bloco RLE diferencial usando Huffman.
     * Codifica o coeficiente DC e os pares AC (zeros, valor).
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits onde os dados serão escritos
     * block: ponteiro para o bloco a ser codificado
     *
     * Retorna 1 se a codificação foi bem-sucedida, 0 em caso de erro.
    */
    // Codifica o coeficiente DC
    if (!write_dc_coefficient(buffer, block->coeficiente_dc)) {
        printf("Erro ao codificar DC: %d\n", block->coeficiente_dc);
        return 0;
    }
    
    // Codifica os pares AC (zeros, valor)
    for (int i = 0; i < block->quantidade; i++) {
        int zeros = block->pares[i].zeros;
        int valor = block->pares[i].valor;
        
        // Se temos o par (0,0), é um EOB
        if (block->pares[i].zeros == 0 && block->pares[i].valor == 0) {
            if (!write_ac_coefficient(buffer, 0, 0)) {
                printf("Erro ao codificar EOB\n");
                return 0;
            }
            return 1;
        }
        
        // Codifica o par AC
        if (!write_ac_coefficient(buffer, zeros, valor)) {
            printf("Erro ao codificar AC[%d]: zeros=%d, valor=%d\n", i, zeros, valor);
            return 0;
        }
    }
    
    // Se não terminou com EOB explícito, adiciona um
    if (block->quantidade == 0 || 
        !(block->pares[block->quantidade-1].zeros == 0 && 
          block->pares[block->quantidade-1].valor == 0)) {
        if (!write_ac_coefficient(buffer, 0, 0)) {
            printf("Erro ao adicionar EOB final\n");
            return 0;
        }
    }
    
    return 1;
}

BitBuffer* huffman_encode_macroblock(MACROBLOCO_RLE_DIFERENCIAL* macroblock) {
    /* Codifica um macrobloco RLE diferencial usando Huffman.
     * Codifica os blocos Y (luminância) e os blocos Cb e Cr (crominância).
     *
     * Parâmetros:
     * macroblock: ponteiro para o macrobloco a ser codificado
     *
     * Retorna um ponteiro para o buffer de bits contendo os dados codificados,
     * ou NULL em caso de erro.
    */
    // Inicializa o buffer com um tamanho razoável
    BitBuffer* buffer = init_bit_buffer(1024);
    if (!buffer) return NULL;
    
    // Codifica os blocos Y (luminância)
    for (int i = 0; i < 4; i++) {
        if (!huffman_encode_block(buffer, &macroblock->Y_vetor[i])) {
            printf("Erro ao codificar bloco Y[%d]. DC: %d\n", i, macroblock->Y_vetor[i].coeficiente_dc);
            free_bit_buffer(buffer);
            return NULL;
        }
    }
    
    // Codifica o bloco Cb (crominância azul)
    if (!huffman_encode_block(buffer, &macroblock->Cb_vetor)) {
        printf("Erro ao codificar bloco Cb. DC: %d\n", macroblock->Cb_vetor.coeficiente_dc);
        free_bit_buffer(buffer);
        return NULL;
    }
    
    // Codifica o bloco Cr (crominância vermelha)
    if (!huffman_encode_block(buffer, &macroblock->Cr_vetor)) {
        printf("Erro ao codificar bloco Cr. DC: %d\n", macroblock->Cr_vetor.coeficiente_dc);
        free_bit_buffer(buffer);
        return NULL;
    }
    
    return buffer;
}

size_t get_huffman_buffer_size(BitBuffer* buffer) {
    /* Retorna o tamanho do buffer de bits em bytes.
     * Se o buffer for NULL, retorna 0.
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits
    */
    if (!buffer) return 0;
    return buffer->byte_position + (buffer->bit_position > 0 ? 1 : 0);
}

int read_bits(BitBuffer* buffer, int num_bits) {
    /* Lê um número específico de bits do buffer de bits.
     * Retorna o valor lido ou -1 em caso de erro (como fim do buffer).
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits
     * num_bits: número de bits a serem lidos
    */
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

int decode_dc_huffman(BitBuffer* buffer) {
    /* Decodifica um código Huffman para um coeficiente DC.
     * Retorna a categoria do coeficiente DC ou -1 em caso de erro.
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits onde os dados serão lidos
    */
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
        for (int i = 0; i <= 12; i++) {
            const HuffmanEntry* entry = &JPEG_DC_LUMINANCE_TABLE[i];
            if (entry->code_length == bits_read && 
                entry->code_value == current_code) {
                return i; // Retorna a categoria ao encontrar correspondência exata
            }
        }
    }
    
    return -1; // Código inválido - não encontrou nas tabelas ou excedeu comprimento máximo
}

int decode_ac_huffman(BitBuffer* buffer, int* run_length, int* category) {
    /* Decodifica um código Huffman para um coeficiente AC.
     * Retorna 1 se encontrou um código válido, 0 se não encontrou,
     * ou -1 em caso de erro.
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits onde os dados serão lidos
     * run_length: ponteiro para armazenar o comprimento do run (número de zeros)
     * category: ponteiro para armazenar a categoria do coeficiente AC
    */
    int bits_read = 0;
    int current_code = 0;
    
    // Lê bits um por um até encontrar um código válido
    while (bits_read < MAX_HUFFMAN_CODE_LENGTH) {
        int bit = read_bits(buffer, 1);
        if (bit < 0) return -1; // Erro de leitura
        
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

int decode_dc_coefficient(int* result_val, BitBuffer* buffer) {
    /* Decodifica um coeficiente DC a partir do buffer de bits.
     * Retorna 1 se a decodificação foi bem-sucedida, 0 em caso de erro.
     *
     * Parâmetros:
     * result_val: ponteiro para armazenar o valor decodificado
     * buffer: ponteiro para o buffer de bits onde os dados serão lidos
    */
    // Decodifica o símbolo Huffman para obter a categoria
    int category = decode_dc_huffman(buffer);
    if (category < 0) return 0; // Erro

    // Se for categoria 0, o valor é 0
    if (category == 0) {
        *result_val = 0;
        return 1; // Sucesso
    }

    // Lê os bits adicionais que representam o valor
    int additional_bits = read_bits(buffer, category);
    if (additional_bits < 0) return 0; // Erro

    // Converte o código para o valor real
    *result_val = decode_coefficient_from_category(category, additional_bits);
    return 1; // Sucesso
}

int decode_ac_coefficient(BitBuffer* buffer, int* run_length, int* value) {
    /* Decodifica um coeficiente AC a partir do buffer de bits.
     * Retorna 1 se a decodificação foi bem-sucedida, 0 em caso de erro,
     * 2 se encontrou EOB (End of Block), ou 3 se encontrou ZRL (Zero Run Length).
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits onde os dados serão lidos
     * run_length: ponteiro para armazenar o comprimento do run (número de zeros)
     * value: ponteiro para armazenar o valor do coeficiente AC decodificado
    */
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

int huffman_decode_block(BitBuffer* buffer, BLOCO_RLE_DIFERENCIAL* block) {
    /* Decodifica um bloco RLE diferencial usando Huffman.
     * Decodifica o coeficiente DC e os pares AC (zeros, valor).
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits onde os dados serão lidos
     * block: ponteiro para o bloco a ser decodificado
     *
     * Retorna 1 se a decodificação foi bem-sucedida, 0 em caso de erro.
    */
    int dc;
    if (!decode_dc_coefficient(&dc, buffer)) return 0;

    block->coeficiente_dc = dc;
    block->quantidade = 0;
    
    // Decodifica coeficientes AC até encontrar EOB
    int pos = 0;
    while (pos < 63) { // Máximo de 63 coeficientes AC
        int run_length, value;
        int result = decode_ac_coefficient(buffer, &run_length, &value);
          if (result == 0) return 0; // Erro
        if (result == 2) {
            // EOB encontrado - adiciona o marcador [0,0] ao bloco antes de sair
            block->pares[block->quantidade].zeros = 0;
            block->pares[block->quantidade].valor = 0;
            block->quantidade++;
            break;
        }
          // Para ZRL, adiciona o marcador [15,0] ao bloco e avança a posição
        if (result == 3) {
            block->pares[block->quantidade].zeros = 15;
            block->pares[block->quantidade].valor = 0;
            block->quantidade++;
            pos += 16;
            continue;
        }
        
        // Avança a posição pelo número de zeros
        pos += run_length;
        if (pos >= 63) break;
        
        // Adiciona o par (run_length, value) ao bloco
        block->pares[block->quantidade].zeros = run_length;
        block->pares[block->quantidade].valor = value;
        block->quantidade++;
    }
    
    return 1;
}

int huffman_decode_macroblock(BitBuffer* buffer, MACROBLOCO_RLE_DIFERENCIAL* dest_macroblock) {
    /* Decodifica um macrobloco RLE diferencial usando Huffman.
     * Decodifica os blocos Y (luminância) e os blocos Cb e Cr (crominância).
     *
     * Parâmetros:
     * buffer: ponteiro para o buffer de bits onde os dados serão lidos
     * dest_macroblock: ponteiro para o macrobloco onde os dados decodificados serão armazenados
     *
     * Retorna 1 se a decodificação foi bem-sucedida, 0 em caso de erro.
    */
    if (!dest_macroblock) return 0;
    
    // Decodifica os blocos Y (luminância)
    for (int i = 0; i < 4; i++) {
        if (!huffman_decode_block(buffer, &dest_macroblock->Y_vetor[i])) return 0;
    }
    
    // Decodifica o bloco Cb (crominância azul)
    if (!huffman_decode_block(buffer, &dest_macroblock->Cb_vetor)) return 0;
    
    // Decodifica o bloco Cr (crominância vermelha)
    if (!huffman_decode_block(buffer, &dest_macroblock->Cr_vetor)) return 0;
    
    return 1;
}

void write_macroblocks_huffman(const char *output_filename, MACROBLOCO_RLE_DIFERENCIAL *rle_macroblocks, int macroblock_count, BITMAPFILEHEADER file_header, BITMAPINFOHEADER info_header, int quality) {
    /* Escreve macroblocos RLE diferencial codificados com Huffman em um arquivo binário.
     * O arquivo contém os headers do BMP, nossos headers e os dados comprimidos dos macroblocos.
     *
     * Parâmetros:
     * output_filename: nome do arquivo de saída
     * rle_macroblocks: ponteiro para o array de macroblocos a serem escritos
     * macroblock_count: número de macroblocos a serem escritos
     * file_header: header do arquivo BMP
     * info_header: header de informações do BMP
     * quality: qualidade da compressão (usada para identificar o tipo de compressão)
    */
    // Abre o arquivo binário de saída
    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        printf("Erro ao abrir o arquivo %s para escrita", output_filename);
        return;
    }

    // Escreve os headers do BMP
    writeHeaders(output_file, file_header, info_header);

    // Escreve nossos headers
    fwrite(&quality, sizeof(int), 1, output_file);
    fwrite(&macroblock_count, sizeof(int), 1, output_file);
    
    // Para cada macrobloco, codifica usando Huffman e escreve no arquivo
    for (int i = 0; i < macroblock_count; i++) {
        BitBuffer *buffer = huffman_encode_macroblock(&rle_macroblocks[i]);
        if (!buffer) {
            printf("Erro ao codificar macrobloco %d com huffman.\n", i);
            continue;
        }

        size_t buffer_size = get_huffman_buffer_size(buffer);

        fwrite(&buffer_size, sizeof(size_t), 1, output_file); // Escreve o tamanho do buffer
        fwrite(buffer->data, sizeof(uint8_t), buffer_size, output_file); // Escreve os dados comprimidos
        
        free_bit_buffer(buffer); // Libera o buffer após escrever
    }

    fclose(output_file);
}

int read_macroblocks_huffman(const char *input_filename, MACROBLOCO_RLE_DIFERENCIAL **blocos_lidos, int *count_lido, BITMAPFILEHEADER *fhead, BITMAPINFOHEADER *ihead, int *quality_lida) {
    /* Lê macroblocos RLE diferencial codificados com Huffman de um arquivo binário.
     * O arquivo contém os headers do BMP, nossos headers e os dados comprimidos dos macroblocos.
     *
     * Parâmetros:
     * input_filename: nome do arquivo de entrada
     * blocos_lidos: ponteiro para armazenar o array de macroblocos lidos
     * count_lido: ponteiro para armazenar o número de macroblocos lidos
     * fhead: ponteiro para o header do arquivo BMP
     * ihead: ponteiro para o header de informações do BMP
     * quality_lida: ponteiro para armazenar a qualidade da compressão lida
     *
     * Retorna 1 se a leitura foi bem-sucedida, 0 em caso de erro.
    */
    // Abre o arquivo binário de entrada
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        printf("Erro ao abrir o arquivo %s para leitura.\n", input_filename);
        return 0;
    }

    // Lê o nosso header do arquivo binário
    readHeader(input_file, fhead);
    readInfoHeader(input_file, ihead);
    fread(quality_lida, sizeof(int), 1, input_file);
    fread(count_lido, sizeof(int), 1, input_file);

    // Aloca memória para os macroblocos que serão lidos
    *blocos_lidos = (MACROBLOCO_RLE_DIFERENCIAL *)calloc((*count_lido), sizeof(MACROBLOCO_RLE_DIFERENCIAL));
    if (!*blocos_lidos) {
        printf("Erro ao alocar memória para os macroblocos.\n");
        fclose(input_file);
        return 0;
    }

    // Lê e decodifica (huffman) todos os macroblocos
    for (int i = 0; i < (*count_lido); i++) { // para cada macrobloco
        size_t buffer_size;
        if (fread(&buffer_size, sizeof(size_t), 1, input_file) != 1) {
            printf("Erro fatal: Não foi possível ler o tamanho do macrobloco %d.\n", i);
            free(*blocos_lidos);
            fclose(input_file);
            return 0;
        }

        BitBuffer *buffer = init_bit_buffer(buffer_size);
        if (fread(buffer->data, 1, buffer_size, input_file) != buffer_size) {
            printf("Erro fatal: Não foi possível ler %d bytes de dados do macrobloco %llu.\n", i, (unsigned long long)buffer_size);
            free_bit_buffer(buffer);
            free(*blocos_lidos);
            fclose(input_file);
            return 0;
        }

        if (!huffman_decode_macroblock(buffer, &((*blocos_lidos)[i]))) {
            printf("Erro: Falha ao decodificar o Huffman do macrobloco %d.\n", i);
        }
        free_bit_buffer(buffer);
    }

    fclose(input_file);
    return 1;
}

int get_coefficient_category(int value) {
    /* Determina a categoria de um coeficiente DC ou AC.
     * Categoria 0: Valor 0
     * Categoria 1: Valores -1, 1
     * Categoria 2: Valores -3 a -2, 2 a 3
     * Categoria 3: Valores -7 a -4, 4 a 7
     * Categoria 4: Valores -15 a -8, 8 a 15
     * Categoria 5: Valores -31 a -16, 16 a 31
     * Categoria 6: Valores -63 a -32, 32 a 63
     * Categoria 7: Valores -127 a -64, 64 a 127
     * Categoria 8: Valores -255 a -128, 128 a 255
     * Categoria 9: Valores -511 a -256, 256 a 511
     * Categoria 10: Valores -1023 a -512, 512 a 1023
     * Categoria 11: Valores -2047 a -1024, 1024 a 2047 (só para DC)
     * 
     * Parâmetros:
     * value: Valor do coeficiente
     *
     * Retorna:
     * Categoria do coeficiente (0 a 11)
     */
    if (value == 0) return 0;
    
    int abs_value = abs(value);
    int category = 0;
    int threshold = 1;
    
    while (abs_value >= threshold) { // Verifica se o valor absoluto é maior ou igual ao limite da categoria
        category++;
        threshold *= 2;
    }
    
    return category;
}

int get_coefficient_code(int value, int category) {
    /* Obtém o código dentro da categoria para um coeficiente.
     * Para valores positivos, o código é o próprio valor.
     * Para valores negativos, usa a representação complementar dentro da categoria.
     *
     * Parâmetros:
     * value: Valor do coeficiente
     * category: Categoria do coeficiente
     *
     * Retorna:
     * Código do coeficiente dentro da categoria
     */
    if (value >= 0) {
        return value;
    } else {
        // Para valores negativos, usa a representação complementar dentro da categoria.
        // O código mapeia valores negativos para a primeira metade dos códigos da categoria.
        // onde (1 << categoria) significa deslocar o bit 1 para a esquerda 'categoria' posições
        // Categoria 1: valores -1, 1
        // Para -1: -1 + (1 << 1) - 1 = -1 + 2 - 1 = 0
        // Categoria 2: valores -3, -2, 2, 3
        // Para -3: -3 + (1 << 2) - 1 = -3 + 4 - 1 = 0 ...
        return value + (1 << category) - 1;
    }
}

int decode_coefficient_from_category(int category, int code) {
    /* Decodifica um coeficiente a partir de um código e sua categoria.
     * Se a categoria for 0, retorna 0.
     * Se o primeiro bit do código estiver ligado, retorna o código como está (valor positivo).
     * Se o primeiro bit estiver desligado, calcula o complemento de 2^category - 1 - code (valor negativo).
     *
     * Parâmetros:
     * category: Categoria do coeficiente
     * code: Código do coeficiente
     *
     * Retorna:
     * Valor decodificado do coeficiente
     */
    if (category == 0) return 0;
    
    // Verifica se o primeiro bit está ligado (valor positivo)
    if ((code >> (category - 1)) & 1) {
        return code; // Valor positivo, mantém como está
    } else {
        // Valor negativo
        // O código é o complemento de 2^category - 1 - code
        return -((1 << category) - 1 - code);
    }
}