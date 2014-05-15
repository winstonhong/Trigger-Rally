
// texture.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"

#include <SDL/SDL_image.h>



// SDL_image doesn't need init/shutdown code

PSSTexture::PSSTexture(PApp &parentApp) : PSubsystem(parentApp)
{
  app.outLog() << "Initialising texture subsystem [SDL_Image]" << std::endl;
}

PSSTexture::~PSSTexture()
{
  app.outLog() << "Shutting down texture subsystem" << std::endl;
  
  texlist.clear();
}



PTexture *PSSTexture::loadTexture(const std::string &name, bool genMipmaps, bool clamp)
{
  PTexture *tex = texlist.find(name);
  if (!tex) {
    try
    {
      tex = new PTexture(name,genMipmaps,clamp);
    }
    catch (PException e)
    {
      if (PUtil::isDebugLevel(DEBUGLEVEL_ENDUSER))
        PUtil::outLog() << "Failed to load " << name << ": " << e.what () << std::endl;
      return null;
    }
    texlist.add(tex);
  }
  return tex;
}



PImage::~PImage()
{
  unload ();
}

void PImage::unload ()
{
  delete[] data;
  data = null;
}

void PImage::load (const std::string &filename)
{
  data = null;
  
  if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
    PUtil::outLog() << "Loading image \"" << filename << "\"" << std::endl;

  // PhysFS / SDL integration with SDL_rwops
  
  PHYSFS_file *pfile = PHYSFS_openRead(filename.c_str());
  
  if (pfile == null) {
    throw MakePException (filename + ", PhysFS: " + PHYSFS_getLastError ());
  }
  
  SDL_RWops *rwops = PUtil::allocPhysFSops(pfile);
  
  SDL_Surface *img = IMG_Load_RW(rwops, 1); // this closes file and frees rwops
  
  if (!img) {
    throw MakePException (filename + ", SDL_image: " + IMG_GetError ());
  }
  
  if (SDL_MUSTLOCK(img)) SDL_LockSurface(img);
  
  // TGA COLOUR SWITCH HACK
  int colmap_normal[] = { 0,1,2,3 };
  int colmap_flipped[] = { 2,1,0,3 };
  int *colmap = colmap_normal;
  const char *fname = filename.c_str();
  int len = strlen(fname);
  if (len > 4) {
    if (!strcmp(fname+len-4,".tga")) colmap = colmap_flipped;
  }
  
  cx = img->w;
  cy = img->h;
  cc = img->format->BytesPerPixel;
  data = new uint8 [cx * cy * cc];
  
  for (int y=0; y<cy; y++) {
    for (int x=0; x<cx; x++) {
      for (int c=0; c<cc; c++) {
        //data[(y*cx+x)*cc+c] = ((uint8*)img->pixels)[(cy-y-1)*img->pitch + x*cc + c];
        data[(y*cx+x)*cc+c] = ((uint8*)img->pixels)[(cy-y-1)*img->pitch + x*cc + colmap[c]];
      }
    }
  }
  
  if (SDL_MUSTLOCK(img)) SDL_UnlockSurface(img);
  SDL_FreeSurface(img);
}

void PImage::load (int _cx, int _cy, int _cc)
{
  cx = _cx;
  cy = _cy;
  cc = _cc;
  
  data = new uint8 [cx * cy * cc];
}



void PTexture::unload()
{
  if (texid)
  {
    glDeleteTextures (1, &texid);
    texid = 0;
  }
}

void PTexture::load (const std::string &filename, bool genMipmaps, bool clamp)
{
  PImage image (filename);
  load (image, genMipmaps, clamp);
  name = filename;
}

