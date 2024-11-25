#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include "juego.h"

// Variables globales
int board[30][20]; // Tablero maximo
int boardWidth;
int boardHeight;
int currentPiece[4][4]; // Pieza actual
int pieceX, pieceY; // Posicion de la pieza actual
int juegoPausado = 0; // 0: no pausado, 1: pausado
int puntuacion = 0; // Variable global para la puntuación
int filasRestantes = 10; // Contador inicial de filas
int velocidad = 500; // Velocidad inicial de caída en milisegundos
int incrementoFilas = 10; // Incremento en el número de filas después de cada nivel
int randomIndex; // Variable global para almacenar el índice de la pieza aleatoria
SDL_Color currentPieceColor; // Variable global para almacenar el color de la pieza actual
Uint32 lastDropTime; // Declarar la variable
Uint32 startTime; // Tiempo de inicio del juego
Uint32 elapsedTime; // Tiempo transcurrido en segundos
Uint32 pauseStartTime = 0; // Tiempo de inicio de la pausa
Uint32 pausedTime = 0; // Tiempo acumulado en pausa
const int dropInterval = 500; // Intervalo de caída en milisegundos (500 ms = 0.5 segundos)
int nextPiece[4][4]; // Siguiente pieza
SDL_Color nextPieceColor; // Color de la siguiente pieza

Mix_Music *backgroundMusic; // Variable global para la música de fondo
Mix_Chunk *rotateSound;
Mix_Chunk *placeSound;
Mix_Chunk *clearLineSound;

// Colores para las piezas
SDL_Color PIECE_COLORS[7] = {
    {255, 0, 0, 255}, // I - Rojo
    {255, 255, 0, 255}, // O - Amarillo
    {128, 0, 128, 255}, // T - Morado
    {0, 255, 0, 255}, // L - Verde
    {0, 0, 255, 255}, // J - Azul
    {255, 165, 0, 255}, // S - Naranja
    {255, 0, 255, 255} // Z - Magenta
};

// Formas de las piezas
const int PIECES[7][4][4] = {
    {{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // I
    {{1, 1}, {1, 1}, {0, 0}, {0, 0}}, // O
    {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // T
    {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // L
    {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // J
    {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // S
    {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}  // Z
};


SDL_Color boardColors[30][20]; // Matriz para almacenar colores de las celdas
// Inicializa el tablero
void inicializarTablero(int width, int height) {
    boardWidth = width;
    boardHeight = height;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            board[i][j] = 0; // Inicializa el tablero vacío
            boardColors[i][j].r = 0;   // Inicializa el componente rojo a 0
            boardColors[i][j].g = 0;   // Inicializa el componente verde a 0
            boardColors[i][j].b = 0;   // Inicializa el componente azul a 0
            boardColors[i][j].a = 255;  // Inicializa el componente alfa a 255 (opaco)
        }
    }
}

// Genera una nueva pieza
void generarPieza() {
    static int initialized = 0;

    if (!initialized) {
        // Inicializar la primera pieza y la siguiente pieza
        int randomIndex = rand() % 7;
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                currentPiece[y][x] = PIECES[randomIndex][y][x];
            }
        }
        currentPieceColor = PIECE_COLORS[randomIndex];

        randomIndex = rand() % 7;
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                nextPiece[y][x] = PIECES[randomIndex][y][x];
            }
        }
        nextPieceColor = PIECE_COLORS[randomIndex];

        initialized = 1;
    } else {
        // Mover la siguiente pieza a la pieza actual
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                currentPiece[y][x] = nextPiece[y][x];
            }
        }
        currentPieceColor = nextPieceColor;

        // Generar nueva siguiente pieza
        int randomIndex = rand() % 7;
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                nextPiece[y][x] = PIECES[randomIndex][y][x];
            }
        }
        nextPieceColor = PIECE_COLORS[randomIndex];
    }

    // Coloca la nueva pieza en la parte superior del tablero
    pieceX = boardWidth / 2 - 2;  // Centrado horizontalmente
    pieceY = 0;                   // Empieza en la parte superior

    // Verifica si la pieza generada colisiona inmediatamente
    if (checkCollision(0, 0)) {
        printf("Juego Terminado!!! Las piezas alcanzaron la parte superior.\n");
        printf("Tu puntuacion final fue: %d\n", puntuacion);
        exit(0);
    }
}


