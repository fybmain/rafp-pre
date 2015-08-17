#include "SDL/SDL.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define COLOR_DEPTH 32

SDL_Surface* load_image(char *filename){
	SDL_Surface* loadimage=NULL;
	SDL_Surface* optimage=NULL;	//loaded image and the optimized image

	if((loadimage=SDL_LoadBMP(filename))!=NULL){
		optimage=SDL_DisplayFormat(loadimage);
		SDL_FreeSurface(loadimage);
	}
	return optimage;
}

void blit_image(int x,int y,SDL_Surface* src,SDL_Surface* dst){
	SDL_Rect offset;

	offset.x=x;
	offset.y=y;
	SDL_BlitSurface(src,NULL,dst,&offset);
}

int main(int argc,int args[]){
	SDL_Surface* image=NULL;
	SDL_Surface* bg=NULL;
	SDL_Surface* window=NULL;
	if(SDL_Init(SDL_INIT_EVERYTHING)==-1)return 1;
	window=SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT,COLOR_DEPTH,SDL_SWSURFACE);
	if(window==NULL)return 2;
	SDL_WM_SetCaption("Blit Test",NULL);

	bg=load_image("bg.bmp");
	blit_image(0,0,bg,window);
	blit_image(320,0,bg,window);
	blit_image(0,240,bg,window);
	blit_image(320,240,bg,window);
	SDL_FreeSurface(bg);

	image=load_image("msg.bmp");
	blit_image(180,140,image,window);
	SDL_FreeSurface(image);

	if(SDL_Flip(window)==-1)return 3;
	SDL_Delay(2000);

	SDL_Quit();
	return 0;
}

