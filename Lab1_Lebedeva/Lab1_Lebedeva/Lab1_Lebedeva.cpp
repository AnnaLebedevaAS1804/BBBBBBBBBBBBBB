// Lab1_Lebedeva.cpp : This file contains the 'main' function. Program exitecution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "Lab1_Lebedeva.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#include <vector>
#include <string>
#include <thread>
using namespace std;

// Единственный объект приложения

CWinApp theApp;

HANDLE hMutex;
HANDLE hMutex_Map;
HANDLE hEventClose;
HANDLE hEventMess;
vector<HANDLE> vEventClose;
vector<HANDLE> vEventMess;
HANDLE hEvents[5];

struct Header
{
	int m_to;
	int m_size;
};

void WorkWithFile(int Thread_id, string mess)
{
	Header h{ 123, mess.length() + 1 };
	string filename = to_string(Thread_id) + ".dat";
	WaitForSingleObject(hMutex_Map, INFINITE);
	HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS ,0, NULL);
	//proverka
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	//WaitForSingleObject(hMutex_Map, INFINITE);
	HANDLE hFileMap = CreateFileMapping( hFile, NULL, PAGE_READWRITE, 0, sizeof(h)+h.m_size,"MyFile");
	LPVOID pBuff = (BYTE*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	memcpy(pBuff, &h, sizeof(h));
	memcpy((char*)pBuff + sizeof(h), mess.c_str(), h.m_size);
	CloseHandle(hFileMap);
	CloseHandle(hFile);
	ReleaseMutex(hMutex_Map);
}
// функция потока
UINT/*DWORD*/ /*WINAPI*/ MyThread(LPVOID Parameter)
{
    int ID = (int)Parameter;

    WaitForSingleObject(hMutex, INFINITE);
    cout << "Поток " << ID << " создан" << endl;
    ReleaseMutex(hMutex);
	HANDLE hEvents_new[2] = { vEventClose[ID], vEventMess[ID] };
	//int eventnumber = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
	while (true)
	{
		int eventnumber = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
		switch (eventnumber)
		{
		case 0:
		{
			//WaitForSingleObject(vEventClose[(int)Parameter], INFINITE);

			WaitForSingleObject(hMutex, INFINITE);
			cout << "Поток " << ID << " завершен" << endl;
			ReleaseMutex(hMutex);
		}
		case 1:
		{
			//WaitForSingleObject(vEventMess[(int)Parameter], INFINITE);
			WorkWithFile(ID, "smth is missing");
			break;
		}
		}
	}
}

void start()
{
	cout << "Все работает." << endl;
	hMutex = CreateMutex(NULL, FALSE, "mutex");
	hMutex_Map = CreateMutex(NULL, FALSE, "My_mutex");
	//HANDLE hEvents[5];
	hEvents[0] = CreateEvent(NULL, FALSE, FALSE, "event_start");
	hEvents[1] = CreateEvent(NULL, FALSE, FALSE, "event_stop");
	hEvents[2] = CreateEvent(NULL, FALSE, FALSE, "event_quit");//выход и подчистить хвосты
	hEvents[3] = CreateEvent(NULL, FALSE, FALSE, "event_messege");
	hEvents[4] = CreateEvent(NULL, FALSE, FALSE, "event_confirm");//ожидание
	
	int k = 0;
	bool exit = false;

	while (!exit)
	{
		// Ожидание события
		int evNum = WaitForMultipleObjects(4, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;

		switch (evNum)
		{
		case 0:
		{
			hEventClose = CreateEvent(NULL, FALSE, FALSE, NULL);
			hEventMess = CreateEvent(NULL, FALSE, FALSE, NULL);
			vEventClose.push_back(hEventClose);
			vEventClose.push_back(hEventMess);
			AfxBeginThread(MyThread,(LPVOID)k++);
			//CreateThread(NULL, 0, MyThread, (LPVOID)k++, 0, NULL);
			break;
		}
		case 1:
		{
			// Если вектор потоков пуст, переходим на следующий case
			if (!vEventClose.empty())
			{
				SetEvent(vEventClose[--k]);

				CloseHandle(vEventClose[k]);
				vEventClose.pop_back();

				CloseHandle(vEventMess[k]);
				vEventMess.pop_back();
				break;
			}
		}
		case 2:
		{
			cout << "Закрытие приложения." << endl;
			exit = true;
			break;
		}
		case 3:
		{
			//int ;
			if (k == -2)//main
			{
				//cout << "Сообщение главного потока" << mess <<endl;
			}
			else if (k == -1)//all
			{
				for (auto event : vEventMess)
					SetEvent(event);
				//cout << "Сообщение всех потоков" << mess <<endl;
			}
			else //one
			{
				SetEvent(vEventMess[k]);
			}
				
		}
		}
		SetEvent(hEvents[4]);
	}
	for (auto event : vEventClose)
	{
		SetEvent(event);
		CloseHandle(event);
	}
	for (auto event : vEventMess)
	{
		CloseHandle(event);
	}
	for (auto event : hEvents)
	{
		CloseHandle(event);
	}

	CloseHandle(hMutex);
	CloseHandle(hMutex_Map);
}



int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: code your application's behavior here.
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else
        {
			setlocale(LC_ALL, "Russian");
			start();
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
