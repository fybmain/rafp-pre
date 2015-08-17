#include "SDL/SDL.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(int argc,char *args[]){
	SDL_Surface *image,*window;
	SDL_Event event;
	int quit=0;

	if(SDL_Init(SDL_INIT_EVERYTHING)==-1)return -1;

	window=SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT,32,SDL_SWSURFACE);
	if(!window)return -2;
	SDL_WM_SetCaption("Event Driven Programming Test",NULL);

	image=SDL_LoadBMP("hello.bmp");
	if(!image)return -3;
	SDL_BlitSurface(image,NULL,window,NULL);
	SDL_FreeSurface(image);

	if(SDL_Flip(window)==-1)return -4;

	/*
		for the first time,i only used SDL_PollEvent() like samples in most tutorials.
		but the cpu usage is really thrilling!
		SDL_PollEvent() will return whether there is really an event or not.
		if i only use SDL_PollEvent(),the cpu will keep running in the "while(){...}".
	*/
	while(quit?SDL_PollEvent(&event):SDL_WaitEvent(&event)){
		if(event.type==SDL_QUIT)quit=1;
	}

	SDL_Quit();
	return 0;
}

