#include <string.h>

#include <wafel/ios/svc.h>

#include "fsa.h"

#define CROSS_PROCESS_HEAP_ID 0xcaff
#define COPY_BUFFER_SIZE 1024

int FSA_Open()
{
    //return iosOpen("/dev/fsa", 0);
    return ((int (*const)())0x5035294)();
}

static void* allocIobuf()
{
	void* ptr = iosAlloc(0xCAFF, 0x828);

	memset(ptr, 0x00, 0x828);

	return ptr;
}

static void freeIobuf(void* ptr)
{
	iosFree(0xCAFF, ptr);
}


int FSA_OpenDir(int fd, const char* path, int* outHandle)
{
	u8* iobuf = allocIobuf();
	u32* inbuf = (u32*)iobuf;
	u32* outbuf = (u32*)&iobuf[0x520];

	strncpy((char*)&inbuf[0x01], path, 0x27F);

	int ret = iosIoctl(fd, 0x0A, inbuf, 0x520, outbuf, 0x293);

	if(outHandle) *outHandle = outbuf[1];

	freeIobuf(iobuf);
	return ret;
}

int FSA_CloseDir(int fd, int handle)
{
	u8* iobuf = allocIobuf();
	u32* inbuf = (u32*)iobuf;
	u32* outbuf = (u32*)&iobuf[0x520];

	inbuf[1] = handle;

	int ret = iosIoctl(fd, 0x0D, inbuf, 0x520, outbuf, 0x293);

	freeIobuf(iobuf);
	return ret;
}

int FSA_OpenFile(int fd, const char* path, const char* mode, int* outHandle)
{
	u8* iobuf = allocIobuf();
	u32* inbuf = (u32*)iobuf;
	u32* outbuf = (u32*)&iobuf[0x520];

	strncpy((char*)&inbuf[0x01], path, 0x27F);
	strncpy((char*)&inbuf[0xA1], mode, 0x10);

	int ret = iosIoctl(fd, 0x0E, inbuf, 0x520, outbuf, 0x293);

	if(outHandle) *outHandle = outbuf[1];

	freeIobuf(iobuf);
	return ret;
}

int FSA_CloseFile(int fd, int fileHandle)
{
    u8* iobuf = allocIobuf();
	u32* inbuf = (u32*)iobuf;
	u32* outbuf = (u32*)&iobuf[0x520];

	inbuf[1] = fileHandle;

	int ret = iosIoctl(fd, 0x15, inbuf, 0x520, outbuf, 0x293);
	freeIobuf(iobuf);
	return ret;
}

int _FSA_ReadWriteFile(int fd, void* data, u32 size, u32 cnt, int fileHandle, u32 flags, bool read)
{
	u8* iobuf = allocIobuf();
	u8* inbuf8 = iobuf;
	u8* outbuf8 = &iobuf[0x520];
	iovec_s* iovec = (iovec_s*)&iobuf[0x7C0];
	u32* inbuf = (u32*)inbuf8;
	u32* outbuf = (u32*)outbuf8;

	inbuf[0x08 / 4] = size;
	inbuf[0x0C / 4] = cnt;
	inbuf[0x14 / 4] = fileHandle;
	inbuf[0x18 / 4] = flags;

	iovec[0].ptr = inbuf;
	iovec[0].len = 0x520;

	iovec[1].ptr = data;
	iovec[1].len = size * cnt;

	iovec[2].ptr = outbuf;
	iovec[2].len = 0x293;

	int ret;
	if(read) ret = iosIoctlv(fd, 0x0F, 1, 2, iovec);
	else ret = iosIoctlv(fd, 0x10, 2, 1, iovec);

	freeIobuf(iobuf);
	return ret;
}

int FSA_ReadFile(int fd, void* data, u32 size, u32 cnt, int fileHandle, u32 flags)
{
	return _FSA_ReadWriteFile(fd, data, size, cnt, fileHandle, flags, true);
}

