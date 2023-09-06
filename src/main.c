#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"

int32_t main()
{
	struct BitmapHeader header;
	struct BitmapInfoHeader infoHeader;
	struct Bitmap bitmap;

	read_bitmap(&header, &infoHeader, &bitmap, "res/example.bmp");

	printf("Magic = %.2s\n", &header.magic);
	printf("Size = %ix%i\n", infoHeader.width, infoHeader.height);
	printf("Bits per pixel = %u\n", infoHeader.bpp);


	return 0;
}