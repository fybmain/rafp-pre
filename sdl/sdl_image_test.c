#include "SDL/SDL_image.h"

#define COLOR_DEPTH 32

#define IMAGE_WIDTH 400
#define IMAGE_HEIGHT 100

#define NUM_IMAGES 8
char* names[NUM_IMAGES]={"test.bmp","test.pnm","test.xpm","test.pcx","test.gif","test.jpeg","test.tga","test.png"};	//i don't know how to create *.lbm pictures

SDL_Surface* load_image(char* filename){
	SDL_Surface* loadimage=NULL;
	SDL_Surface* optimage=NULL;

	if((loadimage=IMG_Load(filename))!=NULL){
		optimage=SDL_DisplayFormat(loadimage);
		SDL_FreeSurface(loadimage);
	}
	return optimage;
}

int main(int argc,char args[]){
	int i;

	SDL_Surface* window=NULL;
	SDL_Surface* pic=NULL;

	SDL_Rect offset;

	window=SDL_SetVideoMode(IMAGE_WIDTH,IMAGE_HEIGHT*NUM_IMAGES,COLOR_DEPTH,SDL_SWSURFACE);

	offset.x=offset.y=0;
	for(i=0;i<NUM_IMAGES;i++){
		pic=load_image(names[i]);
		SDL_BlitSurface(pic,NULL,window,&offset);
		SDL_FreeSurface(pic);
		offset.y+=IMAGE_HEIGHT;
	}

	SDL_Flip(window);
	SDL_Delay(2000);
	SDL_Quit();
	return 0;
}

