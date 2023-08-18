#ifndef _MYDRIVERPRIORITY_BOOSTER_H_
#define _MYDRIVERPRIORITY_BOOSTER_H_




#define PRIORITY_BOOSTER_DEVICE 0x8000

#define IOCTL_PRIORITY_BOOSTER_SET_PRIORITY  CTL_CODE(PRIORITY_BOOSTER_DEVICE, 0x800, \
		METHOD_NEITHER,FILE_ANY_ACCESS)

//typedef unsigned long ULONG;

struct ThreadData {
	ULONG ThreadId;
	int Priority;
};



#endif