void dibujarSiguientePieza(SDL_Renderer* renderer) {
    // Colores de fondo para el área de vista previa
    SDL_Color backgroundColor = {50, 50, 50, 255}; // Un gris oscuro como fondo
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);

    SDL_Rect previewRect = {640, 0, 160, 160}; // Área de vista previa
    SDL_RenderFillRect(renderer, &previewRect);

    // Añadir un borde blanco al área de vista previa
    SDL_Color borderColor = {255, 255, 255, 255}; // Blanco para el borde
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &previewRect);

    // Dibujar la siguiente pieza con el mismo color y textura que las piezas del tablero
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (nextPiece[y][x]) {
                // Dibujar pequeños cuadrados dentro de la celda para crear una "textura"
                int smallCellSize = CELL_SIZE / 4;
                SDL_Rect rect = { 660 + x * CELL_SIZE + (CELL_SIZE / 2), 20 + y * CELL_SIZE + (CELL_SIZE / 2), CELL_SIZE, CELL_SIZE };
                for (int smallY = 0; smallY < 4; smallY++) {
                    for (int smallX = 0; smallX < 4; smallX++) {
                        SDL_SetRenderDrawColor(renderer,
                            nextPieceColor.r * (smallX + 1) / 4, // Ajusta el componente rojo
                            nextPieceColor.g * (smallY + 1) / 4, // Ajusta el componente verde
                            nextPieceColor.b,                    // Mantén el componente azul
                            255); // Completamente opaco

                        SDL_Rect smallRect = {
                            rect.x + smallX * smallCellSize,
                            rect.y + smallY * smallCellSize,
                            smallCellSize,
                            smallCellSize
                        };
                        SDL_RenderFillRect(renderer, &smallRect);
                    }
                }
            }
        }
    }
}

// Verifica la colision
int checkCollision(int offsetX, int offsetY) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentPiece[y][x]) { // Si hay un bloque en la pieza
                int newX = pieceX + x + offsetX;
                int newY = pieceY + y + offsetY;

                // Verificar colisión con los bordes del tablero
                if (newX < 0 || newX >= boardWidth || newY >= boardHeight) {
                    return 1; // Hay colisión
                }

                // Verificar colisión con piezas ya colocadas
                if (newY >= 0 && board[newY][newX]) {
                    return 1; // Hay colisión
                }
            }
        }
    }
    return 0; // No hay colisión
}


// Verifica si la pieza ha llegado al tope del tablero
int checkGameOver() {
    for (int x = 0; x < boardWidth; x++) {
        if (board[0][x] != 0) {
            return 1; // Juego terminado
        }
    }
    return 0; // Juego en curso
}

// Elimina filas completas
void eliminarFilasCompletas() {
    for (int y = boardHeight - 1; y >= 0; y--) {
        int filaCompleta = 1; // Suponemos que la fila está completa

        for (int x = 0; x < boardWidth; x++) {
            if (board[y][x] == 0) {
                filaCompleta = 0; // Hay al menos un espacio vacío
                break;
            }
        }

        if (filaCompleta) {
            // Reproducir sonido de eliminación de fila
            Mix_PlayChannel(-1, clearLineSound, 0);

            // Incrementar la puntuación
            puntuacion += 100; // Incrementa 100 puntos por cada fila eliminada

            // Eliminar la fila completa
            for (int i = y; i > 0; i--) {
                for (int x = 0; x < boardWidth; x++) {
                    board[i][x] = board[i - 1][x]; // Mover la fila hacia abajo
                }
            }
            // Limpiar la fila superior
            for (int x = 0; x < boardWidth; x++) {
                board[0][x] = 0;
            }
            y++; // Verifica la misma fila nuevamente

            // Actualizar contador de filas
            actualizarContadorFilas();
        }
    }
}

void actualizarContadorFilas() {
    filasRestantes--;
    if (filasRestantes == 0) {
        incrementoFilas += 10;
        filasRestantes = incrementoFilas;
        velocidad = velocidad * 0.9; // Incrementa la velocidad reduciendo el intervalo
        if (velocidad < 50) {
            velocidad = 50; // Velocidad mínima para evitar que sea demasiado rápido
        }
    }
}

