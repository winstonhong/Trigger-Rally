
// util.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"

/*! Get token and value from a string line. The token is the string
 * before first space. The value, is the remaining string
 * \return true if could extract token and value */
bool PUtil::getToken(std::string line, std::string& tok, std::string& value)
{
   size_t ns;
   tok = "";
   value = "";

   /* Remove initial spaces from string */
   ns = line.find_first_not_of(" \t");
   if(ns != std::string::npos)
   {
      line = line.substr(ns);
   }

   /* Find first space */
   ns = line.find_first_of(" \t");
   if(ns != std::string::npos)
   {
      /* Set token (before space) and value (next space) */
      tok = line.substr(0, ns);

      if(ns+1 < line.length())
      {
         value = line.substr(ns+1);
         /* Remove trail \n, if any. */
         if(value[value.length()-1] == '\n')
         {
            value = value.substr(0, value.length()-1);
         }
      }
      else
      {
         value = "";
      }
      return(true);
   }
   return(false);
}

char* PUtil::fgets2(char *s, int size, PHYSFS_file *pfile)
{
  int i;
  for (i = 0; i < size-1; i++) {
    
    // check for EOF
    if (PHYSFS_eof(pfile)) return null;
    
    int ret = PHYSFS_read(pfile, s + i, 1, 1);
    
    if (s[i] == '\n') break;
    
    if (ret == -1) return null; // major error
    if (ret == 0) { i--; break; } // er, must be end of file anyway
  }
  
  s[i+1] = '\0';
  
  return s;
}

std::string PUtil::extractPathFromFilename(const std::string &filename)
{
  std::string::size_type found = filename.find_last_of('/');

  if (found == std::string::npos) return std::string("");

  return filename.substr(0, found + 1);
}

std::string PUtil::assemblePath(const std::string &relativefile, const std::string &parentfile)
{
  // This function is required because PhysFS doesn't allow ".."s in path names
  
  std::string totalpath;
  
  if (relativefile[0] == '/') {
    // relativefile is an absolute path
    totalpath = relativefile.substr(1);
  } else {
    // relativefile is indeed relative, so concat with parent path
    totalpath = extractPathFromFilename(parentfile) + relativefile;
  }
  
  std::string totalpath_hold = totalpath;
  
  std::string::size_type found;
  
  while (true) {
    
    // attempt to find a ".." to crunch back
    
    found = totalpath.find("../");
    
    if (found == std::string::npos) break; // no more ".."s
    
    if (found < 2) {
      // ".." is too close to start of path
      PUtil::outLog() << "Error: above local root: \"" << totalpath_hold << "\"" << std::endl;
      return std::string();
    }
    
    std::string::size_type crunch = totalpath.substr(0, found-1).find_last_of('/');
    
    if (crunch == std::string::npos) {
      // Crunch back to root (special case)
      crunch = 0;
      found += 1;
    }
    
    // remove the "/dir/.." chunk
    
    totalpath.erase(crunch, found + 2 - crunch);
  }
  
  return totalpath;
}


TiXmlElement *PUtil::loadRootElement(TiXmlDocument &doc, const char *rootName)
{
  // Old direct file code
  /*
  if (!doc.LoadFile()) {
    PUtil::outLog() << "Load failed: TinyXML couldn't load file, possibly file not found or invalid XML" << std::endl;
    PUtil::outLog() << "TinyXML: " << xmlfile.ErrorDesc() << std::endl;
    return null;
  }
  */
  
  // New PhysFS handler
  
  PHYSFS_file *pfile = PHYSFS_openRead(doc.Value());
  if (pfile == null) {
    PUtil::outLog() << "Load failed: PhysFS: " << PHYSFS_getLastError() << std::endl;
    return null;
  }
  
  int filesize = PHYSFS_fileLength(pfile);
  
  char *xmlbuffer = new char[filesize + 1];
  
  PHYSFS_read(pfile, xmlbuffer, filesize, 1);
  PHYSFS_close(pfile);
  
  xmlbuffer[filesize] = '\0';
  
  doc.Parse(xmlbuffer);
  
  delete [] xmlbuffer;
  
  TiXmlNode *rootnode;
  rootnode = doc.FirstChild(rootName);
  if (!rootnode) {
    PUtil::outLog() << "Couldn't process <" << rootName << "> element, possibly invalid file type" << std::endl;
    PUtil::outLog() << "TinyXML: " << doc.ErrorDesc() << std::endl;
    return null;
  }

  TiXmlElement *rootelem = rootnode->ToElement();
  if (!rootelem) {
    PUtil::outLog() << "Load failed: TinyXML error" << std::endl;
    PUtil::outLog() << "TinyXML: " << doc.ErrorDesc() << std::endl;
    return null;
  }

  return rootelem;
}


