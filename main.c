// https://pyokagan.name/blog/2019-10-14-png/
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <math.h>

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
        input = "basn6a08.png"; // Use the default value
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

    if (compressionMethod != 0) {
        printf("Compression method not supported.\n");
        return 1;
    }

    if (filterMethod != 0) {
        printf("Filter method not supported.\n");
        return 1;
    }

    if (colorType != 6) {
        printf("Color type not supported %d.\n", colorType);
        return 1;
    }

    if (bitDepth != 8) {
        printf("Bit depth not supported.\n");
        return 1;
    }

    if (interlaceMethod != 0) {
        printf("Interlace method not supported.\n");
        return 1;
    }

    printf("Image width %d \n", width);
    printf("Image height %d \n", height);
    printf("Bit depth %d \n", bitDepth);
    printf("Color Type %d \n", colorType);
    printf("Compression Method %d \n", compressionMethod);
    printf("Filter Method %d \n", filterMethod);
    printf("Interlace Method %d \n", interlaceMethod);

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

    printf("IDAT data: ");
    for (int i = 0; i < totalIdatLength; i++) {
        printf("%02x ", idatData[i]);
    }

    // Calculate the size needed for the decompressed data
    // For RGBA (color type 6) with 8-bit depth, each pixel needs 4 bytes
    // Plus 1 byte per scanline for the filter type
    uLongf decompressedLength = height * (width * 4 + 1);
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

    printf("Decompressed data: ");
    for (int i = 0; i < decompressedLength; i++) {
        printf("%02x ", decompressedData[i]);
    }
    printf("\n");

    unsigned int bytesPerPixel = 4;
    unsigned int stride = width * bytesPerPixel;
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
                printf("Invalid filter type: %d\n", filterType);
                return 1;
            }


             // printf("x %d = filter_x: %d : recon_x: %d \n", x, filter_x, recon_x);

            recon[reconIndex] = recon_x & 0xFF;
            reconIndex += 1;
        }
    }

    printf("pixel data: ");
    for (int i = 0; i < (height * stride); i++) {
        printf("%02x ", recon[i]);
    }
    printf("\n");

    const int screenWidth = 800;
    const int screenHeight = 450;

    SetTraceLogLevel(LOG_NONE); // Disable all logs
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        int index = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                unsigned char r = recon[index++];
                unsigned char g = recon[index++];
                unsigned char b = recon[index++];
                unsigned char a = recon[index++];


                Color pixelColor = (Color){r, g, b, a};
                DrawPixel(x, y, pixelColor);
            }
        }

        EndDrawing();
    }

    CloseWindow(); // Close window and OpenGL context

    free(decompressedData);
    free(idatData);
    // Clean up
    free_chunks(head);
    fclose(file);

    return 0;
}