int FSA_WriteFile(int fd, void* data, u32 size, u32 cnt, int fileHandle, u32 flags)
{
	return _FSA_ReadWriteFile(fd, data, size, cnt, fileHandle, flags, false);
}

int copy_file(int fd, const char* src, const char* dst)
{
    int readHandle;
    int res = FSA_OpenFile(fd, src, "r", &readHandle);
    if (res < 0) {
        return res;
    }

    int writeHandle;
    res = FSA_OpenFile(fd, dst, "w", &writeHandle);
    if (res < 0) {
        FSA_CloseFile(fd, readHandle);
        return res;
    }

    void* dataBuffer = iosAllocAligned(CROSS_PROCESS_HEAP_ID, COPY_BUFFER_SIZE, 0x40);
    if (!dataBuffer) {
        FSA_CloseFile(fd, readHandle);
        FSA_CloseFile(fd, writeHandle);
        return -1;
    }

    while ((res = FSA_ReadFile(fd, dataBuffer, 1, COPY_BUFFER_SIZE, readHandle, 0)) > 0) {
        if ((res = FSA_WriteFile(fd, dataBuffer, 1, res, writeHandle, 0)) < 0) {
            break;
        }
    }

    iosFree(CROSS_PROCESS_HEAP_ID, dataBuffer);
    FSA_CloseFile(fd, writeHandle);
    FSA_CloseFile(fd, readHandle);

    return (res > 0) ? 0 : res;
}

int FSA_Mount(int fd, const char* device_path, const char* volume_path, u32 flags, char* arg_string, int arg_string_len)
{
	u8* iobuf = allocIobuf();
	u8* inbuf8 = iobuf;
	u8* outbuf8 = &iobuf[0x520];
	iovec_s* iovec = (iovec_s*)&iobuf[0x7C0];
	u32* inbuf = (u32*)inbuf8;
	u32* outbuf = (u32*)outbuf8;

	strncpy((char*)&inbuf8[0x04], device_path, 0x27F);
	strncpy((char*)&inbuf8[0x284], volume_path, 0x27F);
	inbuf[0x504 / 4] = (u32)flags;
	inbuf[0x508 / 4] = (u32)arg_string_len;

	iovec[0].ptr = inbuf;
	iovec[0].len = 0x520;
	iovec[1].ptr = arg_string;
	iovec[1].len = arg_string_len;
	iovec[2].ptr = outbuf;
	iovec[2].len = 0x293;

	int ret = iosIoctlv(fd, 0x01, 2, 1, iovec);

	freeIobuf(iobuf);
	return ret;
}

int FSA_Unmount(int fd, const char* path, u32 flags)
{
	u8* iobuf = allocIobuf();
	u32* inbuf = (u32*)iobuf;
	u32* outbuf = (u32*)&iobuf[0x520];

	strncpy((char*)&inbuf[0x01], path, 0x27F);
	inbuf[0x284 / 4] = flags;

	int ret = iosIoctl(fd, 0x02, inbuf, 0x520, outbuf, 0x293);

	freeIobuf(iobuf);
	return ret;
}

int FSA_FlushVolume(int fd, const char* volume)
{
	u8* iobuf = allocIobuf();
	u32* inbuf = (u32*)iobuf;
	u32* outbuf = (u32*)&iobuf[0x520];

	strncpy((char*)&inbuf[0x01], volume, 0x27F);

	int ret = iosIoctl(fd, 0x1B, inbuf, 0x520, outbuf, 0x293);

	freeIobuf(iobuf);
	return ret;
}

int FSA_Remove(int fd, const char* path){
	u8* iobuf = allocIobuf();
	u32* inbuf = (u32*)iobuf;
	u32* outbuf = (u32*)&iobuf[0x520];

	strncpy((char*)&inbuf[0x01], path, 0x27F);

	int ret = iosIoctl(fd, 0x08, inbuf, 0x520, outbuf, 0x293);

	freeIobuf(iobuf);
	return ret;
}
