# Projeto: Compressão de Imagens

Projeto da disciplina **SCC0261 - Multimídia**, com o objetivo de desenvolver um compressor para imagens e seu respectivo descompressor.

### Componentes do Grupo

- Christyan Paniago Nantes - 15635906
- Otávio Biagioni Melo - 15482604

## Instruções para Compilação

Utilize `make all` para compilar o código.

## Instruções de Uso

Para comprimir uma imagem bmp utilize o comando:
```
./compressor images/256x256.bmp compressed_output.bin
```

Para descomprimir um binário utilize o comando:
```
./decompressor compressed_output.bin reconstructed_image.bmp
```