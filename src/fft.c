#include "fft.h"
#include <stdint.h>

#include "float.h"

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

	for (uint32_t rs = 1; rs << 1 < length; rs <<= 1)
	{
		float rN = -1.0f / rs;

		for (uint32_t o = 0; o < length; o += rs)
		{
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

inline void fft_unit(float* dest, float* src, uint32_t width, uint32_t height, uint32_t cx, uint32_t cy)
{
	
}

void fft(float* real, float* imag, uint32_t width, uint32_t height)
{
	for (uint32_t row = 0; row < height; row++)
	{
		dft1d(real, imag, width, 1, row * width);
		// fft1d(real, imag, width, 1, row * width);
	}

	for (uint32_t col = 0; col < width; col++)
	{
		dft1d(real, imag, height, width, col);
		// fft1d(real, imag, height, width, col);
	}
}
