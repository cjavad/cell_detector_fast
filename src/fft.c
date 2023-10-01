#include "fft.h"
#include <stdint.h>

#include "bitmap.h"
#include "float.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

void fft_test(BitmapData* data)
{
#ifdef DEBUG
	BitmapImage bmp;
	init_bitmap(&bmp, data->width, data->height);
    
	float* imager = malloc(bmp.bitmap.width * bmp.bitmap.height * sizeof(float));
	float* imagei = malloc(bmp.bitmap.width * bmp.bitmap.height * sizeof(float));

	float* outr = malloc(bmp.bitmap.width * bmp.bitmap.height * sizeof(float));
	float* outi = malloc(bmp.bitmap.width * bmp.bitmap.height * sizeof(float));

	for (uint32_t y = 0; y < data->height; y++)
	{
		uint32_t offset = y * data->row_width;
		for (uint32_t x = 0; x < data->width; x++)
		{
			imager[y * data->width + x] = data->data[offset + x * 3];
		}
	}

	memset(imagei, 0, bmp.bitmap.width * bmp.bitmap.height * sizeof(float));

	printf("Starting test\n");

	FILE* fp;

	memcpy(outr, imager, bmp.bitmap.width * bmp.bitmap.height * sizeof(float));
	memcpy(outi, imagei, bmp.bitmap.width * bmp.bitmap.height * sizeof(float));

	goto fftLabel;

	printf("Starting DFT\n");

	dft(outr, outi, bmp.bitmap.width, bmp.bitmap.height);

	printf("Writing DFT\n");

	for (uint32_t y = 0; y < bmp.bitmap.height; y++)
	{
		uint32_t offset = y * bmp.bitmap.row_width;
		for (uint32_t x = 0; x < bmp.bitmap.width; x++)
		{
			uint32_t bi = offset + x * 3;
			uint32_t fi = y * bmp.bitmap.width + x;

			bmp.bitmap.data[bi + 0] = (uint8_t)(outr[fi] * 255.0f);
			bmp.bitmap.data[bi + 1] = (uint8_t)(outi[fi] * 255.0f);
			bmp.bitmap.data[bi + 2] = 0;
		}
	}

    fp = fopen("res/dft_test.bmp", "wb");
    write_bitmap(fp, &bmp);
    fclose(fp);

	memcpy(outr, imager, bmp.bitmap.width * bmp.bitmap.height * sizeof(float));
	memcpy(outi, imagei, bmp.bitmap.width * bmp.bitmap.height * sizeof(float));

	fftLabel:

	printf("Starting FFT\n");

	fft(outr, outi, bmp.bitmap.width, bmp.bitmap.height);

	printf("Writing FFT\n");

	for (uint32_t y = 0; y < bmp.bitmap.height; y++)
	{
		uint32_t offset = y * bmp.bitmap.row_width;
		for (uint32_t x = 0; x < bmp.bitmap.width; x++)
		{
			uint32_t bi = offset + x * 3;
			uint32_t fi = y * bmp.bitmap.width + x;

			bmp.bitmap.data[bi + 0] = (uint8_t)(outr[fi] * 255.0f);
			bmp.bitmap.data[bi + 1] = (uint8_t)(outi[fi] * 255.0f);
			bmp.bitmap.data[bi + 2] = 0;
		}
	}

	fp = fopen("res/fft_test.bmp", "wb");
	write_bitmap(fp, &bmp);
	fclose(fp);

    free_bitmap(&bmp);
    free(imager);
    free(imagei);
    free(outr);
    free(outi);
#endif
}

__attribute__((always_inline)) inline void dft1d(
	float* const real, 
	float* const imag,
	const uint32_t length, 
	const uint32_t stride, 
	const uint32_t offset
)  {
	float ir[length];
	float ii[length];

	for (uint32_t i = 0, j = offset; i < length; i++)
	{
		ir[i] = real[j];
		ii[i] = imag[j];
		j += stride;
	}

	float rN = -1.0f / (float)length;
	for (uint32_t k = 0; k < length; k++)
	{
		float fac = rN * (float)k;
		float rsum = 0;
		float isum = 0;

		for (uint32_t n = 0; n < length; n++)
		{
			float e = fac * (float)n;

			float c = f_cosf(e);
			float s = f_sinf(e);

			rsum += ir[n] * c - ii[n] * s;
			isum += ir[n] * c + ii[n] * s;
		}

		real[offset + k * stride] = rsum;
		imag[offset + k * stride] = isum;
	}
}

__attribute__((always_inline)) inline void fft1d(
	float* const real, 
	float* const imag,
	const uint32_t length, 
	const uint32_t stride, 
	const uint32_t offset
) {
	float or[length];
	float oi[length];

	for (uint32_t i = 0, j = offset; i < length; i++)
	{
		or[i] = real[j];
		oi[i] = imag[j];
		j += stride;
	}

	for (uint32_t rs = 1; rs < length; rs <<= 1)
	{
		// printf("rs = %u\n", rs);
		float rN = -1.0f / rs;

		for (uint32_t o = 0; o < length; o += rs << 1)
		{
			// if (rs > 32) printf("%u to %u\n", o, o + rs + rs);
			for (uint32_t k = o; k - o < rs && k + rs < length; k++)
			{
				uint32_t px = k;
				uint32_t qx = k + rs;

				float pr = or[px];
				float pi = oi[px];

				float e = rN * (float)k;
				float c = f_cosf(e);
				float s = f_sinf(e);

				float qr = or[qx] * c - oi[qx] * s;
				float qi = oi[qx] * c + or[qx] * s;

				or[px] = pr + qr;
				oi[px] = pi + qi;

				or[qx] = pr - qr;
				oi[qx] = pi - qi;
			}
		}
	}

	for (uint32_t i = 0, j = offset; i < length; i++)
	{
		real[j] = or[i];
		imag[j] = oi[i];
		j += stride;
	}


}

void dft(float* real, float* imag, uint32_t width, uint32_t height)
{
	for (uint32_t row = 0; row < height; row++)
	{
		dft1d(real, imag, width, 1, row * width);
	}

	for (uint32_t col = 0; col < width; col++)
	{
		dft1d(real, imag, height, width, col);
	}
}


// this method produces wrong data
void fft(float* real, float* imag, uint32_t width, uint32_t height)
{
	for (uint32_t row = 0; row < height; row++)
	{
		fft1d(real, imag, width, 1, row * width);
	}

	for (uint32_t col = 0; col < width; col++)
	{
		fft1d(real, imag, height, width, col);
	}
}
