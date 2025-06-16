/*
#include <SDL.h>

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Red Pixel",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          500, 500, SDL_WINDOW_SHOWN);

    if (!window)
    {
        SDL_Log("Could not create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_bool done = SDL_FALSE;

    while (!done)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                done = SDL_TRUE;
                break;
            }
        }

        // Set render draw color to white and clear the screen.
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Set draw color to red.
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

        // Draw a 50x50 red pixel (rectangle) at (25, 25).
        SDL_Rect rect = {25, 25, 50, 50};
        SDL_RenderFillRect(renderer, &rect);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

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

    /*
    if (memcmp(buffer, png_signature, 8) == 0) {
        printf("File is a PNG file.\n");
    } else {
        printf("File is NOT a PNG file.\n");
    }
    */

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
