// demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <intrin.h>

#pragma intrinsic(__rdtsc)


unsigned int i;
unsigned int a;
inline void test()
{
	unsigned __int64 start, end;
	volatile int j;

	start = __rdtsc();
	j = i;
	end = __rdtsc();
	printf("took %lu ticks\n", end - start);
}

int main()
{
	test();
	test();
	test();
	test();
	test();
	test();
	printf("after flush i: ");
	_mm_clflush(&i);
	test();
	test();
	printf("after flush i: ");
	_mm_clflush(&i);
	test();
	test();
	test();
	test();
	printf("after flush i: ");
	_mm_clflush(&i);
	test();
	test();
	test();
	test();
	printf("after flush a: ");
	_mm_clflush(&a);
	test();
	test();
	test();
	test();
    return 0;
}

