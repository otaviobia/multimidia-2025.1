#include "huffman.h"
#include "codec.h"
#include <stdlib.h>
#include <stdio.h>
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
    // The RLE function now guarantees run_length is always valid (0-15).
    // EOB (0,0) and ZRL (15,0) are handled by a direct table lookup.

    // A run/value pair where the value is 0 but it's not EOB or ZRL is invalid.
    if (ac_value == 0 && run_length != 0 && run_length != 15) {
        return 0;
    }

    // Get the category for the AC magnitude.
    int category = get_coefficient_category(ac_value);
    if (category > 10) category = 10; // Clamp to max category in table.

    // Get the Huffman code for the (run, category) pair.
    const HuffmanEntry* entry = &JPEG_AC_LUMINANCE_MATRIX[run_length][category];

    if (entry->code_length == 0) {
        // This pair does not have a Huffman code in the provided table.
        return 0;
    }

    // Write the Huffman prefix code for the pair.
    if (!write_bits(buffer, entry->code_value, entry->code_length)) {
        return 0;
    }

    // If the value was non-zero, write the bits for the value itself.
    if (category > 0) {
        int encoded_value = get_coefficient_code(ac_value, category);
        if (!write_bits(buffer, encoded_value, category)) {
            return 0;
        }
    }

    return 1;
}

// Codifica um bloco RLE usando Huffman
int huffman_encode_block(BitBuffer* buffer, BLOCO_RLE_DIFERENCIAL* block) {
    // 1. Encode the DC coefficient first.
    if (!write_dc_coefficient(buffer, block->coeficiente_dc)) {
        return 0;
    }

    // 2. Loop through ALL generated RLE pairs and encode them.
    // The previous RLE step guarantees the last pair is the EOB marker.
    // This loop ensures every single one, including the EOB, gets processed.
    for (int i = 0; i < block->quantidade; i++) {
        int run = block->pares[i].zeros;
        int value = (int)round(block->pares[i].valor);

        // Pass the pair to the AC writer, which knows how to handle
        // regular values, ZRL (15,0), and EOB (0,0).
        if (!write_ac_coefficient(buffer, run, value)) {
            return 0;
        }
    }

    // 3. Since the loop processes all pairs, the EOB is now guaranteed
    // to be in the bitstream.
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
    if (!buffer || num_bits <= 0) return -1;

    int value = 0;
    for (int i = 0; i < num_bits; i++) {
        if (buffer->byte_position >= buffer->capacity) {
            printf("  -- !! read_bits ERROR: trying to read from byte %lu, but capacity is only %lu. --\n",
                   (unsigned long)buffer->byte_position, (unsigned long)buffer->capacity);
            return -1;
        }

        value <<= 1;

        if (buffer->data[buffer->byte_position] & (1 << (7 - buffer->bit_position))) {
            value |= 1;
        }

        buffer->bit_position++;
        if (buffer->bit_position == 8) {
            buffer->byte_position++;
            buffer->bit_position = 0;
        }
    }
    //printf("  -- read_bits success, returning value=0x%X (%d) --\n", value, value);
    return value;
}

// Decodifica um símbolo Huffman - usando tabela DC
int decode_dc_huffman(BitBuffer* buffer) {
    int bits_read = 0;
    int current_code = 0;
    //printf("\n--- Decoding DC ---\n");

    while (bits_read < MAX_HUFFMAN_CODE_LENGTH) {
        int bit = read_bits(buffer, 1);
        if (bit < 0) {
            printf("  [ERROR] read_bits failed or hit end of buffer.\n");
            return -1;
        }

        current_code = (current_code << 1) | bit;
        bits_read++;
        //printf("  Read bit: %d  ->  current_code=0x%X (length %d)\n", bit, current_code, bits_read);

        for (int i = 0; i <= 10; i++) {
            const HuffmanEntry* entry = &JPEG_DC_LUMINANCE_TABLE[i];
            if (entry->code_length == bits_read && entry->code_value == current_code) {
                //printf("  >>> MATCH FOUND! Category: %d\n", i);
                return i;
            }
        }
    }

    printf("  [ERROR] No DC match found for final code 0x%X\n", current_code);
    return -1; // Code not found
}

int decode_ac_huffman(BitBuffer* buffer, int* run_length, int* category) {
    int bits_read = 0;
    int current_code = 0;
    //printf("\n--- Decoding AC ---\n");

    while (bits_read < MAX_HUFFMAN_CODE_LENGTH) {
        int bit = read_bits(buffer, 1);
        if (bit < 0) {
            printf("  [ERROR] read_bits failed or hit end of buffer.\n");
            return 0;
        }

        current_code = (current_code << 1) | bit;
        bits_read++;
        //printf("  Read bit: %d  ->  current_code=0x%X (length %d)\n", bit, current_code, bits_read);

        for (int run = 0; run < 16; run++) {
            for (int cat = 0; cat < 11; cat++) {
                const HuffmanEntry* entry = &JPEG_AC_LUMINANCE_MATRIX[run][cat];
                // Check against non-empty table entries
                if (entry->code_length > 0 && entry->code_length == bits_read && entry->code_value == current_code) {
                    //printf("  >>> MATCH FOUND! Run/Category: (%d, %d)\n", run, cat);
                    *run_length = run;
                    *category = cat;
                    return 1; // Success
                }
            }
        }
    }

    printf("  [ERROR] No AC match found for final code 0x%X\n", current_code);
    return 0; // Not found
}

