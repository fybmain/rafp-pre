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
typedef float f32;

typedef struct{
	u8 r,g,b;
}rgb6;

#define NUM_COLORS_IN_PAL 256
typedef rgb6 palette6[NUM_COLORS_IN_PAL];

#define VXL_UNKNOWN2 0x1f10
typedef struct{
	char magic_str[16];
	u32 unknown1;
	u32 count_limb1;
	u32 count_limb2;
	u32 size_body;
	u16 unknown2;
	palette6 pal;
}__attribute__ ((packed)) vxl_header;

typedef struct{
	char name[16];
	u32 number;
	u32 unknown1;
	u32 unknown2;
}vxl_limb_header;

typedef struct{
	u32 off_body_span_start;
	u32 off_body_span_end;
	u32 off_body_span_data;
	f32 scale;
	f32 m_transform[3][4];
	f32 v_min_scale[3];
	f32 v_max_scale[3];
	u8 width;
	u8 breadth;
	u8 height;
	u8 unknown;
}vxl_limb_tailer;

typedef struct{
	int width,breadth,height;
}matrix_size;

typedef struct{
	int width,height;
}image_size;

palette6 pal6;
u32 pal[NUM_COLORS_IN_PAL];
char vxl_magic_str[]="Voxel Animation";

u32 rgb6_to_u32(rgb6 *s){
	u32 t;

	t=(s->r<<18)|((s->r&0x3)<<16);
	t|=(s->g<<10)|((s->g&0x3)<<8);
	t|=(s->b<<2)|(s->b&0x3);
	return t;
}

void drawpix(SDL_Surface *surface,int y,int x,u32 color){
	((u32*)(surface->pixels))[(surface->w)*y+x]=color;
}

void my_abort(int ret,char *err_str){
	fprintf(stderr,"Error: %s\nAborted.\n",err_str);
	exit(ret);
}

int decode_limb_body(u8 *span_data,u32 *span_start_list,u8 *matrix,matrix_size *size){
	u8 *pr,*pw;
	int z,count;
	int i,j;

	pw=matrix;
	for(i=0;i<size->width*size->breadth;i++){
		if(*(span_start_list+i)==-1){
			for(j=0;j<size->height;j++)
				*pw++=0;
			continue;
		}
		pr=span_data+*(span_start_list+i);
		for(z=0;z<size->height;){
			count=*pr++;
			for(j=0;j<count;j++){
				*pw++=0;
				z++;
			}
			count=*pr++;
			for(j=0;j<count;j++){
				*pw++=*pr;
				pr+=2;
				z++;
			}
			if(*pr==count)pr++;
			else return -1;
		}
	}
	return 0;
}

void shot_from_top(u8 *matrix,matrix_size *size,u8 *image,image_size *image_size){
	int z;
	int i,j;

	image_size->width=size->width;
	image_size->height=size->breadth;
	for(i=0;i<size->breadth;i++)
		for(j=0;j<size->width;j++){
			*(image+i*size->width+j)=0;
			for(z=size->height;z>0;z--)
				if(*(matrix+(i*size->width+j)*size->height+z-1)){
					*(image+i*size->width+j)=*(matrix+(i*size->width+j)*size->height+z-1);
					break;
				}
		}
}

void draw_img(SDL_Surface *surface,SDL_Rect img_area,u8 *image){
	int i,j;

	for(i=0;i<img_area.h;i++)
		for(j=0;j<img_area.w;j++)
			drawpix(surface,img_area.y+i,img_area.x+j,pal[*(image+i*img_area.w+j)]);
}

