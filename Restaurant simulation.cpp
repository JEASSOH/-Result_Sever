/*
	
	프로그램 설명: 카운트 세마포어에 대한 이해
	시뮬레이션 제한 요소:
-	30개의 의자들이 준비가 되어있으며 동시에 30명이 피자를 먹을 수 있다. 해결 o
-	보통 점심시간에 손님은 200명정도가 식사를 하고 간다. 해결 o
-	점심시간은 11시 30분부터 시작하여 2시에 종료한다. 미해결 o - 어느정도 비슷한 시각 (오차범위 30분~1시간)에 종료(?)나 대기상태로 가게는 했다만,
2시 언저리에 쓰레드를 받지 않고 이미 들어간 쓰레드의 반환값을 나오게 하는 정확한 방법을 모르겠다. 아마 Waitfor~을 잘쓴다면 가능하겠지만 쓰레드의
기초 상식이 너무 부족한 나였던거 같다. 다시 처음부터 재대로 공부하고 다시 시도해보겠다.
-	손님 한 명이 피자를 먹고 나가는 데까지 평균적으로 30분이 소요된다. 하지만 식사시간은 최소 10분에서 최대 50분까지 소요된다고 가정한다. 해결o
-	이벤트(event), 세마포아(semaphore), 타이머(Timer), 또는 크리티컬섹션(Critical Section) 등을 사용한 동기화 수행. 해결 o
-	시간함수 사용 해결o
-	각 손님이 들어온 시간과 식사에 걸리는 시간, 식사 후 퇴장하는 시간을 각각 출력 해결 o
가산점 부분
파일을 사용하여 손님에 대한 사용자 파일링 - 정확히 어떤 방법을 통해 하는지 모르겠다 그래서 실시간으로 식사시작시간,소요시간,방문일자를 프로그래밍상에
다 나오게했다 해결(?)
ㄱ. 손님의 수와 도착시간 임의적 발생. 해결o



*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <process.h>
#include <tchar.h>
#include <iostream>
#include <random>
#include <conio.h>
#include <chrono>
#include <cmath>

using namespace std;

// 시드값을 얻기 위한 random_device 생성.
std::random_device rd;

// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::mt19937 gen(rd());

// 평균 30 유지 표준편차 10.
std::normal_distribution<> dis(30, 10);
std::uniform_int_distribution<> people(50, 200);

#define NUM_OF_CUSTOMER 200
#define RANGE_MIN 10 //최소시간(10분)
//#define RANGE_AVE (RANGE_MIN + RANGE_MAX) / 2//중간값 - 필요?x
#define RANGE_MAX (50 - RANGE_MIN)//최대시간(50분)
#define TABLE_CNT 30//테이블 수 

#define _WIN32_WINNT	0x0400




HANDLE hSemaphore;
//HANDLE hTimer;
DWORD randTimeArr[200];
//DWORD timer = 0;
DWORD openTime;
DWORD enterTime;
DWORD EatingStartTime;
DWORD ExitTime;
DWORD opentimeho = 11;
DWORD opentimemi = 30;
DWORD entertimeho = 11;
DWORD entertimemi = 30;
DWORD exittimeho = 0;
DWORD exittimemi = 0;
DWORD leavetime = 0;
bool threadswitch = true;


void TakeMeal(DWORD time)	
{
	if(threadswitch) {
		WaitForSingleObject(hSemaphore, INFINITE);
		entertimeho = opentimeho;
		entertimemi = opentimemi;

		std::wcout << "Customer" << GetCurrentThreadId() << "식당에 들어온 시간 : " << entertimeho << " 시" << entertimemi << "분\n" << std::endl;
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		std::wcout << "Customer" << GetCurrentThreadId() << "은 앞으로 : " << time << "분 동안 먹을 예정입니다\n" << std::endl;
		Sleep(1000 * time);	// 식사중인 상태를 시뮬레이션 하는 함수.
		std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;

		if (opentimeho <= 14)
		{
			ReleaseSemaphore(hSemaphore, 1, NULL);
		}
		




		//CloseHandle(hTimer);
		_tprintf(_T("Out Customer %d~ \n\n"), GetCurrentThreadId());
		exittimeho = entertimeho;
		exittimemi = entertimemi + time;
		if (exittimemi >= 60)
		{
			exittimeho += 1;
			leavetime = exittimemi - 60;
			exittimemi = leavetime;
			leavetime = 0;
		}

		wcout << "Customer" << GetCurrentThreadId() << "밥먹는데 걸린 시간 : " << sec.count() << " 분\n" << std::endl;
		std::wcout << "나가는 시간 : " << exittimeho << " 시" << exittimemi << "분\n" << std::endl;
		opentimeho = exittimeho;
		opentimemi = exittimemi;
		if (opentimeho >= 14)
		{
			threadswitch = false;
		}
	}
	WaitForSingleObject(hSemaphore, INFINITE);
	CloseHandle(hSemaphore);
}


unsigned int WINAPI ThreadProc(LPVOID lpParam)
{
	
	TakeMeal((DWORD)lpParam);
	return 0;
}

int _tmain(int argc, TCHAR* argv[])
{
	DWORD dwThreadIDs[NUM_OF_CUSTOMER];
	HANDLE hThreads[NUM_OF_CUSTOMER];
	
	DWORD comepeo = people(gen);
	DWORD comepeo2 = 0;
	DWORD comepeo3 = 0;
	wcout << "오늘 오는 사람의 수:" << comepeo << std::endl;
	//struct tm* localtime(const time_t * timeval); //시간함수 생성
	
	//LARGE_INTEGER liDueTime;

	//srand((unsigned)time(NULL));  	// random function seed 설정


	// 쓰레드에게 전달할 random 값 총 200개 생성.
	for (int i = 0; i < NUM_OF_CUSTOMER; i++)
	{
		randTimeArr[i] = (DWORD)(dis(gen));
			
	}



	/*hTimer = CreateWaitableTimer(NULL, FALSE, _T("WaitableTimer"));
	if (!hTimer)
	{
		_tprintf(_T("CreateWaitableTimer failed (%d)\n"), GetLastError());
		return 1;
	}*/


	// 세마포어 생성.
	hSemaphore = CreateSemaphore(
		NULL,    // 디폴트 보안관리자.
		TABLE_CNT,      // 세마포어 초기 값.
		TABLE_CNT,      // 세마포어 최대 값.
		NULL     // unnamed 세마포어 구성.
	);
	if (hSemaphore == NULL)
	{
		_tprintf(_T("CreateSemaphore error: %d\n"), GetLastError());
	}


	// Customer를 의미하는 쓰레드 생성.
	for (int i = 0; i < NUM_OF_CUSTOMER; i++)
	{
		hThreads[i] = (HANDLE)
			_beginthreadex(
				NULL,
				0,
				ThreadProc,
				(void*)randTimeArr[i],
				CREATE_SUSPENDED,
				(unsigned*)&dwThreadIDs[i]
			);

		if (hThreads[i] == NULL)
		{
			_tprintf(_T("Thread creation fault! \n"));
			return -1;
		}
	}

	for (int i = 0; i < NUM_OF_CUSTOMER; i++)
	{
		ResumeThread(hThreads[i]);
	}
	
	if (comepeo <= 64)
	{
		WaitForMultipleObjects(comepeo, hThreads, TRUE, INFINITE);
	}
	else if (comepeo >= 64 && comepeo <= 128)
	{

		comepeo2 = comepeo - 64;
		WaitForMultipleObjects(comepeo, hThreads, TRUE, INFINITE);
		WaitForMultipleObjects(comepeo2, hThreads, TRUE, INFINITE);

	}
	else if (comepeo >= 128 && comepeo <= 192)
	{
		comepeo2 = comepeo - 64;
		comepeo3 = comepeo - 128;
		WaitForMultipleObjects(comepeo, hThreads, TRUE, INFINITE);
		WaitForMultipleObjects(comepeo2, hThreads, TRUE, INFINITE);
		WaitForMultipleObjects(comepeo3, hThreads, TRUE, INFINITE);
	}
	else
	{
		wcout << "식당포화" << std::endl;
	}
	

	_tprintf(_T("----END-----------\n"));

	for (int i = 0; i < NUM_OF_CUSTOMER; i++)
	{
		CloseHandle(hThreads[i]);
	}

	CloseHandle(hSemaphore);

	return 0;
}

