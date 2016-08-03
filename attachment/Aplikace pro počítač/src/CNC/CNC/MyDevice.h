#pragma once

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include "global.h"
#include <wx\wx.h>
#include <vector>


class MainWindow;


//WinUSB header
#include <winusb.h>
#include <Usb100.h>
#include <Setupapi.h>
#include <Dbt.h>

/*Class for connecting and disconnection usb device*/
const UCHAR OUT_EP = 1;
const UCHAR IN_EP = 129;
const UCHAR IN_EP_PACKET = 64;
const UCHAR OUT_EP_PACKET = 64;

class ReceiveFromDevice;

class MyDevice
{
	public:
		MyDevice(void);
		~MyDevice(void);
		void OnConnect();
		void OnDisconnect();
		bool TestIfConnected();
		bool TryToConnect();
		void Init();
		bool GetState() {return attached;};
		bool SendRawData(std::vector<unsigned char>& data);
		bool SendRawData(unsigned char* data, size_t length);
		size_t GetFreeSpaceInQueue();
		uint32_t GetLastProcessedItemStack();
		uint32_t GetCurrentlyProcessedItemStack();
	private:
		bool InitDevice();
		bool DeInitDevice();
		WINUSB_INTERFACE_HANDLE WinUSBHandle;//Handle for WinUSB
		bool attached;//Device state
		ReceiveFromDevice* receive;//Receive, new thread

		//State of the queue
		wxMutex freeSpaceM;
		wxCondition freeSpaceC;
		size_t freeSpace;

		//Last processsed item from stack
		wxMutex lastItemStackM;
		wxCondition lastItemStackC;
		uint32_t lastItemStack;

		//currently processsed item from stack
		wxMutex curItemStackM;
		wxCondition curItemStackC;
		uint32_t curItemStack;
	friend class ReceiveFromDevice;
};
