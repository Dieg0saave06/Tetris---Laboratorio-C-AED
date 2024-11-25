#ifndef JUEGO_H
#define JUEGO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>


#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define CELL_SIZE 30


#define CELL_SIZE 30 // Tama�o de cada celda en p�xeles
#define PIECE_SIZE 4 // Tama�o de las piezas (constante para todos los tama�os)

// Declaraciones de funciones
void iniciarJuego();
void gameLoop(SDL_Renderer* renderer);
void dibujarTablero(SDL_Renderer* renderer);
void inicializarTablero(int width, int height);
void generarPieza();
void moverPieza(int dx, int dy);
void rotarPieza();
int colision(int dx, int dy);
void colocarPieza();


#endif // JUEGO_H_INCLUDED
