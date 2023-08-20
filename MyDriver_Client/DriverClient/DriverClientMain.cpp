#include<iostream>
#include<windows.h>
#include<chrono>
#include<thread>

#include "myDriverPriorityBooster.h"

using namespace std;

int main(int argc, const char* argv[]) {
	if (argc < 3) {
		cout << "Not sufficient argument";
		return -1;
	}
	
	HANDLE hDevice = CreateFileW(DRIVER_WIN_NAME,
		 GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
		OPEN_EXISTING, 0, nullptr);

	if (INVALID_HANDLE_VALUE == hDevice)
	{
		cout << "Failed to open Handle: "<<GetLastError();
		return -1;
	}
		
	ThreadData data;
	data.ThreadId = atoi(argv[1]);
	data.Priority = atoi(argv[2]);


	DWORD returned;

	bool success = DeviceIoControl(hDevice,
		IOCTL_PRIORITY_BOOSTER_SET_PRIORITY,
		&data, sizeof(data),
		nullptr, 0,
		&returned, nullptr);

	if (success)
		cout << "Priority change succeed";
	else
		cout << "Priority change failed " << GetLastError();
	

	CloseHandle(hDevice);
	
	return 0;
}
