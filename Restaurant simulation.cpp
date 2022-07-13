/*
	
	���α׷� ����: ī��Ʈ ������� ���� ����
	�ùķ��̼� ���� ���:
-	30���� ���ڵ��� �غ� �Ǿ������� ���ÿ� 30���� ���ڸ� ���� �� �ִ�. �ذ� o
-	���� ���ɽð��� �մ��� 200�������� �Ļ縦 �ϰ� ����. �ذ� o
-	���ɽð��� 11�� 30�к��� �����Ͽ� 2�ÿ� �����Ѵ�. ���ذ� o - ������� ����� �ð� (�������� 30��~1�ð�)�� ����(?)�� �����·� ���Դ� �ߴٸ�,
2�� �������� �����带 ���� �ʰ� �̹� �� �������� ��ȯ���� ������ �ϴ� ��Ȯ�� ����� �𸣰ڴ�. �Ƹ� Waitfor~�� �߾��ٸ� �����ϰ����� ��������
���� ����� �ʹ� ������ �������� ����. �ٽ� ó������ ���� �����ϰ� �ٽ� �õ��غ��ڴ�.
-	�մ� �� ���� ���ڸ� �԰� ������ ������ ��������� 30���� �ҿ�ȴ�. ������ �Ļ�ð��� �ּ� 10�п��� �ִ� 50�б��� �ҿ�ȴٰ� �����Ѵ�. �ذ�o
-	�̺�Ʈ(event), ��������(semaphore), Ÿ�̸�(Timer), �Ǵ� ũ��Ƽ�ü���(Critical Section) ���� ����� ����ȭ ����. �ذ� o
-	�ð��Լ� ��� �ذ�o
-	�� �մ��� ���� �ð��� �Ļ翡 �ɸ��� �ð�, �Ļ� �� �����ϴ� �ð��� ���� ��� �ذ� o
������ �κ�
������ ����Ͽ� �մԿ� ���� ����� ���ϸ� - ��Ȯ�� � ����� ���� �ϴ��� �𸣰ڴ� �׷��� �ǽð����� �Ļ���۽ð�,�ҿ�ð�,�湮���ڸ� ���α׷��ֻ�
�� �������ߴ� �ذ�(?)
��. �մ��� ���� �����ð� ������ �߻�. �ذ�o



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

// �õ尪�� ��� ���� random_device ����.
std::random_device rd;

// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::mt19937 gen(rd());

// ��� 30 ���� ǥ������ 10.
std::normal_distribution<> dis(30, 10);
std::uniform_int_distribution<> people(50, 200);

#define NUM_OF_CUSTOMER 200
#define RANGE_MIN 10 //�ּҽð�(10��)
//#define RANGE_AVE (RANGE_MIN + RANGE_MAX) / 2//�߰��� - �ʿ�?x
#define RANGE_MAX (50 - RANGE_MIN)//�ִ�ð�(50��)
#define TABLE_CNT 30//���̺� �� 

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

		std::wcout << "Customer" << GetCurrentThreadId() << "�Ĵ翡 ���� �ð� : " << entertimeho << " ��" << entertimemi << "��\n" << std::endl;
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		std::wcout << "Customer" << GetCurrentThreadId() << "�� ������ : " << time << "�� ���� ���� �����Դϴ�\n" << std::endl;
		Sleep(1000 * time);	// �Ļ����� ���¸� �ùķ��̼� �ϴ� �Լ�.
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

		wcout << "Customer" << GetCurrentThreadId() << "��Դµ� �ɸ� �ð� : " << sec.count() << " ��\n" << std::endl;
		std::wcout << "������ �ð� : " << exittimeho << " ��" << exittimemi << "��\n" << std::endl;
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
	wcout << "���� ���� ����� ��:" << comepeo << std::endl;
	//struct tm* localtime(const time_t * timeval); //�ð��Լ� ����
	
	//LARGE_INTEGER liDueTime;

	//srand((unsigned)time(NULL));  	// random function seed ����


	// �����忡�� ������ random �� �� 200�� ����.
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


	// �������� ����.
	hSemaphore = CreateSemaphore(
		NULL,    // ����Ʈ ���Ȱ�����.
		TABLE_CNT,      // �������� �ʱ� ��.
		TABLE_CNT,      // �������� �ִ� ��.
		NULL     // unnamed �������� ����.
	);
	if (hSemaphore == NULL)
	{
		_tprintf(_T("CreateSemaphore error: %d\n"), GetLastError());
	}


	// Customer�� �ǹ��ϴ� ������ ����.
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
		wcout << "�Ĵ���ȭ" << std::endl;
	}
	

	_tprintf(_T("----END-----------\n"));

	for (int i = 0; i < NUM_OF_CUSTOMER; i++)
	{
		CloseHandle(hThreads[i]);
	}

	CloseHandle(hSemaphore);

	return 0;
}

