#include <wafel/types.h>

int FSA_Open();
int FSA_Mount(int fd, const char* device_path, const char* volume_path, u32 flags, char* arg_string, int arg_string_len);
int FSA_Unmount(int fd, const char* path, u32 flags);
int FSA_FlushVolume(int fd, const char* volume);
int FSA_OpenDir(int fd, const char* path, int* outHandle);
int FSA_CloseDir(int fd, int handle);
int FSA_OpenFile(int fd, const char* path, const char* mode, int* outHandle);
int FSA_ReadFile(int fd, void* data, u32 size, u32 cnt, int fileHandle, u32 flags);
int FSA_WriteFile(int fd, void* data, u32 size, u32 cnt, int fileHandle, u32 flags);
int FSA_CloseFile(int fd, int fileHandle);
int copy_file(int fsaFd, const char* src, const char* dst);
int FSA_Remove(int fd, const char* path);
