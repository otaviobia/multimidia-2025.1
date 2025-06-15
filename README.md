# Projeto: Compressão de Imagens

Trabalho desenvolvido para a disciplina **SCC0261 - Multimídia**, com o objetivo de implementar um **compressor** e **descompressor** de imagens no formato BMP.

### Componentes do Grupo

- **Christyan Paniago Nantes** — 15635906  
- **Otávio Biagioni Melo** — 15482604

## Como Compilar

Para compilar todos os arquivos do projeto, execute:

```bash
make all
```

## Como Usar

### 🔻 Compressão

Para comprimir uma imagem BMP:

```bash
./compressor <imagem_entrada.bmp> <arquivo_saida.bin> [qualidade]
```

- `imagem_entrada.bmp`: caminho da imagem original em formato BMP  
- `arquivo_saida.bin`: nome desejado para o arquivo comprimido  
- `qualidade`: (opcional) valor de 1 a 100 indicando o nível de qualidade da compressão (padrão: 50)

**Exemplo:**

```bash
./compressor imagem.bmp comprimido.bin 80
```

---

### 🔺 Descompressão

Para descomprimir um arquivo binário e gerar a imagem reconstruída:

```bash
./decompressor <arquivo_entrada.bin> <imagem_saida.bmp>
```

- `arquivo_entrada.bin`: caminho do arquivo comprimido  
- `imagem_saida.bmp`: nome da imagem a ser gerada após a descompressão

**Exemplo:**

```bash
./decompressor comprimido.bin reconstruida.bmp
```

## 📁 Estrutura do Projeto

```
├── compressor.c
├── decompressor.c
├── utils/
│   └── funções auxiliares
├── Makefile
├── README.md
```