void PTexture::load (PImage &img, bool genMipmaps, bool clamp)
{
  unload();

  textarget = GL_TEXTURE_2D;

#ifdef USE_GEN_MIPMAPS
  if (genMipmaps && !extgl_Extensions.SGIS_generate_mipmap) {
    PUtil::outLog() << "warning: can't generate mipmaps for texture" << std::endl;
    genMipmaps = false;
  }
#endif

  GLuint fmt,fmt2;

  switch (img.getcc()) {
  case 1:
    fmt = GL_LUMINANCE; fmt2 = GL_LUMINANCE8; break;
  case 2:
    fmt = GL_LUMINANCE_ALPHA; fmt2 = GL_LUMINANCE8_ALPHA8; break;
  case 3:
    fmt = GL_RGB; fmt2 = GL_RGB8; break;
  case 4:
    fmt = GL_RGBA; fmt2 = GL_RGBA8; break;
  default:
    throw MakePException ("loading texture failed, unknown image format");
  }

  int cx = img.getcx(), cy = img.getcy();
  int newcx=1, newcy=1, max;
  while (newcx < cx) newcx *= 2;
  while (newcy < cy) newcy *= 2;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max);
  if (newcx > max) newcx = max;
  if (newcy > max) newcy = max;
  
  //PImage *useimg = &img;
  
  if (newcx != cx || newcy != cy) {
    PImage newimage (newcx, newcy, img.getcc ());
    
    gluScaleImage (fmt,
        cx, cy, GL_UNSIGNED_BYTE, img.getData (),
        newcx, newcy, GL_UNSIGNED_BYTE, newimage.getData ());
    
    img.swap (newimage);
  }

  glGenTextures(1,&texid);
  bind();

  glTexParameteri(textarget,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  if (genMipmaps)
    glTexParameteri(textarget,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  else
    glTexParameteri(textarget,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

  if (clamp) {
    glTexParameteri(textarget,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(textarget,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  } else {
    glTexParameteri(textarget,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(textarget,GL_TEXTURE_WRAP_T,GL_REPEAT);
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  if (genMipmaps) {
#ifdef USE_GEN_MIPMAPS

    glTexParameteri(textarget, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

    glTexImage2D(GL_TEXTURE_2D,0,fmt2,
      newcx,newcy,
      0,fmt,GL_UNSIGNED_BYTE,img.getData());

#else

    int level = 0;
    uint8 *imd = img.getData();
    int cc = img.getcc();
    while (1) {
      glTexImage2D(GL_TEXTURE_2D,level,fmt2,
        newcx,newcy,
        0,fmt,GL_UNSIGNED_BYTE,imd);

      if (newcx <= 1 && newcy <= 1) break;

      if (newcx > 1) newcx /= 2;
      if (newcy > 1) newcy /= 2;
      level++;

      for (int y=0; y<newcy; y++) {
        for (int x=0; x<newcx; x++) {
          for (int z=0; z<cc; z++) {
            imd[(y*newcx+x)*cc+z] = (uint8)(((int)
              imd[(y*newcx*4+x*2+0)*cc+z] +
              imd[(y*newcx*4+x*2+1)*cc+z] +
              imd[(y*newcx*4+x*2+0+newcx*2)*cc+z] +
              imd[(y*newcx*4+x*2+1+newcx*2)*cc+z]) / 4);
          }
        }
      }
    }

#endif
  } else {
    glTexImage2D(GL_TEXTURE_2D,0,fmt2,
      newcx,newcy,
      0,fmt,GL_UNSIGNED_BYTE,img.getData());
  }
}

void PTexture::loadPiece(PImage &img, int offx, int offy, int sizex, int sizey, bool genMipmaps, bool clamp)
{
  unload();

  textarget = GL_TEXTURE_2D;

#ifdef USE_GEN_MIPMAPS
  if (genMipmaps && !extgl_Extensions.SGIS_generate_mipmap) {
    PUtil::outLog() << "warning: can't generate mipmaps for texture" << std::endl;
    genMipmaps = false;
  }
#else
  genMipmaps = false;
#endif

  GLuint fmt,fmt2;

  switch (img.getcc()) {
  case 1:
    fmt = GL_LUMINANCE; fmt2 = GL_LUMINANCE8; break;
  case 2:
    fmt = GL_LUMINANCE_ALPHA; fmt2 = GL_LUMINANCE8_ALPHA8; break;
  case 3:
    fmt = GL_RGB; fmt2 = GL_RGB8; break;
  case 4:
    fmt = GL_RGBA; fmt2 = GL_RGBA8; break;
  default:
    throw MakePException ("loading texture failed, unknown image format");
  }

  int cx = sizex, cy = sizey;
  int newcx=1, newcy=1, max;
  while (newcx < cx) newcx *= 2;
  while (newcy < cy) newcy *= 2;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max);
  if (newcx > max) newcx = max;
  if (newcy > max) newcy = max;

  if (newcx != cx || newcy != cy) {
    PImage newimage (newcx, newcy, img.getcc ());
    
    gluScaleImage (fmt,
        cx, cy, GL_UNSIGNED_BYTE, img.getData (),
        newcx, newcy, GL_UNSIGNED_BYTE, newimage.getData ());
    
    img.swap (newimage);
  }

  glGenTextures(1,&texid);
  bind();

  glTexParameteri(textarget,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  if (genMipmaps)
    glTexParameteri(textarget,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  else
    glTexParameteri(textarget,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

  if (clamp) {
    glTexParameteri(textarget,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(textarget,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  } else {
    glTexParameteri(textarget,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(textarget,GL_TEXTURE_WRAP_T,GL_REPEAT);
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glPixelStorei(GL_UNPACK_ROW_LENGTH, img.getcx());
  glPixelStorei(GL_UNPACK_SKIP_ROWS, offy);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, offx);

  //uint8 *offsetdata = img.getData() + ((offy*img.getcx())+offx)*img.getcc();

  if (genMipmaps)
    glTexParameteri(textarget, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

  glTexImage2D(GL_TEXTURE_2D,0,fmt2,
    newcx,newcy,
    0,fmt,GL_UNSIGNED_BYTE,img.getData());

  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
}

void PTexture::loadAlpha(const std::string &filename, bool genMipmaps, bool clamp)
{
  PImage image (filename);
  loadAlpha (image, genMipmaps, clamp);
  name = filename;
}

void PTexture::loadAlpha(PImage &img, bool genMipmaps, bool clamp)
{
  unload();

  textarget = GL_TEXTURE_2D;

#ifdef USE_GEN_MIPMAPS
  if (genMipmaps && !extgl_Extensions.SGIS_generate_mipmap) {
    PUtil::outLog() << "warning: can't generate mipmaps for texture" << std::endl;
    genMipmaps = false;
  }
#endif

  GLuint fmt,fmt2;

  switch (img.getcc()) {
  case 1:
    fmt = GL_ALPHA; fmt2 = GL_ALPHA8; break;
  case 2:
    fmt = GL_LUMINANCE_ALPHA; fmt2 = GL_LUMINANCE8_ALPHA8;
    PUtil::outLog() << "Warning: loadAlpha() has been used for image with 2 channels" << std::endl;
    break;
  case 3:
    fmt = GL_RGB; fmt2 = GL_RGB8;
    PUtil::outLog() << "Warning: loadAlpha() has been used for RGB image" << std::endl;
    break;
  case 4:
    fmt = GL_RGBA; fmt2 = GL_RGBA8;
    PUtil::outLog() << "Warning: loadAlpha() has been used for RGBA image" << std::endl;
    break;
  default:
    throw MakePException ("loading texture failed, unknown image format");
  }

  int cx = img.getcx(), cy = img.getcy();
  int newcx=1, newcy=1, max;
  while (newcx < cx) newcx *= 2;
  while (newcy < cy) newcy *= 2;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max);
  if (newcx > max) newcx = max;
  if (newcy > max) newcy = max;

  if (newcx != cx || newcy != cy) {
    PImage newimage (newcx, newcy, img.getcc ());
    
    gluScaleImage (fmt,
        cx, cy, GL_UNSIGNED_BYTE, img.getData (),
        newcx, newcy, GL_UNSIGNED_BYTE, newimage.getData ());
    
    img.swap (newimage);
  }

  glGenTextures(1,&texid);
  bind();

  glTexParameteri(textarget,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  if (genMipmaps)
    glTexParameteri(textarget,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  else
    glTexParameteri(textarget,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

  if (clamp) {
    glTexParameteri(textarget,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(textarget,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  } else {
    glTexParameteri(textarget,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(textarget,GL_TEXTURE_WRAP_T,GL_REPEAT);
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  if (genMipmaps) {
#ifdef USE_GEN_MIPMAPS

    glTexParameteri(textarget, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

    glTexImage2D(GL_TEXTURE_2D,0,fmt2,
      newcx,newcy,
      0,fmt,GL_UNSIGNED_BYTE,img.getData());

#else

    int level = 0;
    uint8 *imd = img.getData();
    int cc = img.getcc();
    while (1) {
      glTexImage2D(GL_TEXTURE_2D,level,fmt2,
        newcx,newcy,
        0,fmt,GL_UNSIGNED_BYTE,imd);

      if (newcx <= 1 && newcy <= 1) break;

      if (newcx > 1) newcx /= 2;
      if (newcy > 1) newcy /= 2;
      level++;

      for (int y=0; y<newcy; y++) {
        for (int x=0; x<newcx; x++) {
          for (int z=0; z<cc; z++) {
            imd[(y*newcx+x)*cc+z] = (uint8)(((int)
              imd[(y*newcx*4+x*2+0)*cc+z] +
              imd[(y*newcx*4+x*2+1)*cc+z] +
              imd[(y*newcx*4+x*2+0+newcx*2)*cc+z] +
              imd[(y*newcx*4+x*2+1+newcx*2)*cc+z]) / 4);
          }
        }
      }
    }

#endif
  } else {
    glTexImage2D(GL_TEXTURE_2D,0,fmt2,
      newcx,newcy,
      0,fmt,GL_UNSIGNED_BYTE,img.getData());
  }
}

void PTexture::loadCubeMap(const std::string &filenamePrefix, const std::string &filenameSuffix, bool genMipmaps)
{
  PUtil::outLog() << "loading \"" << filenamePrefix << "*" << filenameSuffix << "\" (cube map)" << std::endl;
  unload();

  textarget = GL_TEXTURE_CUBE_MAP;

#ifdef USE_GEN_MIPMAPS
  if (genMipmaps && !extgl_Extensions.SGIS_generate_mipmap) {
    PUtil::outLog() << "warning: can't generate mipmaps for texture" << std::endl;
    genMipmaps = false;
  }
#endif

  PImage img;

  glGenTextures(1,&texid);
  bind();

#ifdef USE_GEN_MIPMAPS
  GLenum sidetarget[6] = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
  };
#endif

  const char *middlename[6] = { "px", "nx", "py", "ny", "pz", "nz" };

  for (int side=0; side<6; side++) {
    std::string filename = filenamePrefix + middlename[side] + filenameSuffix;

    img.load(filename);

    GLuint fmt,fmt2;

    switch (img.getcc()) {
    case 1:
      fmt = GL_LUMINANCE; fmt2 = GL_LUMINANCE8; break;
    case 2:
      fmt = GL_LUMINANCE_ALPHA; fmt2 = GL_LUMINANCE8_ALPHA8; break;
    case 3:
      fmt = GL_RGB; fmt2 = GL_RGB8; break;
    case 4:
      fmt = GL_RGBA; fmt2 = GL_RGBA8; break;
    default:
      throw MakePException (filename + " load failed, unknown image format");
    }

    int cx = img.getcx(), cy = img.getcy();
    int newcx=1, newcy=1, max;
    while (newcx < cx) newcx *= 2;
    while (newcy < cy) newcy *= 2;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max);
    if (newcx > max) newcx = max;
    if (newcy > max) newcy = max;

    if (newcx != cx || newcy != cy) {
      PImage newimage (newcx, newcy, img.getcc ());
      
      gluScaleImage (fmt,
          cx, cy, GL_UNSIGNED_BYTE, img.getData (),
          newcx, newcy, GL_UNSIGNED_BYTE, newimage.getData ());
      
      img.swap (newimage);
    }

    glTexParameteri(textarget,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    if (genMipmaps)
      glTexParameteri(textarget,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    else
      glTexParameteri(textarget,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glTexParameteri(textarget,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(textarget,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    if (genMipmaps) {
#ifdef USE_GEN_MIPMAPS

      glTexParameteri(textarget, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

      glTexImage2D(sidetarget[side],0,fmt2,
        newcx,newcy,
        0,fmt,GL_UNSIGNED_BYTE,img.getData());

#else

      int level = 0;
      uint8 *imd = img.getData();
      int cc = img.getcc();
      while (1) {
        glTexImage2D(GL_TEXTURE_2D,level,fmt2,
          newcx,newcy,
          0,fmt,GL_UNSIGNED_BYTE,imd);
        //break;
        if (newcx <= 1 && newcy <= 1) break;

        if (newcx > 1) newcx /= 2;
        if (newcy > 1) newcy /= 2;
        level++;

        for (int y=0; y<newcy; y++) {
          for (int x=0; x<newcx; x++) {
            for (int z=0; z<cc; z++) {
              imd[(y*newcx+x)*cc+z] = (uint8)(((int)
                imd[(y*newcx*4+x*2+0)*cc+z] +
                imd[(y*newcx*4+x*2+1)*cc+z] +
                imd[(y*newcx*4+x*2+0+newcx*2)*cc+z] +
                imd[(y*newcx*4+x*2+1+newcx*2)*cc+z]) / 4);
            }
          }
        }
      }

#endif
    } else {
      glTexImage2D(GL_TEXTURE_2D,0,fmt2,
        newcx,newcy,
        0,fmt,GL_UNSIGNED_BYTE,img.getData());
    }

    img.unload();
  }

  name = filenamePrefix;
}

void PTexture::bind() const
{
  glBindTexture(textarget, texid);
}

// static
void PTexture::unbind()
{
  // clear both used texture targets
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}



