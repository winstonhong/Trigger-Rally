
// render.h [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)



#include "vbuffer.h"



struct PParticle_s {
  vec3f pos,linvel;
  float life;

  vec2f orix,oriy; // orientation vectors (2d)
};

class PParticleSystem {
protected:
  float colorstart[4],colorend[4];
  float startsize, endsize;
  float decay;
  const PTexture *tex;
  GLenum blendparam1, blendparam2;

  std::vector<PParticle_s> part;

public:
  PParticleSystem() {
    colorstart[0] = colorstart[1] = colorstart[2] = colorstart[3] = 1.0;
    colorend[0] = colorend[1] = colorend[2] = 1.0; colorend[3] = 0.0;
    startsize = 0.0;
    endsize = 1.0;
    decay = 1.0;
    tex = null;
    blendparam1 = GL_SRC_ALPHA;
    blendparam2 = GL_ONE;
  }

public:
  void setColorStart(float r, float g, float b, float a) {
    colorstart[0] = r; colorstart[1] = g; colorstart[2] = b; colorstart[3] = a;
  }
  void setColorEnd(float r, float g, float b, float a) {
    colorend[0] = r; colorend[1] = g; colorend[2] = b; colorend[3] = a;
  }
  void setColor(float r, float g, float b) {
    colorstart[0] = colorend[0] = r;
    colorstart[1] = colorend[1] = g;
    colorstart[2] = colorend[2] = b;
  }
  void setSize(float start, float end) {
    startsize = start; endsize = end;
  }
  void setDecay(float _decay) {
    decay = _decay;
  }
  void setTexture(const PTexture *texptr) {
    tex = texptr;
  }
  void setBlend(GLenum b1, GLenum b2) {
    blendparam1 = b1;
    blendparam2 = b2;
  }

  void addParticle(const vec3f &pos, const vec3f &linvel);

  void tick(float delta);

  friend class PSSRender;
};

struct PVert_tv {
  vec2f st;
  vec3f xyz;
};

#define PTEXT_HZA_LEFT    0x00000000 // default
#define PTEXT_HZA_CENTER  0x00000001
#define PTEXT_HZA_RIGHT   0x00000002
#define PTEXT_VTA_BOTTOM  0x00000000 // default
#define PTEXT_VTA_CENTER  0x00000100
#define PTEXT_VTA_TOP     0x00000200

class PSSRender : public PSubsystem {
private:
  vec3f cam_pos;
  mat44f cam_orimat;

public:
  PSSRender(PApp &parentApp);
  ~PSSRender();
  
  void tick(float delta, const vec3f &eyepos, const mat44f &eyeori, const vec3f &eyevel);
  
  void render(PParticleSystem *psys);
  
  void drawModel(PModel &model, PSSEffect &ssEffect, PSSTexture &ssTexture);
  
  void drawText(const std::string &text, uint32 flags);
  vec2f getTextDims(const std::string &text);
};



class PSSTexture : public PSubsystem {
private:
  PResourceList<PTexture> texlist;

public:
  PSSTexture(PApp &parentApp);
  ~PSSTexture();

  PTexture *loadTexture(const std::string &name, bool genMipmaps = true, bool clamp = false);
};


class PImage {
private:
  uint8 *data;
  int cx,cy,cc;

public:
  PImage () : data (null) { }
  PImage (const std::string &filename) : data (null) { load (filename); }
  PImage (int _cx, int _cy, int _cc) : data (null) { load (_cx, _cy, _cc); }
  ~PImage ();
  
  void load (const std::string &filename);
  void load (int _cx, int _cy, int _cc);
  void unload ();
  
  void expandChannels();

  int getcx() const { return cx; }
  int getcy() const { return cy; }
  int getcc() const { return cc; }
  uint8 *getData() { return data; }
  
  void swap (PImage &other) throw ()
  {
    { uint8 *tmp = data; data = other.data; other.data = tmp; }
    { int tmp = cx; cx = other.cx; other.cx = tmp; }
    { int tmp = cy; cy = other.cy; other.cy = tmp; }
    { int tmp = cc; cc = other.cc; other.cc = tmp; }
  }
};


class PTexture : public PResource {
private:
  GLuint texid;
  GLenum textarget;
  
public:
  PTexture () : texid (0) { }
  PTexture (const std::string &filename, bool genMipmaps, bool clamp) : texid (0) { load (filename, genMipmaps, clamp); }
  PTexture (PImage &img, bool genMipmaps, bool clamp) : texid (0) { load (img, genMipmaps, clamp); }
  ~PTexture() { unload (); }
  
