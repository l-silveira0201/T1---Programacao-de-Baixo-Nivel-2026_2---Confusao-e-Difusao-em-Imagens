/*

Para compilar de dentro do VS Code:

Windows/Linux: CTRL+SHIFT+B (Terminal -> Run Build Task)
macOS: COMMAND+SHIFT+B

Para executar de dentro do VS Code:

Executar normalmente: CTRL+F5 (Run -> Run Without Debugging)
Debugar: F5 (Run -> Start Debugging)
Usando o CTRL+F5, para que o programa execute normalmente, a imagem que se deseja criptografar deve ser nomeada como original.jpg

Pelo terminal:

Windows: mingw32-make
Linux/macOS: make

Para executar:

./bagunceitor [arquivo com a imagem de entrada]

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Para usar strings
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>
#include <stb_image_write.h>

// Um pixel Pixel (24 bits)
typedef struct {
  unsigned char r, g, b;
} Pixel;

// Uma imagem Pixel
typedef struct {
  int width, height;  // largura, altura
  int channels;       // qtd de canais (geralmente 3, RGB)
  Pixel* pixels;
} Img;

// As 2 imagens
Img in, out;

// Protótipos
void load(char* name, Img* pic);
void confusao(int altura, int largura, Pixel in[altura][largura], Pixel out[altura][largura], Pixel chave);
void difusao(Pixel* in, Pixel* out, int n, int a);
void desconfusao(int altura, int largura, Pixel in[altura][largura], Pixel out[altura][largura], Pixel chave);
void desdifusao(Pixel* in, Pixel* out, int n, int a);
int mdc(int a, int b);

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("bagunceitor [origem]\n");
    exit(1);
  }

  // Carrega a imagem original
  load(argv[1], &in);

  // Exibe as dimensões na tela, para conferência
  printf("Origem   : %s %d x %d\n", argv[1], in.width, in.height);

  printf("Processando...\n");

  // Cria imagem de saída e "zera" ela
  int tam = in.width * in.height;
  out = in;
  out.pixels = malloc(tam * sizeof(Pixel));
  memset(out.pixels, 0, tam * sizeof(Pixel));

  // Converte para interpretar como matrizes
  Pixel(*pin)[in.width] = (Pixel(*)[in.height])in.pixels;
  Pixel(*pout)[in.width] = (Pixel(*)[in.height])out.pixels;

  //
  // Neste ponto, voce deve implementar os seus algoritmos!
  // (ou chamar funcoes para fazer isso)
  //
  // Aplica o algoritmo em pin e gera a saida em pout
  // ...
  //
  // Exemplo: inverte as cores
  /*
  for (int i = 0; i < in.height; i++) {
    for (int j = 0; j < in.width; j++) {
      pout[i][j].r = 255 - pin[i][j].r;
      pout[i][j].g = 255 - pin[i][j].g;
      pout[i][j].b = 255 - pin[i][j].b;
    }
  }
  */

  // Definição das chaves
  Pixel imagemChave = {0xA5, 0x3C, 0x7E};
  int numChave = 7919;
  while (mdc(numChave, tam) != 1) {
    numChave++;
  }

  Pixel* temp = malloc(tam * sizeof(Pixel));                                // Criação de uma imagem temporária para não perder 
                                                                            // informação na difusão
  confusao(in.height, in.width, pin, pout, imagemChave);
  stbi_write_png("confusao.png", out.width, out.height, 3, pout, 0);        // Etapa 1: Confusão (C)

  difusao(out.pixels, temp, tam, numChave);
  stbi_write_png("criptografada.png", out.width, out.height, 3, temp, 0);   // Salvando a imagem criptografada (C + D)

  desdifusao(temp, out.pixels, tam, numChave);
  stbi_write_png("desdifusao.png", out.width, out.height, 3, pout, 0);      // Etapa 3: Desdifusão (D^-1)

  desconfusao(in.height, in.width, pout, pout, imagemChave);                // Voltando à imagem original (D^-1 + C^-1)

  // Grava a imagem como PNG para registro
  stbi_write_png("saida.png", out.width, out.height, 3, pout, 0);

  printf("Recuperou a memoria original? ");
  if (memcmp(pin, pout, tam * sizeof(Pixel)) == 0) {
    printf("SIM\n");
  } else {
    printf("NAO\n");
  }

  free(in.pixels);
  free(out.pixels);
  free(temp);
}

void load(char* name, Img* pic) {
  pic->pixels =
      (Pixel*)stbi_load(name, &pic->width, &pic->height, &pic->channels, 0);
  if (!pic->pixels) {
    printf("Erro de leitura: %s\n", stbi_failure_reason());
    exit(1);
  }
  printf("Load: %d x %d x %d\n", pic->width, pic->height, pic->channels);
  // Exibe um bloco de 8 x 8 pixels em hexadecimal (teste)
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      printf("[%02X %02X %02X] ", pic->pixels[i].r, pic->pixels[i].g,
             pic->pixels[i].b);
    }
    printf("\n");
  }
}

// Encontra o MDC entre dois valores pelo Algoritmo de Euclides, para garantir que a e n sejam coprimos na função de difusão
int mdc(int a, int b)
{
  while(b) {
    int resto = a % b;
    a = b;
    b = resto;
  }
  return a;
}

// Realiza o processo de confusão da imagem
void confusao(int altura, int largura, Pixel in[altura][largura], Pixel out[altura][largura], Pixel chave)
{
  for(int i = 0; i < altura; i++) {
    for (int j = 0; j < largura; j++) {
      out[i][j].r = ((in[i][j].r + chave.r) & 0xFF) ^ chave.r;
      out[i][j].g = ((in[i][j].g + chave.g) & 0xFF) ^ chave.g;
      out[i][j].b = ((in[i][j].b + chave.b) & 0xFF) ^ chave.b;
    }
  }
}

// Realiza o processo de difusão da imagem
void difusao(Pixel* in, Pixel* out, int n, int a)
{
  for (int i = 0; i < n; i++){
    // Cast para (long long) pois a multiplicação i * a pode dar um resultado muito grande, estourando o limite de int
    *(out + (long long)i * a % n) = *(in + i);
  }
}

// Realiza o processo inverso de confusão da imagem
void desconfusao(int altura, int largura, Pixel in[altura][largura], Pixel out[altura][largura], Pixel chave)
{
  for(int i = 0; i < altura; i++) {
    for (int j = 0; j < largura; j++) {
      out[i][j].r = ((in[i][j].r ^ chave.r) - chave.r) & 0xFF;
      out[i][j].g = ((in[i][j].g ^ chave.g) - chave.g) & 0xFF;
      out[i][j].b = ((in[i][j].b ^ chave.b) - chave.b) & 0xFF;
    }
  }
}

// Realiza o processo inverso de difusão da imagem
void desdifusao(Pixel* in, Pixel* out, int n, int a)
{
  for (int i = 0; i < n; i++){
    *(out + i) = *(in + (long long)i * a % n);
  }
}
