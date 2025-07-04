// https://pyokagan.name/blog/2019-10-14-png/
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <math.h>
#include <stdint.h>

typedef struct Chunk {
    unsigned int length;
    unsigned char *type;
    unsigned char *data;
    unsigned char *crc;
    struct Chunk *next;
} Chunk;

Chunk *add_chunk(Chunk *head, unsigned int length, unsigned char *type, unsigned char *data, unsigned char *crc) {
    Chunk *node = malloc(sizeof(Chunk));
    node->length = length;
    node->type = malloc(5);
    memcpy(node->type, type, 5);
    node->data = malloc(length);
    memcpy(node->data, data, length);
    node->crc = malloc(4);
    memcpy(node->crc, crc, 4);
    node->next = NULL;

    if (!head) {
        return node;
    }

    Chunk *cur = head;
    while (cur->next) {
        cur = cur->next;
    }
    cur->next = node;

    return head;
}

void free_chunks(Chunk *head) {
    Chunk *current = head;
    while (current != NULL) {
        Chunk *next = current->next;
        free(current->type);
        free(current->data);
        free(current->crc);
        free(current);
        current = next;
    }
}

unsigned char recon_a(unsigned char *recon, int r, int c, unsigned int bytesPerPixel, unsigned int stride) {
    if (c >= bytesPerPixel) {
        return recon[r * stride + c - bytesPerPixel];
    }
    return 0;
}

unsigned char recon_b(unsigned char *recon, int r, int c, unsigned int stride) {
    if (r > 0) {
        return recon[(r - 1) * stride + c];
    }
    return 0;
}

unsigned char recon_c(unsigned char *recon, int r, int c, unsigned int bytesPerPixel, unsigned int stride) {
    if (r > 0 && c >= bytesPerPixel) {
        return recon[(r - 1) * stride + c - bytesPerPixel];
    }
    return 0;
}

void drawZoomedPixel(int x, int y, Color *color, int factor) {
    for (int y2 = 0; y2 < factor; y2++) {
        for (int x2 = 0; x2 < factor; x2++) {
            DrawPixel((x * factor) + x2, (y * factor) + y2, *color);
        }
    }
}

unsigned char paethPredictor(unsigned char a, unsigned char b, unsigned char c) {
    int p = a + b - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);
    if (pa <= pb && pa <= pc) {
        return a;
    }
    if (pb <= pc) {
        return b;
    }
    return c;
}

