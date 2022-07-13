
#pragma warning (disable:4996)

#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <tchar.h>

#define NUM_OF_GATE		3
#define NUM_OF_ENTER	6
#define NUM_OF_ENTER2	6

LONG gTotalCount = 0;

// CRITICAL_SECTION   gCriticalSection;
HANDLE hMutex;

DWORD EnterCount()
{
	//	EnterCriticalSection (&gCriticalSection);
	WaitForSingleObject(hMutex, INFINITE);

	DWORD total = 0;
	
	scanf("%d", &total);

	return total;
	//	LeaveCriticalSection (&gCriticalSection);
	ReleaseMutex(hMutex);
}

void IncreaseCount(DWORD dd[])
{
	//	EnterCriticalSection (&gCriticalSection);
	WaitForSingleObject(hMutex, INFINITE);

	

	for (DWORD i = 0; i < 12; i++)
		gTotalCount += dd[i];

	//	LeaveCriticalSection (&gCriticalSection);
	ReleaseMutex(hMutex);
}

unsigned int WINAPI ThreadProc2(LPVOID lpParam)
{
	
	 

	//IncreaseCount(dd);
	

	return 0;
}

unsigned int WINAPI ThreadProc(DWORD dd[])
{
	

	for (DWORD i = 0; i < 6; i++)
	{
		dd[i] = EnterCount();
	}

	IncreaseCount(dd);

	return 0;
}

unsigned int WINAPI ThreadProc0(DWORD dd[])
{
	

	for (DWORD i = 0; i < 6; i++)
	{
		dd[i] = EnterCount();
	}

	IncreaseCount(dd);

	return 0;
}




int _tmain(int argc, TCHAR* argv[])
{
	DWORD dwThreadIDs[NUM_OF_GATE];
	HANDLE hThreads[NUM_OF_GATE];
	DWORD dd[NUM_OF_ENTER] = { 0 };
	//	InitializeCriticalSection(&gCriticalSection);
	hMutex = CreateMutex(
		NULL,     // 디폴트 보안관리자.
		FALSE,    // 누구나 소유 할 수 있는 상태로 생성.
		NULL      // numaned mutex
	);

	if (hMutex == NULL)
	{
		_tprintf(_T("CreateMutex error: %d\n"), GetLastError());
	}
	
		hThreads[0] = (HANDLE)
			_beginthreadex( //createdthreadex
				NULL,
				0,
				(unsigned int(__stdcall*)(void*))ThreadProc,
				(LPVOID)(&dd[NUM_OF_ENTER]),
				CREATE_SUSPENDED,
				(unsigned*)&dwThreadIDs[0]
			);

		

		hThreads[1] = (HANDLE)
			_beginthreadex( //createdthreadex
				NULL,
				0,
				(unsigned int(__stdcall*)(void*))ThreadProc0,
				(LPVOID)(&dd[NUM_OF_ENTER2]),
				CREATE_SUSPENDED,
				(unsigned*)&dwThreadIDs[1]
			);

		hThreads[2] = (HANDLE)
			_beginthreadex( //createdthreadex
				NULL,
				0,
				ThreadProc2,
				NULL,
				CREATE_SUSPENDED,
				(unsigned*)&dwThreadIDs[2]
			);


		for (DWORD i = 0; i < 3; i++)
		{
		if (hThreads[i] == NULL)
		{
			_tprintf(_T("Thread creation fault! \n"));
			return -1;
		}
	}

	for (DWORD i = 0; i < NUM_OF_GATE; i++)
	{
		ResumeThread(hThreads[i]);
	}



	WaitForMultipleObjects(NUM_OF_GATE, hThreads, TRUE, INFINITE);

	_tprintf(_T("total count: %d \n"), gTotalCount);

	for (DWORD i = 0; i < NUM_OF_GATE; i++)
	{
		CloseHandle(hThreads[i]);
	}

	//	DeleteCriticalSection(&gCriticalSection);
	CloseHandle(hMutex);

	return 0;
}