int main(int argc,char **argv){
	SDL_Surface *window;
	SDL_Event event;
	SDL_Rect img_area;
	int quit=0;

	FILE *fp;
	u8 *fdata;
	long fsize;

	int cur_limb_ind;
	vxl_header *header;
	vxl_limb_header *limb_head_start,*cur_limb_head;
	u8 *limb_body_start;
	vxl_limb_tailer *limb_tail_start,*cur_limb_tail;

	u8 *vxl_matrix,*img;
	matrix_size vxl_matrix_size;
	image_size img_size;

	int ret;
	int i,j,k;

	if(SDL_Init(SDL_INIT_EVERYTHING)==-1)
		my_abort(-1,"SDL Initialization Failed!");

	fp=fopen("rtnk.vxl","rb");
	if(!fp)my_abort(-2,"Failed to open the file 'rtnk.vxl'!");
	if(fseek(fp,0,SEEK_END))
		my_abort(-3,"Failed to find the end of the file!");
	fsize=ftell(fp);
	fdata=malloc(fsize);
	if(!fdata)my_abort(-4,"Failed to allocate memory for the file!");

	fseek(fp,0,SEEK_SET);
	if(fread(fdata,fsize,1,fp)!=1)
		my_abort(-4,"Failed to copy the file to the memory!");
	fclose(fp);

	header=(vxl_header*)fdata;
	for(i=0;i<sizeof(header->magic_str);i++)
		if(vxl_magic_str[i]!=header->magic_str[i])
			my_abort(-4,"The file 'rtnk.vxl' is not a vxl file!");

	if(header->unknown1!=1||header->unknown2!=VXL_UNKNOWN2
			||header->count_limb1!=header->count_limb2)
		my_abort(-5,"VXL Format Error!");

	fp=fopen("unitsno.pal","rb");
	if(!fp)my_abort(-2,"Failed to open the file 'unitsno.pal'!");
	if(fread(pal6,sizeof(pal6),1,fp)!=1)my_abort(-4,"Failed to read the palette file!");
	fclose(fp);
	for(i=0;i<NUM_COLORS_IN_PAL;i++)
		pal[i]=rgb6_to_u32(pal6+i);

	limb_head_start=(vxl_limb_header*)(fdata+sizeof(vxl_header));
	limb_body_start=(u8*)(limb_head_start+header->count_limb1);
	limb_tail_start=(vxl_limb_tailer*)(limb_body_start+header->size_body);

	cur_limb_ind=0;
	cur_limb_head=limb_head_start+cur_limb_ind;
	cur_limb_tail=limb_tail_start+cur_limb_ind;
	vxl_matrix_size.width=cur_limb_tail->width;
	vxl_matrix_size.breadth=cur_limb_tail->breadth;
	vxl_matrix_size.height=cur_limb_tail->height;
	vxl_matrix=malloc(vxl_matrix_size.width*vxl_matrix_size.breadth*vxl_matrix_size.height);
	if(!vxl_matrix)my_abort(-4,"Failed to allocate memory for the voxel matrix!");
	ret=decode_limb_body(
		limb_body_start+cur_limb_tail->off_body_span_data,
		(u32*)(limb_body_start+cur_limb_tail->off_body_span_start),
		vxl_matrix,&vxl_matrix_size
	);
	if(ret==-1)my_abort(-6,"VXL Format Error!");

	img=malloc(vxl_matrix_size.width*vxl_matrix_size.breadth);
	if(!img)my_abort(-4,"Failed to allocate memory for the output image!");
	shot_from_top(vxl_matrix,&vxl_matrix_size,img,&img_size);

	window=SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT,32,SDL_SWSURFACE);
	if(!window)my_abort(-7,"Failed to open the window!");
	SDL_WM_SetCaption("VXL Limb Viewer(From Top)",NULL);
	SDL_FillRect(window,NULL,WINDOW_BACKGROUND);

	img_area.h=img_size.height;
	img_area.w=img_size.width;
	img_area.y=(WINDOW_HEIGHT-img_size.height)/2;
	img_area.x=(WINDOW_WIDTH-img_size.width)/2;
	draw_img(window,img_area,img);

	if(SDL_Flip(window)==-1)my_abort(-7,"Failed to flip the window!");
	while(quit?SDL_PollEvent(&event):SDL_WaitEvent(&event)){
		switch(event.type){
			case SDL_QUIT:
				quit=1;
				break;
			default:break;
		}
	}

	return 0;
}

