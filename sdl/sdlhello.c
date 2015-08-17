#include "SDL/SDL.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(int argc,char* args[]){
	SDL_Surface* image=NULL;
	SDL_Surface* window=NULL;

	SDL_Init(SDL_INIT_EVERYTHING);

	window=SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT,32,SDL_SWSURFACE);
	image=SDL_LoadBMP("hello.bmp");
	SDL_BlitSurface(image,NULL,window,NULL);
	SDL_FreeSurface(image);

	SDL_Flip(window);

	SDL_Delay(2000);
	SDL_Quit();
	return 0;
}

