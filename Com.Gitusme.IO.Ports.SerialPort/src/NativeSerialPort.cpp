/*********************************************************
 * Copyright (c) 2019-2023 gitusme, All rights reserved.
 *********************************************************/

#include "NativeSerialPort.h"
#include <windows.h>
#include <string>
#include <malloc.h>

using namespace std;

NativeSerialPort::NativeSerialPort()
{
}

NativeSerialPort::~NativeSerialPort()
{
}

bool NativeSerialPort::Open(const wchar_t* portname,
	int baudrate,
	char parity,
	char databit,
	char stopbit,
	char synchronizeflag)
{
	this->synchronizeflag = synchronizeflag;
	HANDLE hCom = NULL;
	if (this->synchronizeflag)
	{
		//ͬ����ʽ
		hCom = CreateFile(portname, //������
			GENERIC_READ | GENERIC_WRITE, //֧�ֶ�д
			0, //��ռ��ʽ�����ڲ�֧�ֹ���
			NULL,//��ȫ����ָ�룬Ĭ��ֵΪNULL
			OPEN_EXISTING, //�����еĴ����ļ�
			0, //0��ͬ����ʽ��FILE_FLAG_OVERLAPPED���첽��ʽ
			NULL);//���ڸ����ļ������Ĭ��ֵΪNULL���Դ��ڶ��Ըò���������ΪNULL
	}
	else
	{
		//�첽��ʽ
		hCom = CreateFile(portname, //������
			GENERIC_READ | GENERIC_WRITE, //֧�ֶ�д
			0, //��ռ��ʽ�����ڲ�֧�ֹ���
			NULL,//��ȫ����ָ�룬Ĭ��ֵΪNULL
			OPEN_EXISTING, //�����еĴ����ļ�
			FILE_FLAG_OVERLAPPED, //0��ͬ����ʽ��FILE_FLAG_OVERLAPPED���첽��ʽ
			NULL);//���ڸ����ļ������Ĭ��ֵΪNULL���Դ��ڶ��Ըò���������ΪNULL
	}

	if (hCom == (HANDLE)-1)
	{
		return false;
	}

	//���û�������С 
	if (!SetupComm(hCom, 1024, 1024))
	{
		return false;
	}

	// ���ò��� 
	DCB p;
	memset(&p, 0, sizeof(p));
	p.DCBlength = sizeof(p);
	p.BaudRate = baudrate; // ������
	p.ByteSize = databit; // ����λ

	switch (parity) //У��λ
	{
	case 0:
		p.Parity = NOPARITY; //��У��
		break;
	case 1:
		p.Parity = ODDPARITY; //��У��
		break;
	case 2:
		p.Parity = EVENPARITY; //żУ��
		break;
	case 3:
		p.Parity = MARKPARITY; //���У��
		break;
	}

	switch (stopbit) //ֹͣλ
	{
	case 1:
		p.StopBits = ONESTOPBIT; //1λֹͣλ
		break;
	case 2:
		p.StopBits = TWOSTOPBITS; //2λֹͣλ
		break;
	case 3:
		p.StopBits = ONE5STOPBITS; //1.5λֹͣλ
		break;
	}

	if (!SetCommState(hCom, &p))
	{
		// ���ò���ʧ��
		return false;
	}

	//��ʱ����,��λ������
	//�ܳ�ʱ��ʱ��ϵ��������д���ַ�����ʱ�䳣��
	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = 1000; //�������ʱ
	TimeOuts.ReadTotalTimeoutMultiplier = 500; //��ʱ��ϵ��
	TimeOuts.ReadTotalTimeoutConstant = 5000; //��ʱ�䳣��
	TimeOuts.WriteTotalTimeoutMultiplier = 500; // дʱ��ϵ��
	TimeOuts.WriteTotalTimeoutConstant = 2000; //дʱ�䳣��
	SetCommTimeouts(hCom, &TimeOuts);

	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);//��մ��ڻ�����

	memcpy(pHandle, &hCom, sizeof(hCom));// ������

	return true;
}

void NativeSerialPort::Close()
{
	HANDLE hCom = *(HANDLE*)pHandle;
	CloseHandle(hCom);
}

