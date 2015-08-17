#include <stdio.h>

//the following definitions depend on your compiler
typedef unsigned short u16;
typedef unsigned int u32;

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
shp_ts_image_header cur_img_head;

int main(){
	FILE *fp;
	int i;

	fp=fopen("cons.shp","rb");
	if(!fp)return -1;

	if(fread(&header,sizeof(header),1,fp)!=1)return -2;
	if(header.zero){
		printf("The file 'cons.shp' is not an shp(ts) video file!\nAborted.\n");
		return -3;
	}
	printf("The file 'cons.shp' is an shp(ts) video file including %d frame(s).\n",header.count);
	printf("\
The video:\n\
Width: %d pixel(s)\n\
Height: %d pixel(s)\n\
",header.width,header.height);
	putchar('\n');

	for(i=0;i<header.count;i++){
		if(fread(&cur_img_head,sizeof(cur_img_head),1,fp)!=1)return -2;
		if(cur_img_head.zero){
			printf("Error: Image%d does not keep the format of an shp(ts) video file!\nAborted.\n",i+1);
			return -3;
		}
		printf("image%d: ",i+1);
		printf("(%d,%d)=>(%d,%d),",cur_img_head.x,cur_img_head.y,cur_img_head.x+cur_img_head.width,cur_img_head.y+cur_img_head.height);
		printf("comp%d,offset=0x%x\n",cur_img_head.type_of_compression,cur_img_head.offset_data);
	}

	fclose(fp);
	return 0;
}