void dibujarContadorFilas(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Color color = {64, 224, 208, 255}; // Color turquesa

    char filasTexto[25];
    sprintf(filasTexto, "Filas Restantes: %d", filasRestantes);

    SDL_Surface *surface = TTF_RenderText_Solid(font, filasTexto, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Rect dstrect = {320, 210, width, height}; // Ajusta la posición según sea necesario

    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

// Coloca la pieza en el tablero
void placePieceOnBoard() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentPiece[y][x]) {
                int boardY = pieceY + y;
                board[boardY][pieceX + x] = 1; // Marca la celda en el tablero
                boardColors[boardY][pieceX + x] = currentPieceColor; // Asigna el color a la celda del tablero
            }
        }
    }

    eliminarFilasCompletas(); // Verifica y elimina filas completas
    generarPieza();          // Genera una nueva pieza
    Mix_PlayChannel(-1, rotateSound, 0);
}

void dibujarTiempo(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Color color = {64, 224, 208, 255}; // Color turquesa
    Uint32 currentTime = SDL_GetTicks();
    Uint32 secondsElapsed = (currentTime - startTime - pausedTime) / 1000; // Tiempo en segundos descontando el tiempo en pausa

    if (secondsElapsed > elapsedTime) {
        elapsedTime = secondsElapsed; // Actualiza el tiempo transcurrido
    }

    char tiempoTexto[25];
    sprintf(tiempoTexto, "Tiempo Transcurrido: %d seg", elapsedTime); // Tiempo transcurrido en segundos

    SDL_Surface *surface = TTF_RenderText_Solid(font, tiempoTexto, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Rect dstrect = {300, 310, width, height}; // Posición más a la izquierda

    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

// Dibuja el tablero
void dibujarTablero(SDL_Renderer* renderer) {
    int smallCellSize = CELL_SIZE / 4; // Tamaño de los cuadrados pequeños
    for (int y = 0; y < boardHeight; y++) {
        for (int x = 0; x < boardWidth; x++) {
            SDL_Rect rect = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            if (board[y][x]) {
                // Dibuja pequeños cuadrados dentro de la celda para crear una "textura"
                for (int smallY = 0; smallY < 4; smallY++) {
                    for (int smallX = 0; smallX < 4; smallX++) {
                        // Cambia el color ligeramente para cada pequeño cuadrado
                        SDL_SetRenderDrawColor(renderer,
                            boardColors[y][x].r * (smallX + 1) / 4, // Ajusta el componente rojo
                            boardColors[y][x].g * (smallY + 1) / 4, // Ajusta el componente verde
                            boardColors[y][x].b,                     // Mantén el componente azul
                            255); // Completamente opaco

                        SDL_Rect smallRect = {
                            rect.x + smallX * smallCellSize,
                            rect.y + smallY * smallCellSize,
                            smallCellSize,
                            smallCellSize
                        };
                        SDL_RenderFillRect(renderer, &smallRect);
                    }
                }
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Color negro para las celdas vacías
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void dibujarCuadricula(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Color gris oscuro para la cuadrícula

    // Dibujar líneas verticales
    for (int x = 0; x <= boardWidth; x++) {
        SDL_RenderDrawLine(renderer, x * CELL_SIZE, 0, x * CELL_SIZE, boardHeight * CELL_SIZE);
    }

    // Dibujar líneas horizontales
    for (int y = 0; y <= boardHeight; y++) {
        SDL_RenderDrawLine(renderer, 0, y * CELL_SIZE, boardWidth * CELL_SIZE, y * CELL_SIZE);
    }
}


// Dibuja la pieza actual
void dibujarPieza(SDL_Renderer* renderer) {
    int smallCellSize = CELL_SIZE / 4; // Tamaño de los cuadrados pequeños
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentPiece[y][x]) {
                int pieceXPos = (pieceX + x) * CELL_SIZE;
                int pieceYPos = (pieceY + y) * CELL_SIZE;

                // Dibuja pequeños cuadrados dentro de la celda para crear una "textura"
                for (int smallY = 0; smallY < 4; smallY++) {
                    for (int smallX = 0; smallX < 4; smallX++) {
                        // Cambia el color ligeramente para cada pequeño cuadrado
                        SDL_SetRenderDrawColor(renderer,
                            currentPieceColor.r * (smallX + 1) / 4, // Ajusta el componente rojo
                            currentPieceColor.g * (smallY + 1) / 4, // Ajusta el componente verde
                            currentPieceColor.b,                    // Mantén el componente azul
                            255); // Completamente opaco

                        SDL_Rect smallRect = {
                            pieceXPos + smallX * smallCellSize,
                            pieceYPos + smallY * smallCellSize,
                            smallCellSize,
                            smallCellSize
                        };
                        SDL_RenderFillRect(renderer, &smallRect);
                    }
                }
            }
        }
    }
}

void dibujarSombra(SDL_Renderer* renderer) {
    int shadowPieceY = pieceY;

    // Mueve la sombra hacia abajo hasta que colisione
    while (!checkCollision(0, shadowPieceY - pieceY + 1)) {
        shadowPieceY++;
    }

    // Dibuja la sombra con un color más oscuro/transparente
    SDL_SetRenderDrawColor(renderer, currentPieceColor.r / 2, currentPieceColor.g / 2, currentPieceColor.b / 2, 128);

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentPiece[y][x]) {
                SDL_Rect rect = { (pieceX + x) * CELL_SIZE, (shadowPieceY + y) * CELL_SIZE, CELL_SIZE, CELL_SIZE };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void dibujarContorno(SDL_Renderer* renderer) {
    // Definir el color del contorno (blanco en este caso)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Dibujar los rectángulos del contorno
    SDL_Rect topRect = { 0, 0, boardWidth * CELL_SIZE, 5 }; // Parte superior
    SDL_Rect bottomRect = { 0, boardHeight * CELL_SIZE, boardWidth * CELL_SIZE, 5 }; // Parte inferior
    SDL_Rect leftRect = { 0, 0, 5, boardHeight * CELL_SIZE + 5 }; // Lado izquierdo
    SDL_Rect rightRect = { boardWidth * CELL_SIZE - 5, 0, 5, boardHeight * CELL_SIZE + 5 }; // Lado derecho

    SDL_RenderFillRect(renderer, &topRect);
    SDL_RenderFillRect(renderer, &bottomRect);
    SDL_RenderFillRect(renderer, &leftRect);
    SDL_RenderFillRect(renderer, &rightRect);
}

// Actualiza la posicion de la pieza
void updatePiece() {
    pieceY++; // Intenta mover la pieza hacia abajo
    if (checkCollision(0, 0)) { // Verifica si hay colisión en la nueva posición
        pieceY--;             // Revierte el movimiento si hay colisión
        placePieceOnBoard();  // Coloca la pieza en el tablero
    }
}

void rotatePiece() {

    int tempPiece[4][4] = {0}; // Matriz temporal para almacenar la pieza rotada

    // Rota la pieza 90 grados en sentido horario
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            tempPiece[x][3 - y] = currentPiece[y][x];
        }
    }

    // Verifica si la rotación causa colisión o se sale de los límites
    int offsetX = 0; // Desplazamiento en X para ajustar la rotación si es necesario
    if (pieceX < 0) {
        offsetX = -pieceX; // Ajusta hacia la derecha si está fuera del tablero por la izquierda
    } else if (pieceX + 3 >= boardWidth) {
        offsetX = boardWidth - (pieceX + 4); // Ajusta hacia la izquierda si está fuera por la derecha
    }

    if (!checkCollision(offsetX, 0)) {
        // Si no hay colisión, aplica la rotación y el desplazamiento
        pieceX += offsetX;
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                currentPiece[y][x] = tempPiece[y][x];
            }
        }
    }
   Mix_PlayChannel(-1, rotateSound, 0);
}