int NativeSerialPort::Write(char* data)
{
	HANDLE hCom = *(HANDLE*)pHandle;

	if (this->synchronizeflag)
	{
		// ͬ����ʽ
		DWORD dwBytesWrite = strlen(data); //�ɹ�д��������ֽ���
		BOOL bWriteStat = WriteFile(hCom, //���ھ��
			data, //�����׵�ַ
			dwBytesWrite, //Ҫ���͵������ֽ���
			&dwBytesWrite, //DWORD*���������շ��سɹ����͵������ֽ���
			NULL); //NULLΪͬ�����ͣ�OVERLAPPED*Ϊ�첽����
		if (!bWriteStat)
		{
			return 0;
		}
		return dwBytesWrite;
	}
	else
	{
		//�첽��ʽ
		DWORD dwBytesWrite = strlen(data); //�ɹ�д��������ֽ���
		DWORD dwErrorFlags; //�����־
		COMSTAT comStat; //ͨѶ״̬
		OVERLAPPED m_osWrite; //�첽��������ṹ��

		//����һ������OVERLAPPED���¼����������������õ�����ϵͳҪ����ô��
		memset(&m_osWrite, 0, sizeof(m_osWrite));
		m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, L"WriteEvent");

		ClearCommError(hCom, &dwErrorFlags, &comStat); //���ͨѶ���󣬻���豸��ǰ״̬
		BOOL bWriteStat = WriteFile(hCom, //���ھ��
			data, //�����׵�ַ
			dwBytesWrite, //Ҫ���͵������ֽ���
			&dwBytesWrite, //DWORD*���������շ��سɹ����͵������ֽ���
			&m_osWrite); //NULLΪͬ�����ͣ�OVERLAPPED*Ϊ�첽����
		if (!bWriteStat)
		{
			if (GetLastError() == ERROR_IO_PENDING) //�����������д��
			{
				WaitForSingleObject(m_osWrite.hEvent, 1000); //�ȴ�д���¼�1����
			}
			else
			{
				ClearCommError(hCom, &dwErrorFlags, &comStat); //���ͨѶ����
				CloseHandle(m_osWrite.hEvent); //�رղ��ͷ�hEvent�ڴ�
				return 0;
			}
		}
		return dwBytesWrite;
	}
}

int NativeSerialPort::Read(char* buff, int buffSize)
{
	HANDLE hCom = *(HANDLE*)pHandle;
	if (this->synchronizeflag)
	{
		//ͬ����ʽ
		DWORD dwBytesRead = buffSize; //�ɹ���ȡ�������ֽ���
		BOOL bReadStat = ReadFile(hCom, //���ھ��
			buff, //�����׵�ַ
			dwBytesRead, //Ҫ��ȡ����������ֽ���
			&dwBytesRead, //DWORD*,�������շ��سɹ���ȡ�������ֽ���
			NULL); //NULLΪͬ�����ͣ�OVERLAPPED*Ϊ�첽����
		if (!bReadStat)
		{
			return 0;
		}
		return dwBytesRead;
	}
	else
	{
		//�첽��ʽ
		DWORD dwBytesRead = buffSize; //�ɹ���ȡ�������ֽ���
		DWORD dwErrorFlags; //�����־
		COMSTAT comStat; //ͨѶ״̬
		OVERLAPPED m_osRead; //�첽��������ṹ��

		//����һ������OVERLAPPED���¼����������������õ�����ϵͳҪ����ô��
		memset(&m_osRead, 0, sizeof(m_osRead));
		m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, L"ReadEvent");

		ClearCommError(hCom, &dwErrorFlags, &comStat); //���ͨѶ���󣬻���豸��ǰ״̬
		if (!comStat.cbInQue)return 0; //������뻺�����ֽ���Ϊ0���򷵻�false
		//std::cout << comStat.cbInQue << std::endl;
		BOOL bReadStat = ReadFile(hCom, //���ھ��
			buff, //�����׵�ַ
			dwBytesRead, //Ҫ��ȡ����������ֽ���
			&dwBytesRead, //DWORD*,�������շ��سɹ���ȡ�������ֽ���
			&m_osRead); //NULLΪͬ�����ͣ�OVERLAPPED*Ϊ�첽����
		if (!bReadStat)
		{
			if (GetLastError() == ERROR_IO_PENDING) //����������ڶ�ȡ��
			{
				//GetOverlappedResult���������һ��������ΪTRUE
				//������һֱ�ȴ���ֱ����������ɻ����ڴ��������
				GetOverlappedResult(hCom, &m_osRead, &dwBytesRead, TRUE);
			}
			else
			{
				ClearCommError(hCom, &dwErrorFlags, &comStat); //���ͨѶ����
				CloseHandle(m_osRead.hEvent); //�رղ��ͷ�hEvent���ڴ�
				return 0;
			}
		}
		return dwBytesRead;
	}
}