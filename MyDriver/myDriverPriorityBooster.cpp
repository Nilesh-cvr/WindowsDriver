

#include<ntifs.h>
#include "myDriverPriorityBooster.h"


__drv_dispatchType(IRP_MJ_CREATE) DRIVER_DISPATCH PriorityBoosterCreate;
__drv_dispatchType(IRP_MJ_CLOSE) DRIVER_DISPATCH PriorityBoosterClose;
__drv_dispatchType(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH PriorityBoosterDeviceControl;


_Use_decl_annotations_ NTSTATUS PriorityBoosterCreateClose(PDEVICE_OBJECT devObj, PIRP irp)
 {
	 UNREFERENCED_PARAMETER(devObj);
	 irp->IoStatus.Status = STATUS_SUCCESS;
	 irp->IoStatus.Information = 0;
	 IoCompleteRequest(irp, IO_NO_INCREMENT);
	 KdPrint(("PriorityBoosterClose"));
	 return STATUS_SUCCESS;
 }

_Use_decl_annotations_  NTSTATUS PriorityBoosterDeviceControl(_In_ PDEVICE_OBJECT devObj, _Inout_ PIRP irp)
{
	UNREFERENCED_PARAMETER(devObj);
	auto stack = IoGetCurrentIrpStackLocation(irp);
	auto status = STATUS_SUCCESS;
	KdPrint(("PriorityBoosterDeviceControl"));
	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_PRIORITY_BOOSTER_SET_PRIORITY: {
			auto len = stack->Parameters.DeviceIoControl.InputBufferLength;
			if (len < sizeof(ThreadData))
			{
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			auto data = (ThreadData *)stack->Parameters.DeviceIoControl.Type3InputBuffer;
			if(data == nullptr)
			{
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			PETHREAD myThread;
			status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &myThread);
			if (!NT_SUCCESS(status))
				break;

			KeSetPriorityThread((PKTHREAD)myThread, data->Priority);
			ObDereferenceObject(myThread);

			KdPrint(("Thread Prority chane for %d to %d succeed\n", data->ThreadId, data->Priority));
			break;
		}
		default:
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = 0 ;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return status;
}

//Unloading driver and it's resources
void myDriverUnload(_In_ PDRIVER_OBJECT drvObj)
{
	KdPrint(("MyDriver : myDriverUnload unloading..."));

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\PrioBoss");
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(drvObj->DeviceObject);
}


extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT drvObj, _In_ PUNICODE_STRING RegPath) {
	KdPrint(("MyDriver : DriverEntry..."));

	UNREFERENCED_PARAMETER(RegPath);

	//Driver unload
	drvObj->DriverUnload = myDriverUnload;
	
	//Priority of a process booster
	drvObj->MajorFunction[IRP_MJ_CREATE] = PriorityBoosterCreateClose;
	drvObj->MajorFunction[IRP_MJ_CLOSE] = PriorityBoosterCreateClose;
	drvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PriorityBoosterDeviceControl;


	//Devicename
	UNICODE_STRING devName = RTL_CONSTANT_STRING(DEVICE_NAME);
	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(drvObj, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (status) {
		KdPrint(("Failed to Create device Object\n", status));
		return status;
	}
	else
		KdPrint(("Created device Object\n", status));
		
	//Symbolic link
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(DRIVER_DOS_NAME);
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (status) {
		KdPrint(("Failed to Create Symbolic Link (0x%08X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}
	else
		KdPrint(("Created Symbolic Link(0x % 08X)\n", status));

	
	return STATUS_SUCCESS;


}