bool PUtil::copyFile(const std::string &fileFrom, const std::string &fileTo)
{
  PUtil::outLog() << "Copying \"" << fileFrom << "\" to \"" << fileTo << "\"" << std::endl;
  
  // PhysFS doesn't implement copy, so do it the hard way
  
  PHYSFS_file *pfile_from, *pfile_to;
  
  // Open source file
  
  pfile_from = PHYSFS_openRead(fileFrom.c_str());
  
  if (!pfile_from) {
    PUtil::outLog() << "Copy failed: PhysFS: " << PHYSFS_getLastError() << std::endl;
    return false;
  }
  
  // Make dest directory
  
  std::string filepath = extractPathFromFilename(fileTo);
  
  if (!PHYSFS_mkdir(filepath.c_str())) {
    const char *errstr = PHYSFS_getLastError();
    if (strcmp(errstr, "File exists")) {
      PUtil::outLog() << "Couldn't mkdir \"" << filepath << "\", attempting copy anyway" << std::endl;
      PUtil::outLog() << "PhysFS: " << errstr << std::endl;
    }
  }
  
  // Open dest file
  
  pfile_to = PHYSFS_openWrite(fileTo.c_str());
  
  if (!pfile_to) {
    PHYSFS_close(pfile_from);
    PUtil::outLog() << "Copy failed: PhysFS: " << PHYSFS_getLastError() << std::endl;
    return false;
  }
  
  // Copy over the data in chunks
  
  const int blocksize = 4096;
  char block[blocksize];

  int readcount;  
  do {
    
    readcount = PHYSFS_read(pfile_from, block, 1, blocksize);
    
    PHYSFS_write(pfile_to, block, 1, readcount);
    
  } while (readcount == blocksize);
  
  // Close up shop
  
  PHYSFS_close(pfile_from);
  PHYSFS_close(pfile_to);
  
  return true;
}

std::list<std::string> PUtil::findFiles(const std::string &basedir, const std::string &extension)
{
  std::list<std::string> results;
  
  char **filelist = PHYSFS_enumerateFiles(basedir.c_str());
  
  for (char **i = filelist; *i; i++) {
    
    std::string thisfile = basedir + '/' + *i;
    
    if (PHYSFS_isDirectory(thisfile.c_str())) {
      
      // Recurse into subdirectory
      std::list<std::string> moreresults = findFiles(thisfile, extension);
      results.insert(results.end(), moreresults.begin(), moreresults.end());
      
    } else {
      
      // Check to see if file has correct extension
      if (thisfile.length() >= extension.length() &&
        thisfile.substr(thisfile.length() - extension.length()) == extension)
        results.push_back(thisfile);
    }
  }
  
  PHYSFS_freeList(filelist);
  
  return results;
}

std::string PUtil::formatInt(int value, int width)
{
  std::string text(width, '0');
  
  for (std::string::reverse_iterator c = text.rbegin(); c != text.rend(); c++) {
    
    *c = '0' + (value % 10);
    
    value /= 10;
  }
  
  return text;
}

std::string PUtil::formatInt(int value)
{
  std::string text;
  
  bool neg = false;
  
  if (value < 0) { neg = true; value = -value; }
  else if (value == 0) return std::string ("0");
  
  for ( ; value; value /= 10)
    text = ((char)('0' + (value % 10))) + text;
  
  return (neg) ? ('-' + text) : (text);
}

std::string PUtil::formatTime(float seconds)
{
  int time_mins = (int)(seconds / 60.0f);
  seconds -= time_mins * 60;
  int time_secs = (int)(seconds);
  seconds -= time_secs;
  int time_centis = (int)(seconds * 100.0f);
  
  return
    formatInt(time_mins, 2) + ':' +
    formatInt(time_secs, 2) + '.' +
    formatInt(time_centis, 2);
}


// These are implemented over in physfs_rw.cpp

int physfs_seek(SDL_RWops *context, int offset, int whence);
int physfs_read(SDL_RWops *context, void *ptr, int size, int maxnum);
int physfs_write(SDL_RWops *context, const void *ptr, int size, int num);
int physfs_close(SDL_RWops *context);

SDL_RWops *PUtil::allocPhysFSops(PHYSFS_file *pfile)
{
  SDL_RWops *rwops = SDL_AllocRW();
  
  rwops->seek = physfs_seek;
  rwops->read = physfs_read;
  rwops->write = physfs_write;
  rwops->close = physfs_close;
  rwops->hidden.unknown.data1 = (void *) pfile;
  
  return rwops;
}

