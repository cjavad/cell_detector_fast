#pragma once

#include <stdint.h>

extern const uint32_t MM_128_FLOAT_NAN[4];

inline float f_truncf(float f);
inline float f_absf(float f);
inline float f_cosf(float f);
inline float f_sinf(float f);

__attribute__((always_inline)) inline float f_truncf(float f)
{
	float o;
	__asm__ ("roundss $11, %1, %0" : "=x"(o) : "x"(f) : );
	return o;
}

__attribute__((always_inline)) inline float f_absf(float f)
{
	float o;
	__asm__ ("vandps %2, %1, %0" : "=x"(o) : "x"(f), "m"(MM_128_FLOAT_NAN) : );
	return o;
}

__attribute__((always_inline)) inline float f_cosf(float f)
{
	f = f_absf(f) + 0.5f;
	
	f = f - f_truncf(f) - 0.5f;
	f = f_absf(f);
	
	float f2 = f * f;
	return f2 * (f2 * (-64.0f * f + 80.0f) - 20.0f) + 1.0f;
}

__attribute__((always_inline)) inline float f_sinf(float f)
{
	f = f_absf(f - 0.25f) + 0.5f;
	
	f = f - f_truncf(f) - 0.5f;
	f = f_absf(f);
	
	float f2 = f * f;
	return f2 * (f2 * (-64.0f * f + 80.0f) - 20.0f) + 1.0f;
}