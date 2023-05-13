#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

typedef enum {
  V = 0,
  H = 1
} Direction;

typedef enum {
  S = 0,
  A = 1
} Symmetry;

#pragma pack(push, 1)
typedef struct {
  uint16_t type;           // Сигнатура файла ('BM' для BMP)
  uint32_t fileSize;       // Размер файла в байтах
  uint32_t reserved;       // Зарезервированные поля (всегда 0)
  uint32_t dataOffset;     // Смещение данных от начала файла до начала пикселей
  uint32_t headerSize;     // Размер заголовка (всегда 40 байт)
  uint32_t width;          // Ширина изображения в пикселях
  uint32_t height;         // Высота изображения в пикселях
  uint16_t planes;         // Количество плоскостей (всегда 1)
  uint16_t bitsPerPixel;   // Глубина цвета (количество бит на пиксель)
  uint32_t compression;    // Метод сжатия (0 - без сжатия)
  uint32_t dataSize;       // Размер данных изображения (ширина x высота x глубина цвета в байтах)
  uint32_t hResolution;    // Горизонтальное разрешение (пикселей на метр)
  uint32_t vResolution;    // Вертикальное разрешение (пикселей на метр)
  uint32_t colors;         // Количество цветов в палитре (0 - без палитры)
  uint32_t importantColors;// Количество важных цветов (0 - все цвета важны)
} BMPHeader;
#pragma pack(pop)

void
fillImage(unsigned char *imageData, int width, int height, int stride, int bytesPerPixel, int startY, int thickness) {
  // Окрашивание пикселей в черный цвет
  for (int i = 0; i < thickness; i++) {
    for (int j = 0; j < width; j++) {
      int pixelIndex = startY * stride + j * bytesPerPixel;
      imageData[pixelIndex] = 0;        // Красный канал
      imageData[pixelIndex + 1] = 0;    // Зеленый канал
      imageData[pixelIndex + 2] = 0;    // Синий канал
    }
    startY++;
  }
}

void
generateImage(int size, int thickness, Direction direction, const char *filename, int *points) {
  int width = size;
  int height = size;
  int bytesPerPixel = 3; // 24 бита (8 бит на каждый из каналов RGB)
  int dataSize = width * height * bytesPerPixel;
  
  // Выделение памяти под данные изображения
  unsigned char *imageData = (unsigned char *) malloc(dataSize);
  if (imageData == NULL) {
    printf("Failed to allocate memory for the image data.\n");
    return;
  }
  
  // Заполнение изображения белым цветом
  memset(imageData, 255, dataSize);
  
  // Рисование ломаной линии
  int pointCount = size;
  for (int i = 0; i < pointCount; i++) {
    for (int j = 0; j < thickness; j++) {
      int x, y;
      if(direction == H){
        x = i, y = points[i] + j;
        if(y >= size) y = size - 1;
      } else {
        x = points[i] + j, y = i;
        if(x >= size) x = size - 1;
      }
      int pixelIndex = y * width * bytesPerPixel + x * bytesPerPixel;
      imageData[pixelIndex] = 0;        // Красный канал
      imageData[pixelIndex + 1] = 0;    // Зеленый канал
      imageData[pixelIndex + 2] = 0;    // Синий канал
    }
  }
  
  // Создание заголовка BMP
  BMPHeader header;
  memset(&header, 0, sizeof(BMPHeader));
  header.type = 0x4D42;
  header.fileSize = sizeof(BMPHeader) + dataSize;
  header.dataOffset = sizeof(BMPHeader);
  header.headerSize = 40;
  header.width = width;
  header.height = height;
  header.planes = 1;
  header.bitsPerPixel = 24;
  header.compression = 0;
  header.dataSize = dataSize;
  header.hResolution = 2835;  // 72 dpi
  header.vResolution = 2835;  // 72 dpi
  
  // Запись изображения в файл
  FILE *file = fopen(filename, "wb");
  if (file == NULL) {
    printf("Failed to open the file for writing.\n");
    free(imageData);
    return;
  }
  
  // Запись заголовка
  fwrite(&header, sizeof(BMPHeader), 1, file);
  
  // Запись данных изображения
  fwrite(imageData, dataSize, 1, file);
  
  // Закрытие файла
  fclose(file);
  
  // Освобождение памяти
  free(imageData);
}

int *generatePoints(int size, Symmetry symmetry, int thickness){
  int *points = (int *) malloc(size * 2 * sizeof(int));
  if (points == NULL) {
    printf("Failed to allocate memory for the points.\n");
    return NULL;
  }
  int start = rand() % size;
  points[0] = start;
  for(int i = 1; i < size; i++){
    int prev = points[i - 1];
    int next = prev - thickness + rand() % (2 * thickness + 1);
    if (next < 0) {
      next = 0;
    } else if (next >= size) {
      next = size - 1;
    }
    points[i] = next;
  }
  if(symmetry == S){
    for(int i = 0; i < size/2; i++){
      points[size - i - 1] = points[i];
    }
  }
  return points;
}

int main(int argc, char *argv[]) {
  int size = 32;
  int thickness = 3;
  int symmetry = 0;
  int direction = 0;
  int nImages = 1;
  int seed = (int) time(NULL);
  const char *filename = "output";
  
  // Обработка аргументов командной строки
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-a") == 0) {
      symmetry = 1;  // Асимметричная линия
    } else if (strcmp(argv[i], "-h") == 0) {
      direction = 1;  // Горизонтальная линия
    } else if (strcmp(argv[i], "-s") == 0) {
      symmetry = 2;  // Симметричная линия
    } else if (strcmp(argv[i], "-v") == 0) {
      direction = 2;  // Вертикальная линия
    } else if (strcmp(argv[i], "-seed") == 0 && i + 1 < argc) {
      seed = atoi(argv[i + 1]);  // Устанавливаем начальное значение генератора случайных чисел
      i++;
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      filename = argv[i + 1];  // Устанавливаем имя файла вывода
      i++;
    } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
      nImages = atoi(argv[i + 1]);  // Устанавливаем количество изображений
      i++;
    } else if (strcmp(argv[i], "-size") == 0 && i + 1 < argc) {
      size = atoi(argv[i + 1]);  // Устанавливаем размер изображения
      i++;
    } else if(strcmp(argv[i], "-t") == 0 && i + 1 < argc){
      thickness = atoi(argv[i + 1]);  // Устанавливаем толщину линии
      i++;
    }
  }
  
  // Генерация изображения
  for (int i = 0; i < nImages; i++) {
    srand(seed);
    Direction finalDir;
    Symmetry finalSym;
    switch (seed % 4) {
      case 0:
        finalSym = A;
        finalDir = H;
        break;
      case 1:
        finalSym = S;
        finalDir = H;
        break;
      case 2:
        finalSym = A;
        finalDir = V;
        break;
      case 3:
        finalSym = S;
        finalDir = V;
        break;
    }
    
    if(direction != 0) finalDir = direction == 1 ? H : V;
    char dirLetter = finalDir == H ? 'h' : 'v';
    
    if(symmetry != 0) finalSym = symmetry == 1 ? A : S;
    char symString[5];
    if(finalSym == A) strcpy(symString, "asym");
    else if(finalSym == S) strcpy(symString, "symm");
    
    char finalFilename[200];
    sprintf(finalFilename, "%s-%c-%s-%d.bmp", filename, dirLetter, symString, seed);
    
    int *points = generatePoints(size, finalSym, thickness);
    generateImage(size, thickness, finalDir, finalFilename, points);
    free(points);
    seed = rand();
  }
  
  return 0;
}