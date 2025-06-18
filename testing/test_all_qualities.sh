#!/bin/bash

# Script para testar o compressor/descompressor para todos os níveis de qualidade (1-100).
#
# Uso: ./test_all_qualities.sh <imagem_entrada.bmp>
# Exemplo: ./test_all_qualities.sh images/lenna.bmp

# 1. Verifica se a imagem de entrada foi fornecida como argumento
if [ "$#" -ne 1 ]; then
    echo "Uso: $0 <imagem_entrada.bmp>"
    exit 1
fi

INPUT_IMAGE=$1

# 2. Verifica se a imagem de entrada existe
if [ ! -f "$INPUT_IMAGE" ]; then
    echo "Erro: Arquivo de entrada '$INPUT_IMAGE' não encontrado."
    exit 1
fi

# 3. Verifica se os executáveis existem (é uma boa prática)
if [ ! -x "./compressor" ] || [ ! -x "./decompressor" ]; then
    echo "Erro: Certifique-se de que 'compressor' e 'decompressor' estão compilados e executáveis."
    echo "Tente rodar 'make all' primeiro."
    exit 1
fi

# 4. Cria os diretórios para organizar os arquivos de saída
echo "Criando diretórios de saída..."
mkdir -p compressed_files
mkdir -p reconstructed_images

# 5. Loop principal que itera de 1 a 100
for q in {1..100}
do
    echo "--- Testando Qualidade: $q/100 ---"

    # Define nomes de arquivo únicos para esta qualidade
    COMPRESSED_FILE="compressed_files/compressed_q${q}.bin"
    RECONSTRUCTED_FILE="reconstructed_images/reconstructed_q${q}.bmp"

    # Executa o compressor
    echo "Comprimindo com qualidade $q..."
    ./compressor "$INPUT_IMAGE" "$COMPRESSED_FILE" "$q"

    # Verifica se a compressão foi bem-sucedida antes de descomprimir
    if [ -f "$COMPRESSED_FILE" ]; then
        # Executa o descompressor
        echo "Descomprimindo para '$RECONSTRUCTED_FILE'..."
        ./decompressor "$COMPRESSED_FILE" "$RECONSTRUCTED_FILE"
    else
        echo "Erro: Arquivo comprimido '$COMPRESSED_FILE' não foi criado. Pulando a descompressão."
    fi
done

echo ""
echo "--- Todos os testes foram concluídos! ---"
echo "Arquivos comprimidos estão no diretório: 'compressed_files'"
echo "Imagens reconstruídas estão no diretório: 'reconstructed_images'"