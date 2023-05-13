#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

void generateImage(int size, int thickness, int symmetry, int direction, const char *filename) {
    int width = size;
    int height = size;
    int bytesPerPixel = 3; // 24 бита (8 бит на каждый из каналов RGB)
    int dataSize = width * height * bytesPerPixel;
    int stride = width * bytesPerPixel;

    // Выделение памяти под данные изображения
    unsigned char *imageData = (unsigned char *) malloc(dataSize);
    if (imageData == NULL) {
        printf("Failed to allocate memory for the image data.\n");
        return;
    }

    // Заполнение изображения белым цветом
    memset(imageData, 255, dataSize);

    // Начальные координаты и шаг для линии
    int startX, startY, stepX, stepY;

    if (direction == 0) {  // Вертикальная линия
        startX = size / 2;
        startY = thickness / 2;
        stepX = 0;
        stepY = 1;
    } else {  // Горизонтальная линия
        startX = thickness / 2;
        startY = size / 2;
        stepX = 1;
        stepY = 0;
    }

    // Генерация линии
    int x = startX;
    int y = startY;

    for (int i = 0; i < thickness; i++) {
        // Проверка границ изображения
        if (x < 0 || x >= width || y < 0 || y >= height) {
            break;
        }

        // Окрашивание пикселей в черный цвет
        for (int j = 0; j < width; j++) {
            int pixelIndex = y * stride + j * bytesPerPixel;
            imageData[pixelIndex] = 0;        // Красный канал
            imageData[pixelIndex + 1] = 0;    // Зеленый канал
            imageData[pixelIndex + 2] = 0;    // Синий канал
        }

        // Обновление координат
        x += stepX;
        y += stepY;
    }

    // Симметрия
    if (symmetry == 1) {
        // Зеркальное отображение первой половины изображения
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width / 2; col++) {
                int sourceIndex = row * stride + col * bytesPerPixel;
                int targetIndex = row * stride + (width - col - 1) * bytesPerPixel;

                // Копирование пикселя
                imageData[targetIndex] = imageData[sourceIndex];                 // Красный канал
                imageData[targetIndex + 1] = imageData[sourceIndex + 1];         // Зеленый канал
                imageData[targetIndex + 2] = imageData[sourceIndex + 2];         // Синий канал
            }
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


int main(int argc, char *argv[]) {
    int size = 32;
    int thickness = 3;
    int symmetry = 1;
    int direction = 0;
    const char *filename = "output.bmp";

    // Обработка аргументов командной строки
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            symmetry = 0;  // Асимметричная линия
        } else if (strcmp(argv[i], "-h") == 0) {
            direction = 1;  // Горизонтальная линия
        } else if (strcmp(argv[i], "-s") == 0) {
            symmetry = 1;  // Симметричная линия
        } else if (strcmp(argv[i], "-v") == 0) {
            direction = 0;  // Вертикальная линия
        } else if (strcmp(argv[i], "-seed") == 0 && i + 1 < argc) {
            // Игнорируем аргумент seed
            i++;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            filename = argv[i + 1];  // Устанавливаем имя файла вывода
            i++;
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            // Игнорируем аргумент n
            i++;
        } else if (strcmp(argv[i], "-size") == 0 && i + 1 < argc) {
            size = atoi(argv[i + 1]);  // Устанавливаем размер изображения
            i++;
        }
    }

    // Генерация изображения
    generateImage(size, thickness, symmetry, direction, filename);

    return 0;
}