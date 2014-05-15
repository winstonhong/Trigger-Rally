
// pengine.h [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#ifndef PENGINE_H_INCLUDED
#define PENGINE_H_INCLUDED


#include "../../config.h"

#include <stdlib.h>

#include <iostream>

#include <vector>
#include <list>
#include <string>
#include <utility>


// "SDL.h" is supposed to be the portable way, but it
// doesn't seem to work in some circumstances...
#include "SDL.h"
//#include <SDL/SDL.h>


// The PhysicsFS game file system
#include <physfs.h>


// This is a slightly modified GLEW for internal build
#define GLEW_STATIC
#include "GL/glew.h"


// TinyXML is built into the pengine library
#include "tinyxml.h"


// Some maths utils and macros
#include "vmath.h"


#define null 0


class PUtil;

class PApp;
class PSubsystem;
class   PSSRender;
class     PParticleType;
struct    PParticle_s;
class   PSSTexture;
class     PImage;
class     PTexture;
class   PSSAudio;
class     PAudioSample;
class     PAudioInstance;
class   PSSEffect;
class     PEffect;
class   PSSModel;
class     PFace;
class     PMesh;
class     PModel;
class   PTerrain;

class PException;
class   PUserException;
class   PFileException;
class   PParseException;


#include "exception.h"


// Utility

#define DEBUGLEVEL_CRITICAL     0
#define DEBUGLEVEL_ENDUSER      10
#define DEBUGLEVEL_TEST         20
#define DEBUGLEVEL_DEVELOPER    30


class PUtil {
private:
  PUtil() { } // cannot be constructed
  
  static int deblev;
  
public:
  // Output streams
  static std::ostream &outLog() { return std::cout; }
  
  // Debug level
  static bool isDebugLevel(int debugLevel) { return deblev >= debugLevel; }
  static void setDebugLevel(int debugLevel) { deblev = debugLevel; }
 
  /*! Get token and value from a string line. The token is the string
   * before first space. The value, is the remaining string
   * \return true if could extract token and value */
  static bool getToken(std::string line, std::string& tok, std::string& value);

  static char* fgets2(char *s, int size, PHYSFS_file *pfile);
  
  // Given "data/blah/pic.jpg" will return "data/blah/"
  static std::string extractPathFromFilename(const std::string &filename);
  
  static std::string assemblePath(const std::string &relativefile, const std::string &parentfile);
  
  // Load XML file and return the root element of given name (failure: null)
  static TiXmlElement *loadRootElement(TiXmlDocument &doc, const char *rootName);
  
  static bool copyFile(const std::string &fileFrom, const std::string &fileTo);
  static std::list<std::string> findFiles(const std::string &basedir, const std::string &extension);
  
  static std::string formatInt(int value, int width);
  static std::string formatInt(int value);
  
  static std::string formatTime(float seconds);
  
  // RWops created must be freed by using SDL freesrc on load
  static SDL_RWops *allocPhysFSops(PHYSFS_file *pfile);
};



#include "app.h"
#include "subsys.h"
#include "audio.h"
#include "render.h"


#endif // PENGINE_H_INCLUDED


