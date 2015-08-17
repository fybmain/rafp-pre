#include <stdio.h>

unsigned fget_u16(FILE *fp){
	unsigned ret;

	ret=fgetc(fp);
	ret|=fgetc(fp)<<8;
	return ret;
}

int main(){
	FILE *fp;
	unsigned width,height,count;

	fp=fopen("cons.shp","rb");
	if(!fp)return -1;

	if(fget_u16(fp)){
		printf("The file 'cons.shp' is not an shp(ts) video file!\nAborted.\n");
		return -2;
	}
	width=fget_u16(fp);
	height=fget_u16(fp);
	count=fget_u16(fp);

	fclose(fp);

	printf(
"\
The header of the shp(ts) video 'cons.shp' indicates that:\n\
Per image:\n\
Width: %u pixel(s)\n\
Height: %u pixel(s)\n\
\n\
%u frame(s) included.\n\
"
	,width,height,count);
	return 0;
}

