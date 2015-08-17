#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_BACKGROUND 0xffffff

//the following definitions depend on your compiler
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct{
	u8 r,g,b;
}rgb6;

typedef struct shp_ts_header{
	u16 zero;
	u16 width;
	u16 height;
	u16 count;
}shp_ts_header;

typedef struct shp_ts_image_header{
	u16 x;
	u16 y;
	u16 width;
	u16 height;
	u32 type_of_compression;
	u32 unknown;
	u32 zero;
	u32 offset_data;
}shp_ts_image_header;

shp_ts_header header;
shp_ts_image_header img_head;

u8 fget_u8(FILE *fp){
	return fgetc(fp);
}

u16 fget_u16(FILE *fp){
	u16 u;

	u=fgetc(fp);
	u|=fgetc(fp)<<8;
	return u;
}

#define NUM_COLORS_IN_PAL 256
typedef rgb6 pal[NUM_COLORS_IN_PAL];

pal rgb6_pal;
u32 palette[NUM_COLORS_IN_PAL];

shp_ts_header header;

int cur_img_ind=1;
int cenx,ceny;
SDL_Rect image_area;

int up_down=0,down_down=0;

u32 rgb6_to_u32(rgb6 *s){
	u32 t;

	t=(s->r<<18)|((s->r&0x3)<<16);
	t|=(s->g<<10)|((s->g&0x3)<<8);
	t|=(s->b<<2)|(s->b&0x3);
	return t;
}

void drawpix(SDL_Surface *surface,int x,int y,u32 color){
	((u32*)(surface->pixels))[(surface->w)*y+x]=color;
}

int draw_shp(SDL_Surface *surface,FILE *fp,int ind){
	shp_ts_image_header img_head;
	u8 *img,*ptr;
	int cur_pos,count_bytes,value;
	SDL_Rect rect;
	int i,j;

	ind=(ind-1)%header.count;
	if(fseek(fp,sizeof(header)+ind*sizeof(img_head),SEEK_SET))return -1;
	if(fread(&img_head,sizeof(img_head),1,fp)!=1)return -1;
	if(img_head.zero)return -2;

	img=malloc(img_head.width*img_head.height*1);
	if(!img)return -3;

	if(fseek(fp,img_head.offset_data,SEEK_SET))return -1;
	if(img_head.type_of_compression&0x2){
		ptr=img;
		for(i=0;i<img_head.height;i++){
			cur_pos=0;
			for(count_bytes=fget_u16(fp)-2;count_bytes>0;count_bytes--){
				value=fget_u8(fp);
				if(value){
					*ptr++=value;
					cur_pos++;
				}else{
					count_bytes--;
					for(value=fgetc(fp);value&&(cur_pos<img_head.width);value--,cur_pos++)
						*ptr++=0x0;
				}
			}
		}
	}else if(fread(img,img_head.width*img_head.height,1,fp)!=1)return -1;

	SDL_FillRect(surface,&image_area,palette[0]);
	for(i=0;i<img_head.height;i++)
		for(j=0;j<img_head.width;j++)
			drawpix(surface,cenx+img_head.x+j,ceny+img_head.y+i,palette[*(img+img_head.width*i+j)]);
	free(img);
	return 0;
}

Uint32 callback_up(Uint32 interval,void *param){
	SDL_Event e;

	if(!up_down)return 0;
	if(!down_down){
		e.type=SDL_USEREVENT;
		e.user.code=1;
		SDL_PushEvent(&e);
	}
	return interval;
}

Uint32 callback_down(Uint32 interval,void *param){
	SDL_Event e;

	if(!down_down)return 0;
	if(!up_down){
		e.type=SDL_USEREVENT;
		e.user.code=2;
		SDL_PushEvent(&e);
	}
	return interval;
}

int refresh(SDL_Surface *surface,FILE* fp){
	if(draw_shp(surface,fp,cur_img_ind))return -1;
	if(SDL_Flip(surface)==-1)return -2;
	return 0;
}

int main(int argc,char **argv){
	SDL_Surface *window;
	SDL_Event event;
	int quit=0;
	FILE *fp;
	int i;

	if(SDL_Init(SDL_INIT_EVERYTHING)==-1)return -1;

	fp=fopen("unittem.pal","rb");
	if(!fp)return -2;
	if(fread(rgb6_pal,sizeof(rgb6_pal),1,fp)!=1)return -3;
	fclose(fp);

	for(i=0;i<NUM_COLORS_IN_PAL;i++)
		palette[i]=rgb6_to_u32(rgb6_pal+i);

	fp=fopen("cons.shp","rb");
	if(!fp)return -2;
	if(fread(&header,sizeof(header),1,fp)!=1)return -3;
	if(img_head.zero)return -4;

	window=SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT,32,SDL_SWSURFACE);
	if(!window)return -5;
	SDL_WM_SetCaption("Shp(ts) Viewer",NULL);
	SDL_FillRect(window,NULL,WINDOW_BACKGROUND);

	image_area.x=cenx=(WINDOW_WIDTH-header.width)/2;
	image_area.y=ceny=(WINDOW_HEIGHT-header.height)/2;
	image_area.w=header.width;
	image_area.h=header.height;

	refresh(window,fp);

	while(quit?SDL_PollEvent(&event):SDL_WaitEvent(&event)){
		switch(event.type){
			case SDL_QUIT:
				quit=1;
				break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym){
					case SDLK_UP:
						up_down=1;
						SDL_AddTimer(100,callback_up,NULL);
						callback_up(100,NULL);
						break;
					case SDLK_DOWN:
						down_down=1;
						SDL_AddTimer(100,callback_down,NULL);
						callback_down(100,NULL);
						break;
				}
				break;
			case SDL_KEYUP:
				switch(event.key.keysym.sym){
					case SDLK_UP:
						up_down=0;
						break;
					case SDLK_DOWN:
						down_down=0;
						break;
				}
				break;
			case SDL_USEREVENT:
				switch(event.user.code){
					case 1:
						if(cur_img_ind>1){
							cur_img_ind--;
							refresh(window,fp);
						}
						break;
					case 2:
						if(cur_img_ind<header.count){
							cur_img_ind++;
							refresh(window,fp);
						}
						break;
				}
				break;
		}
	}

	SDL_Quit();
	fclose(fp);
	return 0;
}

