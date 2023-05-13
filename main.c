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

void fillImage(unsigned char *imageData, int width, int height, int stride, int bytesPerPixel, int startY, int thickness) {
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

void mirrorImage(unsigned char *imageData, int width, int height, int stride, int bytesPerPixel) {
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

void generateImage(int size, int thickness, int symmetry, int direction, const char *filename, int pointCount, int *points) {
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

    // Рисование ломаной линии
    for (int i = 0; i < pointCount - 1; i++) {
        int startX = points[i * 2];
        int startY = points[i * 2 + 1];
        int endX = points[(i + 1) * 2];
        int endY = points[(i + 1) * 2 + 1];

        int dx = abs(endX - startX);
        int dy = abs(endY - startY);
        int sx = startX < endX ? 1 : -1;
        int sy = startY < endY ? 1 : -1;
        int err = dx - dy;

        while (1) {
            // Проверка границ изображения
            if (startX < 0 || startX >= width || startY < 0 || startY >= height) {
                break;
            }

            // Окрашивание пикселя в черный цвет
            int pixelIndex = startY * stride + startX * bytesPerPixel;
            imageData[pixelIndex] = 0;        // Красный канал
            imageData[pixelIndex + 1] = 0;    // Зеленый канал
            imageData[pixelIndex + 2] = 0;    // Синий канал

            // Завершение, если достигнут конечный пиксель
            if (startX == endX && startY == endY) {
                break;
            }

            int err2 = 2 * err;
            if (err2 > -dy) {
                err -= dy;
                startX += sx;
            }
            if (err2 < dx) {
                err += dx;
                startY += sy;
            }
        }
    }

    // Симметрия
    if (symmetry == 1) {
        mirrorImage(imageData, width, height, stride, bytesPerPixel);
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
    int pointCount = 5;
    int points[] = { 10, 10, 20, 5, 25, 20, 15, 25, 5, 15 };

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
    generateImage(size, thickness, symmetry, direction, filename , pointCount, points);

    return 0;
}