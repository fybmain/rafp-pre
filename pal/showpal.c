#include <SDL/SDL.h>
#include <stdio.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640
#define NUMROW 16
#define NUMCOL 16
#define RECT_WIDTH 40
#define RECT_HEIGHT 40

//the following line depends on your compiler
typedef unsigned char u8;
typedef unsigned int u32;

typedef struct{
	u8 r,g,b;
}rgb6;

//the palette is the simplest format:the whole file consists of an array of 256 colors.for every byte,only the lowest 6 bits are effective.
#define NUM_COLORS_IN_PAL 256
typedef rgb6 pal[NUM_COLORS_IN_PAL];

pal pale;

//high r-g-b low
u32 rgb6_to_u32(rgb6 *s){
	u32 t;

	t=(s->r<<18)|((s->r&0x3)<<16);
	t|=(s->g<<10)|((s->g&0x3)<<8);
	t|=(s->b<<2)|(s->b&0x3);
	return t;
}

void drawpix(SDL_Surface *surface,int x,int y,u32 pix){
	((u32*)(surface->pixels))[(surface->w)*y+x]=pix;
}

void drawrect(SDL_Surface *surface,int x,int y,int width,int height,u32 color){
	int i,j;

	for(i=y;i<y+height;i++)
		for(j=x;j<x+width;j++)
			drawpix(surface,j,i,color);
}

int main(){
	SDL_Surface *window;
	SDL_Event event;
	FILE *fp;
	rgb6 *pcolor;
	int quit=0;
	int i,j;

	if(SDL_Init(SDL_INIT_EVERYTHING)==-1)return -1;
	window=SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT,32,SDL_SWSURFACE);
	if(!window)return -2;
	SDL_WM_SetCaption("Palette Reading Test",NULL);

	fp=fopen("unittem.pal","rb");
	if(!fp)return -3;
	if(fread(&pale,sizeof(rgb6),NUM_COLORS_IN_PAL,fp)!=NUM_COLORS_IN_PAL)return -4;
	fclose(fp);

	pcolor=pale;
	for(i=0;i<NUMCOL;i++)
		for(j=0;j<NUMROW;j++)
			drawrect(window,i*RECT_WIDTH,j*RECT_HEIGHT,RECT_WIDTH,RECT_HEIGHT,rgb6_to_u32(pcolor++));

	if(SDL_Flip(window)==-1)return -5;

	while(quit?SDL_PollEvent(&event):SDL_WaitEvent(&event)){
		if(event.type==SDL_QUIT)quit=1;
	}
	return 0;
}