void cargarSonidos() {
    rotateSound = Mix_LoadWAV("sonidos/rotar_pieza.wav");
    placeSound = Mix_LoadWAV("sonidos/colocar_pieza.wav");
    clearLineSound = Mix_LoadWAV("sonidos/elimina_fila.wav");

    if (!rotateSound || !placeSound || !clearLineSound) {
        printf("Error al cargar los sonidos: %s\n", Mix_GetError());
    }
}

void dibujarPuntuacion(SDL_Renderer* renderer, TTF_Font *font) {
    // Dibujar el texto de la puntuación
    SDL_Color color = {255, 255, 255}; // Color blanco
    char puntuacionTexto[20];
    sprintf(puntuacionTexto, "Puntos: %d", puntuacion); // Formatea la puntuación

    SDL_Surface *surface = TTF_RenderText_Solid(font, puntuacionTexto, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Rect dstrect = {400, 50, width, height}; // Posición en la pantalla, desplazada hacia abajo

    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void renderizarMensajePausa(SDL_Renderer* renderer, TTF_Font *font) {
    SDL_Color color = {255, 255, 255}; // Color blanco
    SDL_Surface *surface = TTF_RenderText_Solid(font, "PAUSA", color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Rect dstrect = { 98, (480 - height) / 2, width, height }; // Ajustar la posición a la izquierda

    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

// Bucle principal del juego
void gameLoop(SDL_Renderer *renderer) {
    SDL_Event event;
    int running = 1;
    TTF_Font *font = TTF_OpenFont("fuentes/letrita.ttf", 30); // Cargar fuente con tamaño más pequeño

    generarPieza(); // Genera la primera pieza
    startTime = SDL_GetTicks(); // Inicializar el tiempo de inicio del juego
    Uint32 lastDropTime = SDL_GetTicks(); // Tiempo de la última caída

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0); // Salir completamente del programa
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        if (!checkCollision(-1, 0) && !juegoPausado) pieceX--; // Mover a la izquierda
                        break;
                    case SDLK_RIGHT:
                        if (!checkCollision(1, 0) && !juegoPausado) pieceX++; // Mover a la derecha
                        break;
                    case SDLK_DOWN:
                        if (!juegoPausado) updatePiece(); // Acelerar caída
                        break;
                    case SDLK_UP:
                        if (!juegoPausado) rotatePiece(); // Rotar la pieza
                        break;
                    case SDLK_p: // Pausar el juego
                        juegoPausado = !juegoPausado; // Cambiar estado de pausa
                        if (juegoPausado) {
                            Mix_PauseMusic(); // Pausar música
                            pauseStartTime = SDL_GetTicks(); // Registrar el inicio de la pausa
                        } else {
                            Mix_ResumeMusic(); // Reanudar música
                            pausedTime += SDL_GetTicks() - pauseStartTime; // Actualizar el tiempo acumulado en pausa
                        }
                        break;
                    case SDLK_ESCAPE: // Regresar al menú principal
                        running = 0; // Salir del bucle de juego
                        break;
                }
            }
        }

        // Limpiar la pantalla
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Color de fondo negro
        SDL_RenderClear(renderer);

        // Dibujar los elementos del juego
        dibujarTablero(renderer);
        dibujarCuadricula(renderer);
        dibujarContorno(renderer);
        dibujarSombra(renderer);
        dibujarPieza(renderer);
        dibujarSiguientePieza(renderer);
        dibujarPuntuacion(renderer, font);
        dibujarTiempo(renderer, font);
        dibujarContadorFilas(renderer, font);

        // Si el juego está pausado, dibuja el mensaje de pausa
        if (juegoPausado) {
            renderizarMensajePausa(renderer, font);
        } else {
            // Controla la caída de la pieza
            Uint32 currentTime = SDL_GetTicks(); // Obtiene el tiempo actual
            if (currentTime - lastDropTime > velocidad) {
                updatePiece(); // Actualiza la posición de la pieza
                lastDropTime = currentTime; // Reinicia el temporizador
            }
        }

        SDL_RenderPresent(renderer); // Presenta el renderizado
    }

    TTF_CloseFont(font); // Cerrar la fuente al final del bucle
}

// Funcion principal para iniciar el juego
void iniciarJuego() {
    int width = BOARD_WIDTH; // Establecer el ancho del tablero
    int height = BOARD_HEIGHT; // Establecer la altura del tablero

    // Inicializar el tablero
    inicializarTablero(width, height);

    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error al inicializar SDL: %s\n", SDL_GetError());
        return;
    }

    // Cargar sonidos
    cargarSonidos();

    // Cargar música de fondo
    backgroundMusic = Mix_LoadMUS("sonidos/te.mp3"); // Asegúrate de usar la ruta correcta
    if (!backgroundMusic) {
        printf("Error al cargar la música: %s\n", Mix_GetError());
    } else {
        Mix_PlayMusic(backgroundMusic, -1); // Reproduce la música en bucle
    }

    SDL_Window* window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN); // Aumentamos el ancho a 800

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Inicializar el tiempo de inicio y tiempo transcurrido
    startTime = SDL_GetTicks();
    elapsedTime = 0;

    // Iniciar el bucle del juego
    gameLoop(renderer);

    // Liberar recursos
    Mix_FreeMusic(backgroundMusic); // Libera la música al final
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