  void load (const std::string &filename, bool genMipmaps, bool clamp);
  void load(PImage &img, bool genMipmaps = true, bool clamp = false);
  void loadPiece(PImage &img, int offx, int offy, int sizex, int sizey, bool genMipmaps = true, bool clamp = false);
  void loadAlpha(const std::string &filename, bool genMipmaps = true, bool clamp = false);
  void loadAlpha(PImage &img, bool genMipmaps = true, bool clamp = false);
  void loadCubeMap(const std::string &filenamePrefix, const std::string &filenameSuffix, bool genMipmaps = true);
  void unload();

  void bind() const;

  static void unbind();
};




class PSSEffect : public PSubsystem {
private:
  PResourceList<PEffect> fxlist;

public:
  PSSEffect(PApp &parentApp);
  ~PSSEffect();

  PEffect *loadEffect(const std::string &name);
};



#define CULLFACE_NONE       0
#define CULLFACE_CW         1
#define CULLFACE_CCW        2

#define BLEND_NONE          0
#define BLEND_ADD           1
#define BLEND_MULTIPLY      2
#define BLEND_ALPHA         3
#define BLEND_PREMULTALPHA  4


struct fx_renderstate_s {
  bool depthtest;

  bool lighting;
  bool lightmodeltwoside;

  struct {
    GLenum func;
    float ref;
  } alphatest;

  int cullface;

  int blendmode;

  struct {
    int texindex;
  } texunit[1];
};


struct fx_pass_s {
  fx_renderstate_s rs;
};

struct fx_technique_s {
  std::string name;

  std::vector<fx_pass_s> pass;

  bool validated;
  bool textures_ready;

  fx_technique_s() {
    validated = false;
    textures_ready = false;
  }
};

struct fx_texture_s {
  std::string name;

  std::string filename;

  GLenum type;

  // This is filled in just before rendering
  PTexture *texobject;

  fx_texture_s() {
    texobject = null;
  }
};


class PEffect : public PResource {
private:
  // resources
  std::vector<fx_texture_s> tex;

  // techniques
  std::vector<fx_technique_s> tech;

  int cur_tech;

public:
  PEffect(const std::string &filename);
  ~PEffect();

  void unload();

  void loadFX(const std::string &filename);
  void loadMTL(const std::string &filename);

  int getNumTechniques();
  bool validateTechnique(int technique);
  const std::string &getTechniqueName(int technique);
  bool findTechnique(const std::string &techname, int *technique);

  bool setCurrentTechnique(int technique);
  int getCurrentTechnique();

  bool setFirstValidTechnique();

  bool renderBegin(int *numPasses, PSSTexture &sstex);
  void renderPass(int pass);
  void renderEnd();

private:
  void migrateRenderState(fx_renderstate_s *rs_old, fx_renderstate_s *rs_new);
};




class PSSModel : public PSubsystem {
private:
  PResourceList<PModel> modlist;

public:
  PSSModel(PApp &parentApp);
  ~PSSModel();

  PModel *loadModel(const std::string &name);
};


class PFace {
public:
  vec3f facenormal;
  uint32 vt[3];
  uint32 tc[3];
  uint32 nr[3];

public:
//    PFace(vec
};


class PMesh {
public:
  std::vector<vec3f> vert;
  std::vector<vec2f> texco;
  std::vector<vec3f> norm;
  std::vector<PFace> face;

  std::string fxname;
  PEffect *effect;
};


class PModel : public PResource {
public:
  std::vector<PMesh> mesh;

  std::pair<vec3f, vec3f> getExtents() const;

public:
  PModel (const std::string &filename, float globalScale = 1.0);
  
private:
  void loadASE (const std::string &filename, float globalScale);
  void loadOBJ (const std::string &filename, float globalScale);
};

struct PTerrainFoliageBand {
  float middle, range;
  float density;
  int trycount;
  
  PTexture *sprite_tex;
  int sprite_count;
};

struct PTerrainFoliage {
  vec3f pos;
  float ang;
  float scale;
};

struct PTerrainFoliageSet {
  std::vector<PTerrainFoliage> inst;
  
  PVBuffer buff[2];
  int numvert, numelem;
};

struct PTerrainTile {
  int posx, posy;
  int lru_counter;
  
  PVBuffer vert;
  int numverts;
  
  PTexture tex;
  
  vec3f mins,maxs; // AABB
  
  //
  
  std::vector<PTerrainFoliageSet> foliage;
};


class PTerrain // TODO: make this RAII conformant
{
protected:
  bool loaded;
  
  int tilesize, tilecount, totsize, totmask, totsizesq;
  
  float scale_hz, scale_vt, scale_hz_inv, scale_vt_inv, scale_tile_inv;
  
  int cmaptotsize, cmaptilesize, cmaptotmask;
  
