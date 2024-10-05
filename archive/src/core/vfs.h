#pragma once
#include "defines.h"

#define MAX_VIRTUAL_FILENAME_LEN 256

typedef struct VFS_Pack VFS_Pack;

const char VFS_OpenErr_DoesNotExist[] = "PATH DOES NOT EXIST";

typedef struct VFS_File {
  size_t n_bytes;
  void* data;
} VFS_File;

// virtual file open result
typedef struct VFS_FileRes {
  bool success;
  const char* error_reason;
  VFS_File file;
} VFS_FileRes;

VFS_Pack* VFS_Open(const char* filepath);

bool VFS_Close(VFS_Pack*);

VFS_FileRes VFS_VirtualRead(VFS_Pack* vfs, const char* unique_path);

typedef struct VFS_PackBuilder {
  const char* pack_filename;
} VFS_PackBuilder;

typedef struct VFS_FileEntry {
  char filename[MAX_VIRTUAL_FILENAME_LEN];
  size_t offset;
  size_t size;
} VFS_FileEntry;

VFS_PackBuilder VFS_Pack_Create();