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

int main(int argc, char* argv[])
{
    FILE* file = fopen("basn6a08.png", "rb"); // Open file in binary read mode
    if (!file)
    {
        perror("Unable to open file");
        return 1;
    }

    const unsigned char png_signature[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    unsigned char buffer[8];

    size_t bytesRead = fread(buffer, 1, 8, file);

    if (bytesRead != 8) {
        printf("File too small to be a PNG.\n");
        return 1;
    }

    if (memcmp(buffer, png_signature, 8) == 0) {
        printf("File is a PNG file.\n");
    } else {
        printf("File is NOT a PNG file.\n");
    }

    return 0;

    /*
    // Find file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    // Allocate buffer for file contents
    unsigned char* buffer = malloc(size);
    if (!buffer)
    {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }

    // Read file into buffer
    size_t read = fread(buffer, 1, size, file);
    if (read != size)
    {
        perror("Failed to read file");
        free(buffer);
        fclose(file);
        return 1;
    }

    // check for .PNG header


    // Use the buffer (example: print first 8 bytes as hex)
    for (int i = 0; i < size && i < 8; i++)
    {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
    */

    // Clean up
    free(buffer);
    fclose(file);

    return 0;
}
