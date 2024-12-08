#ifndef YUV_TO_BMP_H
#define YUV_TO_BMP_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
///TODO: Переписать и для цветныхъ изображеній
// Структура для заголовка BMP
#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;      // Тип файла, должно быть 'BM'
    uint32_t bfSize;      // Размер файла в байтах
    uint16_t bfReserved1; // Зарезервировано, должно быть 0
    uint16_t bfReserved2; // Зарезервировано, должно быть 0
    uint32_t bfOffBits;   // Смещение до начала данных изображения
} BMPFileHeader;

typedef struct {
    uint32_t biSize;          // Размер структуры
    int32_t  biWidth;         // Ширина изображения
    int32_t  biHeight;        // Высота изображения (отрицательное значение для перевёрнутого порядка строк)
    uint16_t biPlanes;        // Число цветовых плоскостей, должно быть 1
    uint16_t biBitCount;      // Бит на пиксель (8 для оттенков серого)
    uint32_t biCompression;   // Тип сжатия (0 для несжатого изображения)
    uint32_t biSizeImage;     // Размер изображения в байтах
    int32_t  biXPelsPerMeter; // Горизонтальное разрешение (пиксели на метр)
    int32_t  biYPelsPerMeter; // Вертикальное разрешение (пиксели на метр)
    uint32_t biClrUsed;       // Количество используемых цветов (0 для полного набора)
    uint32_t biClrImportant;  // Количество важных цветов (0 для всех цветов)
} BMPInfoHeader;
#pragma pack(pop)

// Функция для сохранения изображения в BMP
void save_as_bmp(const char *filename, unsigned char *image, int width, int height) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file");
        return;
    }

    // Вычисляем отступ для выравнивания строк по 4 байта
    int row_padding = (4 - (width % 4)) % 4;
    int padded_row_size = width + row_padding;

    // Заголовок файла BMP
    BMPFileHeader fileHeader;
    fileHeader.bfType = 0x4D42; // 'BM'
    fileHeader.bfSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + (padded_row_size * height) + 256 * 4;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + 256 * 4;

    // Информация о BMP
    BMPInfoHeader infoHeader;
    infoHeader.biSize = sizeof(BMPInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = -height; // Отрицательное значение для нормального порядка строк
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 8; // 8 бит на пиксель (градации серого)
    infoHeader.biCompression = 0; // BI_RGB, без сжатия
    infoHeader.biSizeImage = padded_row_size * height;
    infoHeader.biXPelsPerMeter = 2835; // 72 DPI в пикселях на метр
    infoHeader.biYPelsPerMeter = 2835; // 72 DPI в пикселях на метр
    infoHeader.biClrUsed = 256; // Используем 256 цветов
    infoHeader.biClrImportant = 256;

    // Пишем заголовки в файл
    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, file);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, file);

    // Пишем цветовую палитру (256 оттенков серого)
    for (int i = 0; i < 256; i++) {
        unsigned char color[4] = {i, i, i, 0}; // RGB и зарезервированный байт
        fwrite(color, sizeof(color), 1, file);
    }

    // Пишем данные изображения
    unsigned char padding[3] = {0, 0, 0}; // Для заполнения до кратности 4 байтам
    for (int y = 0; y < height; y++) {
        fwrite(&image[y * width], 1, width, file); // Пишем строку
        fwrite(padding, 1, row_padding, file);  // Пишем отступ
    }

    fclose(file);
}

#endif // YUV_TO_BMP_H