int main(int argc, char *argv[]) {
    const char *input;

    if (argc > 1) {
        input = argv[1]; // Use the first argument
    } else {
        input = "basn0g01.png"; // Use the default value
    }

    FILE *file = fopen(input, "rb"); // Open file in binary read mode
    if (!file) {
        perror("Unable to open file");
        return 1;
    }

    const unsigned char png_signature[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    unsigned char buffer[8];

    size_t bytesRead = fread(buffer, 1, 8, file);

    printf("File first 8 bytes: ");
    for (int i = 0; i < bytesRead && i < 8; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");

    if (bytesRead != 8) {
        printf("File too small to be a PNG.\n");
        return 1;
    }

    // no memcmp
    for (int i = 0; i < bytesRead && i < 8; i++) {
        if (buffer[i] != png_signature[i]) {
            printf("File is NOT a PNG file.\n");
            return 1;
        }
    }

    printf("File is a PNG file.\n");


    //if (memcmp(buffer, png_signature, 8) == 0) {
    //   printf("File is a PNG file.\n");
    //} else {
    //    printf("File is NOT a PNG file.\n");
    //}


    Chunk *head = NULL;

    while ((fread(buffer, 1, 8, file)) > 0) {
        unsigned int length = (buffer[0] << 24) |
                              (buffer[1] << 16) |
                              (buffer[2] << 8) |
                              (buffer[3]);

        unsigned char type[5] = {
            buffer[4],
            buffer[5],
            buffer[6],
            buffer[7],
            '\0',
        };

        //printf("length = %d \n", length);
        //printf("type %s \n", type);

        unsigned char *chunkData = malloc(length);
        size_t bytes_read = fread(chunkData, 1, length, file);
        if (bytes_read != length) {
            printf("Incorrect chunk data length.\n");
            return 1;
        }
        unsigned char crc[4];
        bytes_read = fread(crc, 1, 4, file);
        if (bytes_read != 4) {
            printf("Incorrect crc data length.\n");
            return 1;
        }

        head = add_chunk(head, length, type, chunkData, crc);

        free(chunkData);
    }

    Chunk *cur = head;
    while (cur) {
        printf("----------------\n");
        printf("Chunk type: %s\n", cur->type);
        printf("Chunk length: %d\n", cur->length);
        printf("Chunk data: \n");
        for (int i = 0; i < cur->length; i++) {
            printf("%02x ", cur->data[i]);
        }
        printf("\nChunk crc: \n");
        for (int i = 0; i < 4; i++) {
            printf("%02x ", cur->crc[i]);
        }
        printf("\n----------------\n");
        cur = cur->next;
    }

    unsigned char *headerData = head->data;
    unsigned int width = (headerData[0] << 24) |
                         (headerData[1] << 16) |
                         (headerData[2] << 8) |
                         (headerData[3]);

    unsigned int height = (headerData[4] << 24) |
                          (headerData[5] << 16) |
                          (headerData[6] << 8) |
                          (headerData[7]);

    unsigned char bitDepth = headerData[8];
    unsigned char colorType = headerData[9];
    unsigned char compressionMethod = headerData[10];
    unsigned char filterMethod = headerData[11];
    unsigned char interlaceMethod = headerData[12];

    printf("Image width %d \n", width);
    printf("Image height %d \n", height);
    printf("Bit depth %d \n", bitDepth);
    printf("Color Type %d \n", colorType);
    printf("Compression Method %d \n", compressionMethod);
    printf("Filter Method %d \n", filterMethod);
    printf("Interlace Method %d \n", interlaceMethod);

    if (compressionMethod != 0) {
        printf("Compression method not supported.\n");
        return 1;
    }

    if (filterMethod != 0) {
        printf("Filter method not supported.\n");
        return 1;
    }

    if (colorType != 0 && colorType != 6 && colorType != 3 && colorType != 2) {
        printf("Color type not supported %d.\n", colorType);
        return 1;
    }

    if (bitDepth != 1 && bitDepth != 4 && bitDepth != 8 && bitDepth != 16) {
        printf("Bit depth not supported.\n");
        return 1;
    }

    if (interlaceMethod != 0) {
        printf("Interlace method not supported.\n");
        return 1;
    }

    // color type 3 must have PLTE chunk
    Color *colorPalette = NULL;
    if (colorType == 3) {
        int numPalettes = 0;
        bool foundPLTE = false;
        Chunk *cur = head;
        while (cur) {
            if (cur->type[0] == 'P' && cur->type[1] == 'L' && cur->type[2] == 'T' && cur->type[3] == 'E') {
                foundPLTE = true;
                break;
            }
            cur = cur->next;
        }
        if (!foundPLTE) {
            printf("PLTE chunk not found.\n");
            return 1;
        }
        if (cur->length % 3 != 0) {
            printf("PLTE chunk length is not a multiple of 3.\n");
            return 1;
        }

        numPalettes = cur->length / 3;
        printf("Number of palettes: %d\n", numPalettes);
        colorPalette = malloc(numPalettes * sizeof(Color));
        int index = 0;
        for (int i = 0; i < cur->length; i += 3) {
            colorPalette[index].r = cur->data[i];
            colorPalette[index].g = cur->data[i + 1];
            colorPalette[index].b = cur->data[i + 2];
            colorPalette[index].a = 255;
            index++;
        }
    }


    int totalIdatLength = 0;
    Chunk *current = head;
    while (current != NULL) {
        if (current->type[0] == 'I' && current->type[1] == 'D' && current->type[2] == 'A' && current->type[3] == 'T') {
            totalIdatLength += current->length;
        }
        current = current->next; // Move to the next node
    }

    size_t offset = 0;
    unsigned char *idatData = malloc(totalIdatLength);
    current = head;
    while (current != NULL) {
        if (current->type[0] == 'I' && current->type[1] == 'D' && current->type[2] == 'A' && current->type[3] == 'T') {
            memcpy(idatData + offset, current->data, current->length);
            offset += current->length;
        }
        current = current->next; // Move to the next node
    }

    /*printf("IDAT data: ");
    for (int i = 0; i < totalIdatLength; i++) {
        printf("%02x ", idatData[i]);
    }*/

    double bytesPerPixel = colorType == 6 ? 4 : 1;
    if (colorType == 3) {
        if (bitDepth == 8) {
            bytesPerPixel = 1;
        } else if (bitDepth == 4) {
            bytesPerPixel = 0.5;
        }
    } else if (colorType == 6) {
        if (bitDepth == 8) {
            bytesPerPixel = 4;
        } else if (bitDepth == 16) {
            bytesPerPixel = 8;
        }
    } else if (colorType == 2) {
        if (bitDepth == 8) {
            bytesPerPixel = 3;
        } else if (bitDepth == 16) {
            bytesPerPixel = 6;
        }
    } else if (colorType == 0) {
        if (bitDepth == 1) {
            // 1 / 8
            bytesPerPixel = 0.125;
        }
    } else {
        printf("Color type (part 2) not supported %d.\n", colorType);
        return 1;
    }

    // Calculate the size needed for the decompressed data
    // For RGBA (color type 6) with 8-bit depth, each pixel needs 4 bytes
    // Plus 1 byte per scanline for the filter type
    // For color type 3, each byte is an index to the palette
    uLongf decompressedLength = height * (width * bytesPerPixel + 1);
    unsigned char *decompressedData = malloc(decompressedLength);

    // Decompress the data
    int result = uncompress(decompressedData, &decompressedLength,
                            idatData, totalIdatLength);

    if (result != Z_OK) {
        printf("Decompression failed with error code: %d\n", result);
        free(decompressedData);
        free(idatData);
        return 1;
    }

    printf("\nDecompressed data length: %lu\n", decompressedLength);

    /*
        printf("Decompressed data: ");
        for (int i = 0; i < decompressedLength; i++) {
            printf("%02x ", decompressedData[i]);
        }
        printf("\n");
    */

    unsigned int stride = width * bytesPerPixel;
    printf("stride: %d \n", stride);
    int reconIndex = 0;
    unsigned char recon[height * stride];
    for (int i = 0; i < height * stride; i++) {
        recon[i] = 0;
    }

    int i = 0;
    for (int y = 0; y < height; y++) {
        // for each scanline
        unsigned char filterType = decompressedData[i];
        //printf("height: %d has filterType: %d : i = %d \n", y, filterType, i);
        i += 1;
        for (int x = 0; x < stride; x++) {
            // for each byte in the scanline
            unsigned char filter_x = decompressedData[i];
            i += 1;
            unsigned char recon_x;
            if (filterType == 0) {
                recon_x = filter_x;
            } else if (filterType == 1) {
                recon_x = filter_x + recon_a(recon, y, x, bytesPerPixel, stride);
            } else if (filterType == 2) {
                recon_x = filter_x + recon_b(recon, y, x, stride);
            } else if (filterType == 3) {
                recon_x = filter_x + floor(
                              (recon_a(recon, y, x, bytesPerPixel, stride) + recon_b(recon, y, x, stride)) / 2);
            } else if (filterType == 4) {
                unsigned char recA = recon_a(recon, y, x, bytesPerPixel, stride);
                unsigned char recB = recon_b(recon, y, x, stride);
                unsigned char recC = recon_c(recon, y, x, bytesPerPixel, stride);
                recon_x = filter_x + paethPredictor(recA, recB, recC);
            } else {
                printf("Invalid filter type: %d. y = %d x = %d\n", filterType, y, x);
                return 1;
            }


            // printf("x %d = filter_x: %d : recon_x: %d \n", x, filter_x, recon_x);

            recon[reconIndex] = recon_x & 0xFF;
            reconIndex += 1;
        }
    }


    /*
    printf("pixel data: ");
    for (int i = 0; i < (height * stride); i++) {
        printf("%02x ", recon[i]);
    }
    printf("\n");
    */

    const int screenWidth = 800;
    const int screenHeight = 800;

    printf("Initializing raylib\n");

    SetTraceLogLevel(LOG_NONE); // Disable all logs
    InitWindow(screenWidth, screenHeight, "PNG Viewer");
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    int zoomFactor = 1;

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            if (wheel > 0) {
                if (zoomFactor <= 20) {
                    zoomFactor++;
                }
            } else {
                if (zoomFactor > 1) {
                    zoomFactor--;
                }
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);


        DrawText("Mouse wheel to zoom", 190, 200, 20, LIGHTGRAY);

        if (colorType == 0) {
            if (bitDepth == 1)
            {
                for (int y = 0; y < height; y++) {
                    int realX = 0;
                    for (int x = 0; x < stride; x++)
                    {
                        int pixelIndex = x + (y * stride);
                        int pixelValue = recon[pixelIndex];
                        // pixelValue contains 8 pixels
                        for (int j = 7; j >= 0; j--) {
                            unsigned char bit = (pixelValue >> j) & 1;

                            if (bit == 1) {
                                Color whiteColor = (Color){255, 255, 255, 255};
                                drawZoomedPixel(realX, y, &whiteColor, zoomFactor);
                            } else {
                                Color blackColor = (Color){0, 0, 0, 255};
                                drawZoomedPixel(realX, y, &blackColor, zoomFactor);
                            }
                            realX++;
                        }
                    }
                }
            }
        } else if (colorType == 6) {
            int index = 0;
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    unsigned char r = recon[index++];
                    unsigned char g = recon[index++];
                    unsigned char b = recon[index++];
                    unsigned char a = recon[index++];


                    Color pixelColor = (Color){r, g, b, a};
                    drawZoomedPixel(x, y, &pixelColor, zoomFactor);
                }
            }
        } else if (colorType == 3) {
            if (bitDepth == 8) {
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        int pixelIndex = x + y * width;
                        drawZoomedPixel(x, y, &colorPalette[recon[pixelIndex]], zoomFactor);
                    }
                }
            } else if (bitDepth == 4) {
                for (int y = 0; y < height; y++) {
                    int actualX = 0;
                    for (int x = 0; x < width / 2; x++) {
                        int pixelIndex = x + y * (width / 2);
                        unsigned char r = recon[pixelIndex];
                        // r contains two indices
                        unsigned char index1 = (r >> 4) & 0x0F;
                        unsigned char index2 = r & 0x0F;
                        drawZoomedPixel(actualX, y, &colorPalette[index1], zoomFactor);
                        actualX++;
                        drawZoomedPixel(actualX, y, &colorPalette[index2], zoomFactor);
                        actualX++;
                    }
                }
            }
        } else if (colorType == 2) {
            if (bitDepth == 8) {
                int index = 0;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        unsigned char r = recon[index++];
                        unsigned char g = recon[index++];
                        unsigned char b = recon[index++];

                        Color pixelColor = (Color){r, g, b, 255};
                        drawZoomedPixel(x, y, &pixelColor, zoomFactor);
                    }
                }
            } else {
                int index = 0;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        uint8_t r = (recon[index++] << 8 | recon[index++]) >> 8;
                        uint8_t g = (recon[index++] << 8 | recon[index++]) >> 8;
                        uint8_t b = (recon[index++] << 8 | recon[index++]) >> 8;


                        Color pixelColor = (Color){r, g, b, 255};
                        drawZoomedPixel(x, y, &pixelColor, zoomFactor);
                    }
                }
            }
        }


        EndDrawing();
    }

    CloseWindow(); // Close window and OpenGL context

    if (colorPalette) {
        printf("Cleaning up palette\n");
        free(colorPalette);
    }

    free(decompressedData);
    free(idatData);
    // Clean up
    free_chunks(head);
    fclose(file);

    return 0;
}
