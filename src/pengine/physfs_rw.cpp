
// physfs_rw.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"



int physfs_seek(SDL_RWops *context, int offset, int whence)
{
  PHYSFS_file *pfile = (PHYSFS_file *)context->hidden.unknown.data1;
  
  int target;
  
  int curpos = PHYSFS_tell(pfile);
  
  switch (whence) {
  default:
  case SEEK_SET:
    target = offset;
    break;
  case SEEK_CUR:
    target = curpos + offset;
    break;
  case SEEK_END:
    target = PHYSFS_fileLength(pfile) + offset;
    break;
  }
  
    int result = PHYSFS_seek(pfile, target);
    if (! result) {
        throw MakePException("Error seeking: " + PHYSFS_getLastError());
    }
    
    return PHYSFS_tell(pfile);
  
    PHYSFS_seek(pfile, target);
  
  return curpos;
}


int physfs_read(SDL_RWops *context, void *ptr, int size, int maxnum)
{
  PHYSFS_file *pfile = (PHYSFS_file *)context->hidden.unknown.data1;
  
  return PHYSFS_read(pfile, ptr, size, maxnum);
}


int physfs_write(SDL_RWops *context, const void *ptr, int size, int num)
{
  PHYSFS_file *pfile = (PHYSFS_file *)context->hidden.unknown.data1;
  
  return PHYSFS_write(pfile, ptr, size, num);
}


int physfs_close(SDL_RWops *context)
{
  PHYSFS_file *pfile = (PHYSFS_file *)context->hidden.unknown.data1;
  
  PHYSFS_close(pfile);
  
  SDL_FreeRW(context);
  
  return 0;
}


