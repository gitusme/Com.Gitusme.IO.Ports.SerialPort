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
				/// 串口参数配置接口
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
					/// 使用同步模式，默认false为异步
					/// </summary>
					bool Synchronizable = true;
					/// <summary>
					/// 波特率，9600、19200、38400、43000、56000、57600、115200
					/// </summary>
					int BaudRate = 115200;
					/// <summary>
					/// 校验位，0为无校验，1为奇校验，2为偶校验，3为标记校验
					/// </summary>
					Parity Parity = Parity::None;
					/// <summary>
					/// 数据位，4-8，通常为8位
					/// </summary>
					int DataBits = 8;
					/// <summary>
					/// 停止位，1为1位停止位，2为2位停止位,3为1.5位停止位
					/// </summary>
					int StopBits = 1;
				};

				/// <summary>
				/// 串口操作接口
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
					/// 打开串口
					/// </summary>
					/// <param name="portName">串口名，在Windows下是"COM1""COM2"等，在Linux下是"/dev/ttyS1"等</param>
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
					/// 关闭串口
					/// </summary>
					void Close() {
						if (_isOpen) {
							_api->Close();
						}
					}

					/// <summary>
					/// 从串口读取数据
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
					/// 向串口写入数据
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
					/// 串口打开状态
					/// </summary>
					property bool IsOpen {
						bool get() {
							return _isOpen;
						}
					}

					/// <summary>
					/// 串口参数配置
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