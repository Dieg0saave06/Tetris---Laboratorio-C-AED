#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include "juego.h"
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#define SIZE_ITEMS 3
#define MAIN_MENU_ITEMS 3

const char *sizeOptions[SIZE_ITEMS] = {""};
const char *mainMenuOptions[MAIN_MENU_ITEMS] = {"JUGAR", "INTEGRANTES", "SALIR"};
int selectedSize = 0;
int selectedMainMenu = 0; // Variable para el menu principal
SDL_Texture *cornerImage;

void renderSizeMenu(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Color color = {255, 255, 255}; // Color blanco
    SDL_Color selectedColor = {255, 0, 0}; // Color rojo para la opcion seleccionada

    for (int i = 0; i < SIZE_ITEMS; i++) {
        SDL_Surface *surface = TTF_RenderText_Solid(font, sizeOptions[i], (i == selectedSize) ? selectedColor : color);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        int width, height;
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
        SDL_Rect dstrect = {100, 100 + i * 50, width, height}; // Ajusta la posicion

        SDL_RenderCopy(renderer, texture, NULL, &dstrect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}


void mostrarIntegrantes(SDL_Renderer *renderer, TTF_Font *font) {
    bool showingIntegrantes = true;
    SDL_Event event;

    while (showingIntegrantes) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                // Salir del programa
                exit(0);
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_RETURN) {
                    // Salir de la pantalla de integrantes y regresar al menú principal
                    showingIntegrantes = false;
                }
            }
        }

        // Limpiar pantalla
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fondo negro
        SDL_RenderClear(renderer);

        // Renderizar los nombres de los integrantes
        SDL_Color color = {255, 255, 255}; // Blanco
        const char *integrantes[] = {
            "SAAVEDRA, DIEGO GABRIEL",
            "ARECHAVALA, SANTINO",
            "SALEH, MAXIMO",
            "NUEVO, JOAQUIN RODRIGO"
        };

        for (int i = 0; i < 4; i++) {
            SDL_Surface *surface = TTF_RenderText_Solid(font, integrantes[i], color);
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            int width, height;
            SDL_QueryTexture(texture, NULL, NULL, &width, &height);
            SDL_Rect dstrect = {100, 100 + i * 50, width, height};
            SDL_RenderCopy(renderer, texture, NULL, &dstrect);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);
        }

        // Renderizar un mensaje para regresar al menú principal
        SDL_Surface *surface = TTF_RenderText_Solid(font, "Presiona ESC o Enter para volver", color);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        int width, height;
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
        SDL_Rect dstrect = {100, 300, width, height};
        SDL_RenderCopy(renderer, texture, NULL, &dstrect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);

        // Presentar renderizado
        SDL_RenderPresent(renderer);
    }
}



void renderMainMenu(SDL_Renderer *renderer, TTF_Font *font) {
    // Renderizar el título
    TTF_Font *titleFont = TTF_OpenFont("fuentes/fuentin.ttf", 48); // Fuente más grande para el título
    if (!titleFont) {
        printf("Error al cargar la fuente del título: %s\n", TTF_GetError());
        return;
    }

    SDL_Color titleColor = {255, 255, 255}; // Blanco para el título
    SDL_Surface *titleSurface = TTF_RenderText_Solid(titleFont, "TETRIS", titleColor);
    SDL_Texture *titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    int titleWidth, titleHeight;
    SDL_QueryTexture(titleTexture, NULL, NULL, &titleWidth, &titleHeight);

    SDL_Rect titleRect = {320 - titleWidth / 2, 50, titleWidth, titleHeight}; // Centrado horizontalmente
    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);

    SDL_DestroyTexture(titleTexture);
    SDL_FreeSurface(titleSurface);
    TTF_CloseFont(titleFont); // Liberar la fuente del título

    // Renderizar las opciones del menú alineadas a la izquierda
    SDL_Color color = {255, 255, 255};      // Color blanco
    SDL_Color selectedColor = {255, 0, 0}; // Rojo para la opción seleccionada

    for (int i = 0; i < MAIN_MENU_ITEMS; i++) {
        SDL_Surface *surface = TTF_RenderText_Solid(font, mainMenuOptions[i], (i == selectedMainMenu) ? selectedColor : color);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        int width, height;
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
        SDL_Rect dstrect = {50, 200 + i * 50, width, height}; // Alineado a la izquierda (x = 50)
        SDL_RenderCopy(renderer, texture, NULL, &dstrect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}


int main(int argc, char *argv[]) {
    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error al inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Inicializar TTF
    if (TTF_Init() == -1) {
        printf("Error al inicializar TTF: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Inicializar SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Error al inicializar SDL_mixer: %s\n", Mix_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
}

    // Inicializar SDL_image
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("Error al inicializar SDL_image: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    // Crear ventana y renderer
    SDL_Window *window = SDL_CreateWindow("Menu Principal", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Cargar la imagen
    SDL_Surface *surface = IMG_Load("imagenes/UTN.png");
    if (!surface) {
        printf("Error al cargar la imagen: %s\n", IMG_GetError());
    } else {
        cornerImage = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface); // Liberar la superficie después de crear la textura
}
    // Cargar fuente
    TTF_Font *font = TTF_OpenFont("fuentes/fuentin.ttf", 24);
    if (!font) {
        printf("Error al cargar la fuente: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    bool running = true;
    bool inSizeMenu = false; // Nueva variable para el estado del menu de tamano
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;  // Cierra el programa
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_UP) {
                    selectedMainMenu = (selectedMainMenu == 0) ? MAIN_MENU_ITEMS - 1 : selectedMainMenu - 1;
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    selectedMainMenu = (selectedMainMenu + 1) % MAIN_MENU_ITEMS;
                } else if (event.key.keysym.sym == SDLK_RETURN) {
                    if (selectedMainMenu == 0) {
                        iniciarJuego(renderer); // Iniciar el juego
                    } else if (selectedMainMenu == 1) {
                        mostrarIntegrantes(renderer, font); // Mostrar información de los integrantes
                    } else if (selectedMainMenu == 2) {
                        printf("ADIOS!!!\n");
                        running = false; // Salir del programa
                    }
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    printf("ADIOS!!!\n");
                    running = false; // Si se presiona ESC en el menú, cerrar el programa
                }
            }
        }
        // Limpiar la pantalla
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Color de fondo negro
        SDL_RenderClear(renderer);

        int imgWidth, imgHeight;
    SDL_QueryTexture(cornerImage, NULL, NULL, &imgWidth, &imgHeight);

    // Renderizar imagen
    SDL_Rect dstrect1 = {0, 0, imgWidth, imgHeight};
    SDL_RenderCopy(renderer, cornerImage, NULL, &dstrect1);

        // Renderizar el menu
        if (inSizeMenu) {
            renderSizeMenu(renderer, font);
        } else {
            renderMainMenu(renderer, font);
        }

        // Presentar el renderizado
        SDL_RenderPresent(renderer);
    }

    // Limpiar recursos
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    SDL_DestroyTexture(cornerImage);
    return 0;
}