  //std::vector<uint8> hmap;
  std::vector<float> hmap;
  
  PImage cmap;
  
  std::vector<float> fmap;
  std::vector<PTerrainFoliageBand> foliageband;
  
  std::list<PTerrainTile> tile;
  
  // tiles share index buffers
  PVBuffer ind;
  int numinds;
  
  PTexture *tex_hud_map;
  
protected:
  
  PTerrainTile *getTile(int x, int y);
  
  float getInterp(float x, float y, float *data) {
    x *= scale_hz_inv;
    int xi = (int)x;
    if (x < 0.0) xi--;
    x -= (float)xi;
    int xiw = xi & totmask, xiw2 = (xiw+1) & totmask;
    
    y *= scale_hz_inv;
    int yi = (int)y;
    if (y < 0.0) yi--;
    y -= (float)yi;
    int yiw = yi & totmask, yiw2 = (yiw+1) & totmask;
    
    const int cx = totsize;
    
    float xv1,xv2;
    if (y > 0.0) {
      if (y < 1.0) {
        if (x < y) {
          xv1 = data[yiw*cx+xiw];
          xv2 = INTERP(data[yiw2*cx+xiw],data[yiw2*cx+xiw2],x/y);
        } else {
          xv1 = INTERP(data[yiw*cx+xiw],data[yiw*cx+xiw2],(x-y)/(1.0-y));
          xv2 = data[yiw2*cx+xiw2];
        }
        return INTERP(xv1,xv2,y);
      } else {
        return INTERP(data[yiw2*cx+xiw],data[yiw2*cx+xiw2],x);
      }
    } else {
      return INTERP(data[yiw*cx+xiw],data[yiw*cx+xiw2],x);
    }
  }
  
public:
  PTerrain(TiXmlElement *element, const std::string &filepath, PSSTexture &ssTexture);
  ~PTerrain();
  
  void unload();
  
  void render(const vec3f &campos, const mat44f &camorim);
  
  void drawSplat(float x, float y, float scale, float angle);
  
  
  struct ContactInfo {
    vec3f pos;
    vec3f normal;
  };
  
  void getContactInfo(ContactInfo &tci) {
    float x = tci.pos.x * scale_hz_inv;
    int xi = (int)x;
    if (x < 0.0) xi--;
    x -= (float)xi;
    int xiw = xi & totmask, xiw2 = (xi+1) & totmask;
    
    float y = tci.pos.y * scale_hz_inv;
    int yi = (int)y;
    if (y < 0.0) yi--;
    y -= (float)yi;
    int yiw = yi & totmask, yiw2 = (yi+1) & totmask;
    
    float *data = &hmap[0];
    const int cx = totsize;
    
    float xv1,xv2;
    if (y > 0.0) {
      if (y < 1.0) {
        if (x < y) {
          tci.normal.x = data[yiw2*cx+xiw] - data[yiw2*cx+xiw2];
          tci.normal.y = data[yiw*cx+xiw] - data[yiw2*cx+xiw];
          xv1 = data[yiw*cx+xiw];
          xv2 = INTERP(data[yiw2*cx+xiw],data[yiw2*cx+xiw2],x/y);
        } else {
          tci.normal.x = data[yiw*cx+xiw] - data[yiw*cx+xiw2];
          tci.normal.y = data[yiw*cx+xiw2] - data[yiw2*cx+xiw2];
          xv1 = INTERP(data[yiw*cx+xiw],data[yiw*cx+xiw2],(x-y)/(1.0-y));
          xv2 = data[yiw2*cx+xiw2];
        }
        tci.pos.z = INTERP(xv1,xv2,y);
      } else {
        tci.normal.x = data[yiw2*cx+xiw] - data[yiw2*cx+xiw2];
        tci.normal.y = data[yiw*cx+xiw] - data[yiw2*cx+xiw];
        tci.pos.z = INTERP(data[yiw2*cx+xiw],data[yiw2*cx+xiw2],x);
      }
    } else {
      tci.normal.x = data[yiw*cx+xiw] - data[yiw*cx+xiw2];
      tci.normal.y = data[yiw*cx+xiw2] - data[yiw2*cx+xiw2];
      tci.pos.z = INTERP(data[yiw*cx+xiw],data[yiw*cx+xiw2],x);
    }
    tci.normal.z = scale_hz;
    tci.normal.normalize();
  }
  
  float getHeight(float x, float y) {
    return getInterp(x, y, &hmap[0]);
  }
  
  float getFoliageLevel(float x, float y) {
    return getInterp(x, y, &fmap[0]);
  }
  
  PTexture *getHUDMapTexture() { return tex_hud_map; }
  
  float getMapSize() { return totsize * scale_hz; }
};