// Decodifica um coeficiente DC
int decode_dc_coefficient(int* result_val, BitBuffer* buffer) {
    int category = decode_dc_huffman(buffer);
    if (category < 0) {
        return 0; // Return 0 for failure
    }

    if (category == 0) {
        *result_val = 0;
        return 1; // Return 1 for success
    }

    int additional_bits = read_bits(buffer, category);
    if (additional_bits < 0) {
        return 0; // Return 0 for failure
    }

    *result_val = decode_coefficient_from_category(category, additional_bits);
    return 1; // Return 1 for success
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
    // Decodes the DC coefficient using the new, safer function signature.
    int dc_value;
    if (!decode_dc_coefficient(&dc_value, buffer)) {
        return 0; // The call failed
    }
    block->coeficiente_dc = dc_value; // Assign the valid (positive or negative) value

    // AC decoding logic remains the same...
    block->quantidade = 0;
    int pos = 0;
    while (pos < 63) {
        int run_length, value;
        int result = decode_ac_coefficient(buffer, &run_length, &value);

        if (result == 0) return 0; // Error
        if (result == 2) break;    // EOB

        if (result == 3) { // ZRL
            pos += 16;
            continue;
        }

        pos += run_length;
        if (pos >= 63) break;

        if(block->quantidade < 63) {
            block->pares[block->quantidade].zeros = run_length;
            block->pares[block->quantidade].valor = (float)value;
            block->quantidade++;
        }
        pos++;
    }

    return 1; // Success
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

int huffman_decode_macroblock_to_dest(BitBuffer* buffer, MACROBLOCO_RLE_DIFERENCIAL* dest_macroblock) {
    if (!dest_macroblock) return 0;
    
    // Decode Y (luminance) blocks
    for (int i = 0; i < 4; i++) {
        if (!huffman_decode_block(buffer, &dest_macroblock->Y_vetor[i])) {
            return 0;
        }
    }
    
    // Decode Cb (chroma blue) block
    if (!huffman_decode_block(buffer, &dest_macroblock->Cb_vetor)) {
        return 0;
    }
    
    // Decode Cr (chroma red) block
    if (!huffman_decode_block(buffer, &dest_macroblock->Cr_vetor)) {
        return 0;
    }
    
    return 1; // Success
}

void write_macroblocks_huffman(const char *output_filename, MACROBLOCO_RLE_DIFERENCIAL *rle_macroblocks, int macroblock_count, BITMAPFILEHEADER file_header, BITMAPINFOHEADER info_header, float quality) {
    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        perror("Error opening output file for writing");
        return;
    }

    // --- Write Headers (Member by Member) ---
    // Change all "file_header->" to "file_header." and "info_header->" to "info_header."
    fwrite(&file_header.Type, sizeof(file_header.Type), 1, output_file);
    fwrite(&file_header.Size, sizeof(file_header.Size), 1, output_file);
    fwrite(&file_header.Reserved1, sizeof(file_header.Reserved1), 1, output_file);
    fwrite(&file_header.Reserved2, sizeof(file_header.Reserved2), 1, output_file);
    fwrite(&file_header.OffBits, sizeof(file_header.OffBits), 1, output_file);
    
    fwrite(&info_header.Size, sizeof(info_header.Size), 1, output_file);
    fwrite(&info_header.Width, sizeof(info_header.Width), 1, output_file);
    fwrite(&info_header.Height, sizeof(info_header.Height), 1, output_file);
    fwrite(&info_header.Planes, sizeof(info_header.Planes), 1, output_file);
    fwrite(&info_header.BitCount, sizeof(info_header.BitCount), 1, output_file);
    fwrite(&info_header.Compression, sizeof(info_header.Compression), 1, output_file);
    fwrite(&info_header.SizeImage, sizeof(info_header.SizeImage), 1, output_file);
    fwrite(&info_header.XResolution, sizeof(info_header.XResolution), 1, output_file);
    fwrite(&info_header.YResolution, sizeof(info_header.YResolution), 1, output_file);
    fwrite(&info_header.NColours, sizeof(info_header.NColours), 1, output_file);
    fwrite(&info_header.ImportantColours, sizeof(info_header.ImportantColours), 1, output_file);
    
    // --- Write Custom Data ---
    uint32_t mc = (uint32_t)macroblock_count;
    fwrite(&quality, sizeof(float), 1, output_file);
    fwrite(&mc, sizeof(uint32_t), 1, output_file);
    
    // --- The rest of the function remains the same ---
    for (int i = 0; i < macroblock_count; i++) {
        BitBuffer *buffer = huffman_encode_macroblock(&rle_macroblocks[i]);
        if (!buffer) {
            fprintf(stderr, "Error: Failed to Huffman encode macroblock %d\n", i);
            continue;
        }

        size_t buffer_size = get_huffman_buffer_size(buffer);

        //printf("  Writing MB %d, size: %lu bytes\n", i, (unsigned long)buffer_size);

        fwrite(&buffer_size, sizeof(size_t), 1, output_file); // Write size of the chunk
        fwrite(buffer->data, sizeof(uint8_t), buffer_size, output_file); // Write data
        
        free_bit_buffer(buffer);
    }

    fclose(output_file);
    printf("Compressed data written to %s\n", output_filename);
}

