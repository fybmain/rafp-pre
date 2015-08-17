#include <stdio.h>
#include <stdlib.h>

//the following definitions depend on your compiler
typedef unsigned char u8;
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

int main(){
	FILE *fp,*fpout;
	u8 *img,*ptr,*buff;
	int ind,cur_pos,count_bytes,value;
	int i,j;

	fp=fopen("cons.shp","rb");
	if(!fp){
		printf("Error: Failed to open the file 'cons.shp'.\nAborted.\n");
		return -1;
	}

	fread(&header,sizeof(header),1,fp);
	if(header.zero){
		printf("Error: This is not an shp(ts) video file!\nAborted.\n");
		return -2;
	}
	printf("The shp(ts) video file 'cons.shp' includes %d frame(s).\n",header.count);

	printf("Choose one frame to decode:");
	scanf("%d",&ind);
	fseek(fp,(ind-1)*sizeof(img_head),SEEK_CUR);
	fread(&img_head,sizeof(img_head),1,fp);
	if(img_head.zero){
		printf("Error: The header of Frame %d does not keep the format of an shp(ts) file!\nAborted.\n",ind);
		return -2;
	}
	printf("\
Frame %d:\n\
Width: %d pixel(s)\n\
Height: %d pixel(s)\n\
Painted Area: (%d,%d)=>(%d,%d)\n\
Is Compressed: %s\n\
Offset: 0x%x\n\
",
	ind,header.width,header.height,
	img_head.x,img_head.y,img_head.x+img_head.width,img_head.y+img_head.height,
	((img_head.type_of_compression&0x2)?"Yes":"No"),img_head.offset_data);

	img=malloc(img_head.width*img_head.height*1);
	if(!img){
		printf("Failed to allocate memory for the image!\nAborted.\n");
		return -3;
	}

//	fseek(fp,sizeof(header)+sizeof(img_head)*header.count+img_head.offset_data,SEEK_SET);
	fseek(fp,img_head.offset_data,SEEK_SET);
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
	}else fread(img,img_head.width*img_head.height,1,fp);
	fclose(fp);

	fpout=fopen("frame.out","w");
	if(!fpout){
		printf("Error: Failed to open the file 'frame.out'.\nAborted.\n");
		return -1;
	}
	buff=malloc(header.width*3+1);
	for(i=0;i<header.width*3;i+=3)*(buff+i)='0';
	for(i=1;i<header.width*3;i+=3)*(buff+i)='0';
	for(i=2;i<header.width*3;i+=3)*(buff+i)=' ';
	*(buff+header.width*3)='\n';
	for(i=0;i<img_head.y;i++)fwrite(buff,header.width*3+1,1,fpout);
	ptr=img;
	for(i=0;i<img_head.height;i++){
		fwrite(buff,img_head.x*3,1,fpout);
		for(j=0;j<img_head.width;j++)
			fprintf(fpout,"%2x ",*ptr++);
		fwrite(buff+(img_head.x+img_head.width)*3,(header.width-img_head.x-img_head.width)*3+1,1,fpout);
	}
	for(i=img_head.y+img_head.height;i<header.height;i++)
		fwrite(buff,header.width*3+1,1,fpout);
	free(img);
	free(buff);
	fclose(fpout);

	return 0;
}

