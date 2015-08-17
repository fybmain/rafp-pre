#include <stdio.h>
#include <stdlib.h>

//the following definitions depend on your compiler
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef float f32;

#define VXL_UNKNOWN2 0x1f10
typedef struct{
	char magic_str[16];
	u32 unknown1;
	u32 count_limb1;
	u32 count_limb2;
	u32 size_body;
	u16 unknown2;
	u8 pal[256][3];	//in this program,we just decode the vxl file
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

char vxl_magic_str[]="Voxel Animation";

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

int main(){
	FILE *fp;
	u8 *fdata,*vxl_matrix,*img;
	vxl_header *header;
	vxl_limb_header *limb_head_start,*cur_limb_head;
	u8 *limb_body_start;
	vxl_limb_tailer *limb_tail_start,*cur_limb_tail;
	matrix_size vxl_matrix_size;
	image_size img_size;
	long fsize;
	int cur_limb_ind,ret;
	int i,j,k;

	fp=fopen("rtnk.vxl","rb");
	if(!fp)my_abort(-1,"Failed to open the file 'rtnk.vxl'!");
	if(fseek(fp,0,SEEK_END))
		my_abort(-2,"Failed to find the end of the file!");
	fsize=ftell(fp);
	fdata=malloc(fsize);
	if(!fdata)my_abort(-3,"Failed to allocate memory for the file!");

	fseek(fp,0,SEEK_SET);
	if(fread(fdata,fsize,1,fp)!=1)
		my_abort(-2,"Failed to copy the file to the memory!");
	fclose(fp);

	header=(vxl_header*)fdata;
	for(i=0;i<sizeof(header->magic_str);i++)
		if(vxl_magic_str[i]!=header->magic_str[i])
			my_abort(-4,"The file 'rtnk.vxl' is not a vxl file!");

	if(header->unknown1!=1||header->unknown2!=VXL_UNKNOWN2
			||header->count_limb1!=header->count_limb2)
		my_abort(-5,"VXL Format Error!");

	printf("The file 'rtnk.vxl' is an '%s' file including %d limbs.\n",header->magic_str,header->count_limb1);

	limb_head_start=(vxl_limb_header*)(fdata+sizeof(vxl_header));
	limb_body_start=(u8*)(limb_head_start+header->count_limb1);
	limb_tail_start=(vxl_limb_tailer*)(limb_body_start+header->size_body);
	cur_limb_head=limb_head_start;
	cur_limb_tail=limb_tail_start;
	for(i=0;i<header->count_limb1;i++){
		if(cur_limb_head->unknown1!=1||cur_limb_head->unknown2!=0)
			my_abort(-5,"VXL Format Error!");
		printf("\
Limb%d(%d):\"%s\"\n\
Offset List of Span Starts at:0x%x\n\
Offset List of Span Ends at:0x%x\n\
Span Data at:0x%x\n\
",
			i,cur_limb_head->number,cur_limb_head->name,
			(unsigned)(limb_body_start-fdata)+cur_limb_tail->off_body_span_start,
			(unsigned)(limb_body_start-fdata)+cur_limb_tail->off_body_span_end,
			(unsigned)(limb_body_start-fdata)+cur_limb_tail->off_body_span_data
		);

		printf("Transforming Matrix:\n");
		for(j=0;j<3;j++){
			for(k=0;k<4;k++)
				printf("%f ",cur_limb_tail->m_transform[j][k]);
			putchar('\n');
		}
		printf("Scaling Vector:\nMin:");
		for(j=0;j<3;j++)printf(" %f",cur_limb_tail->v_min_scale[j]);
		printf("\nMax:");
		for(j=0;j<3;j++)printf(" %f",cur_limb_tail->v_max_scale[j]);
		printf("\nWidth=%d,Breadth=%d,Height=%d.\n\n",
			cur_limb_tail->width,
			cur_limb_tail->breadth,
			cur_limb_tail->height
		);

		cur_limb_head++;
		cur_limb_tail++;
	}

	printf("Choose one limb to decode:");
	scanf("%d",&cur_limb_ind);
	cur_limb_head=limb_head_start+cur_limb_ind;
	cur_limb_tail=limb_tail_start+cur_limb_ind;
	vxl_matrix_size.width=cur_limb_tail->width;
	vxl_matrix_size.breadth=cur_limb_tail->breadth;
	vxl_matrix_size.height=cur_limb_tail->height;
	vxl_matrix=malloc(vxl_matrix_size.width*vxl_matrix_size.breadth*vxl_matrix_size.height);
	if(!vxl_matrix)my_abort(-3,"Failed to allocate memory for the voxel matrix!");
	ret=decode_limb_body(limb_body_start+cur_limb_tail->off_body_span_data,(u32*)(limb_body_start+cur_limb_tail->off_body_span_start),vxl_matrix,&vxl_matrix_size);
	if(ret==-1)my_abort(-5,"VXL Format Error!");

	img=malloc(vxl_matrix_size.width*vxl_matrix_size.breadth);
	if(!img)my_abort(-3,"Failed to allocate memory for the output image!");
	shot_from_top(vxl_matrix,&vxl_matrix_size,img,&img_size);

	fp=fopen("output.txt","wb");
	if(!fp)my_abort(-1,"Failed to open the output file!");
	for(i=0;i<img_size.height;i++){
		for(j=0;j<img_size.width;j++)
			fprintf(fp,"%2x ",*(img+img_size.width*i+j));
		fputc('\n',fp);
	}
	fclose(fp);

	return 0;
}

