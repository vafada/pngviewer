/*
#include "raylib.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        //DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        for (int x = 10; x < 100; x++) {
            for (int y = 10; y < 100; y++) {
                DrawPixel(x, y, MAROON);
            }
        }


        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


int main(int argc, char *argv[]) {
    FILE *file = fopen("basn6a08.png", "rb"); // Open file in binary read mode
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

    // Clean up
    free_chunks(head);
    fclose(file);

    return 0;
}