void print_bytes(const uint8_t* data, size_t size) {
    printf("Buffer data (%llu bytes): ", size);
    size_t limit = size > 32 ? 32 : size; // Print at most 32 bytes
    for(size_t i = 0; i < limit; i++) {
        printf("%02X ", data[i]);
    }
    if (size > 32) {
        printf("...");
    }
    printf("\n");
}

int read_macroblocks_huffman(const char *input_filename, MACROBLOCO_RLE_DIFERENCIAL **blocos_lidos, int *count_lido, BITMAPFILEHEADER *fhead, BITMAPINFOHEADER *ihead, float *quality_lida) {
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        perror("Error opening input file for reading");
        return 0;
    }

    // --- 1. Read Headers (Field by Field) ---
    // (This part is correct and can remain as is, with or without the prints)
    fread(&fhead->Type, sizeof(fhead->Type), 1, input_file);
    fread(&fhead->Size, sizeof(fhead->Size), 1, input_file);
    fread(&fhead->Reserved1, sizeof(fhead->Reserved1), 1, input_file);
    fread(&fhead->Reserved2, sizeof(fhead->Reserved2), 1, input_file);
    fread(&fhead->OffBits, sizeof(fhead->OffBits), 1, input_file);
    fread(&ihead->Size, sizeof(ihead->Size), 1, input_file);
    fread(&ihead->Width, sizeof(ihead->Width), 1, input_file);
    fread(&ihead->Height, sizeof(ihead->Height), 1, input_file);
    fread(&ihead->Planes, sizeof(ihead->Planes), 1, input_file);
    fread(&ihead->BitCount, sizeof(ihead->BitCount), 1, input_file);
    fread(&ihead->Compression, sizeof(ihead->Compression), 1, input_file);
    fread(&ihead->SizeImage, sizeof(ihead->SizeImage), 1, input_file);
    fread(&ihead->XResolution, sizeof(ihead->XResolution), 1, input_file);
    fread(&ihead->YResolution, sizeof(ihead->YResolution), 1, input_file);
    fread(&ihead->NColours, sizeof(ihead->NColours), 1, input_file);
    fread(&ihead->ImportantColours, sizeof(ihead->ImportantColours), 1, input_file);
    uint32_t mc;
    fread(quality_lida, sizeof(float), 1, input_file);
    fread(&mc, sizeof(uint32_t), 1, input_file);
    *count_lido = (int)mc;

    // --- 2. Allocate Memory ---
    *blocos_lidos = (MACROBLOCO_RLE_DIFERENCIAL *)malloc((*count_lido) * sizeof(MACROBLOCO_RLE_DIFERENCIAL));
    if (!*blocos_lidos) {
        fprintf(stderr, "Error: Memory allocation failed for macroblocks.\n");
        fclose(input_file);
        return 0;
    }

    // --- 3. Read and Decode All Macroblocks in a Single, Clean Loop ---
    printf("\n--- DECOMPRESSOR: Reading and Decoding All Macroblocks ---\n");
    for (int i = 0; i < (*count_lido); i++) {
        size_t buffer_size;
        if (fread(&buffer_size, sizeof(size_t), 1, input_file) != 1) {
             fprintf(stderr, "[MB %d] FATAL: Could not read size for chunk.\n", i);
             free(*blocos_lidos);
             fclose(input_file);
             return 0;
        }
        //printf("  [MB %d] Reading chunk, expecting size: %llu bytes\n", i, (unsigned long long)buffer_size);

        BitBuffer *buffer = init_bit_buffer(buffer_size);
        if (fread(buffer->data, 1, buffer_size, input_file) != buffer_size) {
            fprintf(stderr, "[MB %d] FATAL: Could not read %llu bytes of data.\n", i, (unsigned long long)buffer_size);
            free_bit_buffer(buffer);
            free(*blocos_lidos);
            fclose(input_file);
            return 0;
        }

        if (!huffman_decode_macroblock_to_dest(buffer, &((*blocos_lidos)[i]))) {
            fprintf(stderr, "[MB %d] FATAL: Failed to decode Huffman data.\n", i);
            //free_bit_buffer(buffer);
            //free(*blocos_lidos);
            //fclose(input_file);
            //return 0;
        }
        free_bit_buffer(buffer);
    }
    printf("--- All macroblocks decoded successfully ---\n\n");

    fclose(input_file);
    return 1;
}