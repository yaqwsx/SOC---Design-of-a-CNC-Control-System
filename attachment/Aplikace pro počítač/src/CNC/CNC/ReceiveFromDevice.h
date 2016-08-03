#pragma once
#include <wx/wx.h>
#include "MyDevice.h"
#include "MainWindow.h"

class MainWindow;

const int MAX_SIZE = 8;//Max num. of packets for one message
const int errSleep = 100;//How long wait after error state? in ms

//Thread for receiving data from interpolator
class ReceiveFromDevice :
	public wxThread
{
	public:
		ReceiveFromDevice(void);
		~ReceiveFromDevice(void);
		ExitCode Entry();//Body of thread
		void ProcessMessage();//Processes message from device
		void Reset();//Rests state
		void SetDependences(MyDevice* d){device = d;};
	private:
		unsigned char data[IN_EP_PACKET*MAX_SIZE];
		size_t received;//Number of received bytes
		UCHAR receivedPackets;//Number of received packets
		MyDevice* device;//Pointer to device class
};

