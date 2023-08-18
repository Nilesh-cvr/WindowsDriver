

#include<ntifs.h>
#include "myDriverPriorityBooster.h"

#define DRIVER_TAG 'dcba'

//UNICODE_STRING g_RegistryPath;



NTSTATUS PriorityBoosterCreate(_In_ DEVICE_OBJECT devObj, _Inout_ PIRP irp);
NTSTATUS PriorityBoosterClose(_In_ DEVICE_OBJECT devObj, _Inout_ PIRP irp);
NTSTATUS PriorityBoosterDeviceControl(_In_ PDEVICE_OBJECT devObj, _Inout_ PIRP irp);


__drv_dispatchType(IRP_MJ_CREATE) DRIVER_DISPATCH PriorityBoosterCreate;
__drv_dispatchType(IRP_MJ_CLOSE) DRIVER_DISPATCH PriorityBoosterClose;
__drv_dispatchType(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH PriorityBoosterDeviceControl;


_Use_decl_annotations_ NTSTATUS PriorityBoosterCreate(_In_ DEVICE_OBJECT devObj, _Inout_ PIRP irp)
{
	UNREFERENCED_PARAMETER(devObj);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

_Use_decl_annotations_ NTSTATUS PriorityBoosterClose(_In_ DEVICE_OBJECT devObj, _Inout_ PIRP irp)
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
void myDriverUnload(PDRIVER_OBJECT drvObj)
{
	//UNREFERENCED_PARAMETER(drvObj);
	KdPrint(("Nilesh : myDriverUnload unloading..."));
//	ExFreePool(g_RegistryPath.Buffer);

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\DosDevices\\Example");
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(drvObj->DeviceObject);
}


extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT drvObj, _In_ PUNICODE_STRING RegPath) {
	KdPrint(("Nilesh : DriverEntry..."));

	UNREFERENCED_PARAMETER(RegPath);
	UNICODE_STRING usDriverName, usDosDeviceName;
	PDEVICE_OBJECT pDeviceObject = NULL;

	//Driver unload
	drvObj->DriverUnload = myDriverUnload;
	
	/*
	
	//Driver registry printing
	g_RegistryPath.Buffer = (WCHAR *)ExAllocatePool2(PagedPool,RegPath->Length, DRIVER_TAG);
	if (g_RegistryPath.Buffer == nullptr)
	{
		KdPrint(("Nilesh : Registrypath is Null..."));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	g_RegistryPath.MaximumLength = RegPath->MaximumLength;
	RtlCopyUnicodeString(&g_RegistryPath, (PCUNICODE_STRING)RegPath);
	DbgPrint("Nilesh : Registrypath is Null...%s", g_RegistryPath);
	*/

	//Get version info
	RTL_OSVERSIONINFOW versionInfo = {0};
	versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
	RtlGetVersion(PRTL_OSVERSIONINFOW(&versionInfo));
	DbgPrint("Nilesh : RtlGetVersion...versionInfo.dwBuildNumber = %lu, versionInfo.dwOSVersionInfoSize = %lu, versionInfo.dwPlatformId =%lu", versionInfo.dwBuildNumber, versionInfo.dwOSVersionInfoSize,
		versionInfo.dwPlatformId);
	
	

	//Priority of a process booster
	//drvObj->MajorFunction[IRP_MJ_CREATE] = PriorityBoosterCreate;
	//drvObj->MajorFunction[IRP_MJ_CLOSE] = PriorityBoosterClose;
	drvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PriorityBoosterDeviceControl;


	
	RtlInitUnicodeString(&usDriverName, L"\\Device\\Nilesh");
	RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevices\\Nilesh");

	NTSTATUS status = IoCreateDevice(drvObj, 0, &usDriverName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	if (status != 0) {
		KdPrint(("Failed to Create device Objec\n", status));
		return status;
	}
	else
		KdPrint(("Created device Objec\n", status));

	
	status = IoCreateSymbolicLink(&usDosDeviceName, &usDriverName);
	if (status != 0) {
		KdPrint(("Failed to Create Symbolic Link (0x%08X)\n", status));
		IoDeleteDevice(pDeviceObject);
		return status;
	}
	else
		KdPrint(("Created Symbolic Link(0x % 08X)\n", status));

	/*
	//Devicename
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\PriorityBooster");
	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(drvObj, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!status) {
		KdPrint(("Failed to Create device Objec\n", status));
		return status;
	}
	else
		KdPrint(("Created device Objec\n", status));
		
	//Symbolic link
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\DosDevice\\PriorityBooster");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!status) {
		KdPrint(("Failed to Create Symbolic Link (0x%08X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}
	else
		KdPrint(("Created Symbolic Link(0x % 08X)\n", status));

	*/
	
	return STATUS_SUCCESS;


}