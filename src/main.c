#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
#include "grayscale.h"

int32_t main()
{
    FILE* fp = fopen("res/example.bmp", "rb");

	BitmapHeader header;
	BitmapInfoHeader infoHeader;
	Bitmap bitmap;

	read_bitmap(fp, &header, &infoHeader, &bitmap);

    fclose(fp);

	printf("Magic = %.2s\n", (char *) &header.magic);
	printf("Size = %ix%i\n", infoHeader.width, infoHeader.height);
	printf("Bits per pixel = %u\n", infoHeader.bpp);



    fp = fopen("res/out.bmp", "wb");

    GrayScale image;
    bmp_to_grayscale(&bitmap, &image);
    write_grayscale(fp, &image);

    fclose(fp);

	return 0;
}