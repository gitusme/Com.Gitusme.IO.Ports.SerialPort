/*********************************************************
 * Copyright (c) 2019-2023 gitusme, All rights reserved.
 *********************************************************/

#include "NativeSerialPort.h"
#include <stdio.h>
#include <stdlib.h>
#include <vcclr.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Com {
	namespace Gitusme {
		namespace IO {
			namespace Ports {

				public enum class Parity
				{
					None = 0,
					Odd = 1,
					Even = 2,
					Mark = 3,
					Space = 4
				};

				/// <summary>
				/// ���ڲ������ýӿ�
				/// </summary>
				public ref class SerialPortSettings
				{
				public:
					SerialPortSettings() {
					}
					~SerialPortSettings() {
					}

				public:
					/// <summary>
					/// ʹ��ͬ��ģʽ��Ĭ��falseΪ�첽
					/// </summary>
					bool Synchronizable = true;
					/// <summary>
					/// �����ʣ�9600��19200��38400��43000��56000��57600��115200
					/// </summary>
					int BaudRate = 115200;
					/// <summary>
					/// У��λ��0Ϊ��У�飬1Ϊ��У�飬2ΪżУ�飬3Ϊ���У��
					/// </summary>
					Parity Parity = Parity::None;
					/// <summary>
					/// ����λ��4-8��ͨ��Ϊ8λ
					/// </summary>
					int DataBits = 8;
					/// <summary>
					/// ֹͣλ��1Ϊ1λֹͣλ��2Ϊ2λֹͣλ,3Ϊ1.5λֹͣλ
					/// </summary>
					int StopBits = 1;
				};

				/// <summary>
				/// ���ڲ����ӿ�
				/// </summary>
				public ref class SerialPort
				{
				public:
					SerialPort() {
						_isOpen = false;
						_settings = gcnew SerialPortSettings();
					}

					~SerialPort() {
						Close();
					}

					/// <summary>
					/// �򿪴���
					/// </summary>
					/// <param name="portName">����������Windows����"COM1""COM2"�ȣ���Linux����"/dev/ttyS1"��</param>
					bool Open(String^ portName) {
						if (portName == nullptr) {
							throw gcnew NullReferenceException("portName is null");
						}
						if (_isOpen) {
							return true;
						}

						pin_ptr<const wchar_t> nativePortName = PtrToStringChars(portName);
						_isOpen = _api->Open(nativePortName,
							Settings->BaudRate,
							(int)(Settings->Parity),
							Settings->DataBits,
							Settings->StopBits,
							Settings->Synchronizable);
						return _isOpen;
					}

					/// <summary>
					/// �رմ���
					/// </summary>
					void Close() {
						if (_isOpen) {
							_api->Close();
						}
					}

					/// <summary>
					/// �Ӵ��ڶ�ȡ����
					/// </summary>
					/// <param name="buffer"></param>
					/// <returns></returns>
					int Read(array<Byte>^ buffer) {
						if (_isOpen) {
							pin_ptr<System::Byte> ptr = &buffer[0];
							char* data = reinterpret_cast<char*>(ptr);
							return _api->Read(data, buffer->Length);
						}
						return 0;
					}

					/// <summary>
					/// �򴮿�д������
					/// </summary>
					/// <param name="buffer"></param>
					/// <returns></returns>
					int Write(array<Byte>^ buffer) {
						if (_isOpen) {
							pin_ptr<System::Byte> ptr = &buffer[0];
							char* data = reinterpret_cast<char*>(ptr);
							return _api->Write(data);
						}
						return 0;
					}

				public:
					/// <summary>
					/// ���ڴ�״̬
					/// </summary>
					property bool IsOpen {
						bool get() {
							return _isOpen;
						}
					}

					/// <summary>
					/// ���ڲ�������
					/// </summary>
					property SerialPortSettings^ Settings {
						SerialPortSettings^ get() {
							return _settings;
						}
						void set(SerialPortSettings^ value) {
							_settings = value;
						}
					}

				private:
					NativeSerialPort* _api = new NativeSerialPort();
					bool _isOpen;
					SerialPortSettings^ _settings;
				};

			}
		}
	}
}