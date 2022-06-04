#include "page_size.h"

#include <windows.h>

u4 GetPageSizeInBytesFromOS() {
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwPageSize;
}
