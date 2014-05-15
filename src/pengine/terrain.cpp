
// terrain.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"



PTerrain::~PTerrain ()
{
  unload();
}


void PTerrain::unload()
{
  loaded = false;

  tile.clear();

  hmap.clear();
}


PTerrain::PTerrain (TiXmlElement *element, const std::string &filepath, PSSTexture &ssTexture) :
  loaded (false)
{
  unload();
  
  std::string heightmap, colormap, foliagemap, hudmap;
  
  scale_hz = 1.0;
  scale_vt = 1.0;
  
  const char *val;
  
  val = element->Attribute("tilesize");
  if (val) tilesize = atoi(val);
  
  val = element->Attribute("horizontalscale");
  if (val) scale_hz = atof(val);
  
  val = element->Attribute("verticalscale");
  if (val) scale_vt = atof(val);
  
  val = element->Attribute("heightmap");
  if (val) heightmap = val;
  
  val = element->Attribute("colormap");
  if (val) colormap = val;
  
  val = element->Attribute("foliagemap");
  if (val) foliagemap = val;
  
  val = element->Attribute("hudmap");
  if (val) hudmap = val;
  
  for (TiXmlElement *walk = element->FirstChildElement();
    walk; walk = walk->NextSiblingElement()) {
    
    if (!strcmp(walk->Value(), "foliageband")) {
      
      PTerrainFoliageBand tfb;
      tfb.middle = 0.5f;
      tfb.range = 0.5f;
      tfb.density = 1.0f;
      //tfb.model = null;
      //tfb.modelscale = 1.0f;
      tfb.sprite_tex = null;
      tfb.sprite_count = 1;
      
      val = walk->Attribute("middle");
      if (val) tfb.middle = atof(val);
      
      val = walk->Attribute("range");
      if (val) tfb.range = atof(val);
      
      val = walk->Attribute("density");
      if (val) tfb.density = atof(val);
      
      /*
      val = walk->Attribute("model");
      if (val) tfb.model = ssModel.loadModel(PUtil::assemblePath(val, filepath));
      
      val = walk->Attribute("modelscale");
      if (val) tfb.modelscale = atof(val);
      */
      
      val = walk->Attribute("sprite");
      if (val) tfb.sprite_tex = ssTexture.loadTexture(PUtil::assemblePath(val, filepath));
      
      val = walk->Attribute("spritecount");
      if (val) tfb.sprite_count = atoi(val);
      
      foliageband.push_back(tfb);
    }
  }
  
  
  if (!heightmap.length()) {
    throw MakePException ("Load failed: terrain has no heightmap");
  }
  
  if (!colormap.length()) {
    throw MakePException ("Load failed: terrain has no colormap");
  }
  
  if (tilesize != (tilesize & (-tilesize)) ||
    tilesize < 4) {
    throw MakePException ("Load failed: tile size not power of two dimension, or too small");
  }
  
  if (scale_hz <= 0.0 || scale_vt == 0.0) {
    throw MakePException ("Load failed: invalid scale value");
  }
  
  scale_hz_inv = 1.0 / scale_hz;
  scale_vt_inv = 1.0 / scale_vt;
  scale_tile_inv = scale_hz_inv / (float)tilesize;
  
  PImage img;
  try
  {
    img.load (PUtil::assemblePath (heightmap, filepath));
  }
  catch (...)
  {
    PUtil::outLog() << "Load failed: couldn't open heightmap \"" << heightmap << "\"\n";
    throw;
  }
  
  totsize = img.getcx();
  if (totsize != img.getcy() ||
    totsize != (totsize & (-totsize)) ||
    totsize < 16) {
    throw MakePException ("Load failed: heightmap not square, or not power of two dimension, or too small");
  }
  
  totsizesq = totsize * totsize;
  
  if (tilesize > totsize) tilesize = totsize;
  
  tilecount = totsize / tilesize;
  totmask = totsize - 1;
  
  //PUtil::outLog() << "img: " << totsize << " squared, " << img.getcc() << " cc\n";
  
  hmap.resize(totsizesq);
  
#if 0
  if (img.getcc() != 1) {
    if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
      PUtil::outLog() << "Warning: heightmap is not single channel\n";
    int cc = img.getcc();
    uint8 *dat = img.getData();
    for (int s=0, d=0; d<totsizesq; s+=cc, d+=1) hmap[d] = dat[s];
  } else {
    std::copy(img.getData(), img.getData() + totsize * totsize, &hmap[0]);
  }
#else
  if (img.getcc() != 1) {
    if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
      PUtil::outLog() << "Warning: heightmap is not single channel\n";
  }
  
  int cc = img.getcc();
  uint8 *dat = img.getData();
  
  #define FILTERSIZE      3
  #define FILTEROFFSET    ((FILTERSIZE-1)/2)
  float blurfilter[FILTERSIZE][FILTERSIZE] = {
#if 1
    { 0.03, 0.12, 0.03 },
    { 0.12, 0.40, 0.12 },
    { 0.03, 0.12, 0.03 }
#else
    { 0.05, 0.15, 0.05 },
    { 0.15, 0.20, 0.15 },
    { 0.05, 0.15, 0.05 }
#endif
  };
  
  for (int y=0; y<totsize; ++y) {
    for (int x=0; x<totsize; ++x) {
      float accum = 0.0;
      for (int yi=0; yi<FILTERSIZE; ++yi) {
        for (int xi=0; xi<FILTERSIZE; ++xi) {
          accum += (float)dat[
            (((y + yi - FILTEROFFSET) & totmask) * totsize +
            ((x + xi - FILTEROFFSET) & totmask)) * cc] * blurfilter[yi][xi];
        }
      }
      hmap[y*totsize + x] = accum * scale_vt;
    }
  }
#endif
  
  img.unload();
  
  try
  {
    cmap.load(PUtil::assemblePath(colormap, filepath));
  }
  catch (...)
  {
    PUtil::outLog() << "Load failed: couldn't open colormap \"" << colormap << "\"\n";
    throw;
  }
  
  cmaptotsize = cmap.getcx();
  if (cmaptotsize != cmap.getcy() ||
    cmaptotsize != (cmaptotsize & (-cmaptotsize)) ||
    cmaptotsize < tilecount) {
    throw MakePException ("Load failed: colormap not square, or not power of two dimension, or too small");
  }
  
  cmaptilesize = cmaptotsize / tilecount;
  cmaptotmask = cmaptotsize - 1;
  
  // calculate foliage try counts for tile size
  
  for (unsigned int b = 0; b < foliageband.size(); b++) {
    foliageband[b].trycount =
      (int) (foliageband[b].density * (float)totsizesq * scale_hz * scale_hz);
  }
  
  // load foliage map
  
  fmap.resize(totsizesq, 0.0f);
  
  if (foliagemap.length()) {
    try
    {
      img.load(PUtil::assemblePath(foliagemap, filepath));
    }
    catch (...)
    {
      PUtil::outLog() << "Load failed: couldn't open foliage map \"" << heightmap << "\"\n";
      throw;
    }
    
    if (totsize != img.getcy() ||
      totsize != img.getcx()) {
      throw MakePException ("Load failed: foliage map size doesn't match heightmap");
    }
    
    int cc = img.getcc();
    uint8 *dat = img.getData();
    
    if (cc != 1) {
      if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
        PUtil::outLog() << "Warning: foliage map is not single channel\n";
    }
    
    for (int i = 0; i < totsizesq; i++) {
      fmap[i] = (float) dat[i * cc] / 255.0f;
    }
  }
  
  // load hud map
  
  tex_hud_map = null;
  
  if (hudmap.length()) {
    tex_hud_map = ssTexture.loadTexture(PUtil::assemblePath(hudmap, filepath));
  }
  
  // prepare shared index buffers
  
  int tilesizep1 = tilesize + 1;
  
  numinds = 0;
  
  PRamFile ramfile;
  uint16 index;
  for (int y=0; y<tilesize; ++y) {
    int add1 = (y+1) * tilesizep1;
    int add2 = (y+0) * tilesizep1;
    if (y > 0) {
      index = 0 + add1;
      ramfile.write(&index, sizeof(uint16));
      numinds += 1;
    }
    for (int x=0; x<tilesizep1; ++x) {
      index = x + add1;
      ramfile.write(&index, sizeof(uint16));
      index = x + add2;
      ramfile.write(&index, sizeof(uint16));
      numinds += 2;
    }
    if (y+1 < tilesize) {
      ramfile.write(&index, sizeof(uint16));
      numinds += 1;
    }
  }
  
  ind.create(ramfile.getSize(), PVBuffer::IndexContent, PVBuffer::StaticUsage, ramfile.getData());
  ramfile.clear();
  
  loaded = true;
}



PTerrainTile *PTerrain::getTile(int tilex, int tiley)
{
  // find the least recently used tile while searching for x,y
  int best_lru = 0, unused = 0;
  PTerrainTile *tileptr = null;
  for (std::list<PTerrainTile>::iterator iter = tile.begin();
    iter != tile.end(); ++iter) {
    if (iter->posx == tilex && iter->posy == tiley) {
      iter->lru_counter = 0;
      //PUtil::outLog() << "1: " << tilex << " " << tiley << std::endl;
      return &*iter;
    }

    if (best_lru < iter->lru_counter) {
      best_lru = iter->lru_counter;
      tileptr = &*iter;
    }

    if (iter->lru_counter > 1) ++unused;
  }

  // if there aren't enough unused tiles, create a new one

  if (unused < 10 || best_lru <= 1) {
    tile.push_back(PTerrainTile());
    tileptr = &tile.back();
  }

  tileptr->posx = tilex;
  tileptr->posy = tiley;
  tileptr->lru_counter = 0;

  tileptr->mins = vec3f((float)tilex * scale_hz, (float)tiley * scale_hz, 1000000000.0);
  tileptr->maxs = vec3f((float)(tilex+1) * scale_hz, (float)(tiley+1) * scale_hz, -1000000000.0);
  
  // TODO: quadtree based thing
  
  //std::vector<bool>
  
  static PRamFile ramfile1, ramfile2;
  
  ramfile1.clear();
  
  int tileoffsety = tiley * tilesize;
  int tileoffsetx = tilex * tilesize;
  int tilesizep1 = tilesize + 1;
  for (int y=0; y<tilesizep1; ++y) {
    int posy = tileoffsety + y;
    int sampley = posy & totmask;
    for (int x=0; x<tilesizep1; ++x) {
      int posx = tileoffsetx + x;
      int samplex = posx & totmask;
      vec3f vert = vec3f(
        (float)posx * scale_hz,
        (float)posy * scale_hz,
        (float)hmap[(sampley * totsize) + samplex]);
      ramfile1.write(vert, sizeof(vec3f));
      if (tileptr->mins.z > vert.z)
        tileptr->mins.z = vert.z;
      if (tileptr->maxs.z < vert.z)
        tileptr->maxs.z = vert.z;
    }
  }
  tileptr->vert.create(ramfile1.getSize(), PVBuffer::VertexContent, PVBuffer::StaticUsage, ramfile1.getData());

  //tileptr->maxs.z += 10.0;

  //tileptr->mins = vec3f((float)tilex * scale_hz, (float)tiley * scale_hz, 0.0);
  //tileptr->maxs = vec3f((float)(tilex+1) * scale_hz, (float)(tiley+1) * scale_hz, 100.0);

  tileptr->numverts = tilesizep1 * tilesizep1;
  
  tileptr->tex.loadPiece(cmap,
    (tilex * cmaptilesize) & cmaptotmask, (tiley * cmaptilesize) & cmaptotmask,
    cmaptilesize, cmaptilesize, true, true);
  
  // Create foliage
  
  srand(1);
  
  tileptr->foliage.resize(foliageband.size());
  
  for (unsigned int b = 0; b < foliageband.size(); b++) {
    
    tileptr->foliage[b].inst.clear();
    
    // Create foliage instances
    
    for (int i = 0; i < foliageband[b].trycount; i++) {
      
      vec2f ftry = vec2f(
        (float)((tileptr->posx * tilesize) + rand01 * tilesize) * scale_hz,
        (float)((tileptr->posy * tilesize) + rand01 * tilesize) * scale_hz);
        
      float fol = getFoliageLevel(ftry.x, ftry.y);
      
      if ((1.0 - fabs((fol - foliageband[b].middle) / foliageband[b].range)) < rand01) continue;
      
      tileptr->foliage[b].inst.push_back(PTerrainFoliage());
      tileptr->foliage[b].inst.back().pos.x = ftry.x;
      tileptr->foliage[b].inst.back().pos.y = ftry.y;
      tileptr->foliage[b].inst.back().pos.z = getHeight(ftry.x, ftry.y);
      tileptr->foliage[b].inst.back().ang = rand01 * PI*2.0f;
      tileptr->foliage[b].inst.back().scale = (1.0f + fol * 0.5f) * (rand01 * rand01 + 0.5) * 1.4;
    }
    
    // Create vertex buffers for rendering
    
#define HMULT   1.0
#define VMULT   2.0
    
    ramfile1.clear();
    ramfile2.clear();
    
    tileptr->foliage[b].numvert = 0;
    tileptr->foliage[b].numelem = 0;
    
    float angincr = PI / (float)foliageband[b].sprite_count;
    for (unsigned int j=0; j<tileptr->foliage[b].inst.size(); j++) {
      for (float anga = 0.0f; anga < PI - 0.01f; anga += angincr) {
        float interang = tileptr->foliage[b].inst[j].ang + anga;
        int stv = tileptr->foliage[b].numvert;
        PVert_tv tmpv;
        
        tmpv.xyz = tileptr->foliage[b].inst[j].pos +
          vec3f(cos(interang)*HMULT,sin(interang)*HMULT,0.0f) * tileptr->foliage[b].inst[j].scale;
        tmpv.st = vec2f(1.0f,0.0f);
        ramfile1.write(&tmpv,sizeof(PVert_tv));
        tileptr->foliage[b].numvert++;
        
        tmpv.xyz = tileptr->foliage[b].inst[j].pos +
          vec3f(-cos(interang)*HMULT,-sin(interang)*HMULT,0.0f) * tileptr->foliage[b].inst[j].scale;
        tmpv.st = vec2f(0.0f,0.0f);
        ramfile1.write(&tmpv,sizeof(PVert_tv));
        tileptr->foliage[b].numvert++;
        
        tmpv.xyz = tileptr->foliage[b].inst[j].pos +
          vec3f(-cos(interang)*HMULT,-sin(interang)*HMULT,VMULT) * tileptr->foliage[b].inst[j].scale;
        tmpv.st = vec2f(0.0f,1.0f-1.0f/32.0f);
        ramfile1.write(&tmpv,sizeof(PVert_tv));
        tileptr->foliage[b].numvert++;
        
        tmpv.xyz = tileptr->foliage[b].inst[j].pos +
          vec3f(cos(interang)*HMULT,sin(interang)*HMULT,VMULT) * tileptr->foliage[b].inst[j].scale;
        tmpv.st = vec2f(1.0f,1.0f-1.0f/32.0f);
        ramfile1.write(&tmpv,sizeof(PVert_tv));
        tileptr->foliage[b].numvert++;
        
        int ind;
        ind = stv + 0;
        ramfile2.write(&ind,sizeof(uint32));
        tileptr->foliage[b].numelem++;
        ind = stv + 1;
        ramfile2.write(&ind,sizeof(uint32));
        tileptr->foliage[b].numelem++;
        ind = stv + 2;
        ramfile2.write(&ind,sizeof(uint32));
        tileptr->foliage[b].numelem++;
        ind = stv + 0;
        ramfile2.write(&ind,sizeof(uint32));
        tileptr->foliage[b].numelem++;
        ind = stv + 2;
        ramfile2.write(&ind,sizeof(uint32));
        tileptr->foliage[b].numelem++;
        ind = stv + 3;
        ramfile2.write(&ind,sizeof(uint32));
        tileptr->foliage[b].numelem++;
      }
    }
    
    if (tileptr->foliage[b].numelem) {
      tileptr->foliage[b].buff[0].create(ramfile1.getSize(),
        PVBuffer::VertexContent, PVBuffer::StaticUsage, ramfile1.getData());
      tileptr->foliage[b].buff[1].create(ramfile2.getSize(),
        PVBuffer::IndexContent, PVBuffer::StaticUsage, ramfile2.getData());
    }
  }
  
  //PUtil::outLog() << "2: " << tileptr->posx << " " << tileptr->posy << std::endl;
  return tileptr;
}


void PTerrain::render(const vec3f &campos, const mat44f &camorim)
{
  float blah = camorim.row[0][0]; blah = blah; // unused
  
  // increase all lru counters
  for (std::list<PTerrainTile>::iterator iter = tile.begin();
    iter != tile.end(); ++iter) ++iter->lru_counter;

  // get frustum
  frustumf frust;
  {
    mat44f mat_mv, mat_p, mat_c;

    glGetFloatv(GL_MODELVIEW_MATRIX, mat_mv);
    glGetFloatv(GL_PROJECTION_MATRIX, mat_p);

    mat_c = mat_mv.concatenate(mat_p);

    frust.construct(mat_c);
  }

  int ctx = (int)(campos.x * scale_tile_inv);
  if (campos.x < 0.0) --ctx;
  int cty = (int)(campos.y * scale_tile_inv);
  if (campos.y < 0.0) --cty;

  int mintx = ctx - 3,
    maxtx = ctx + 4,
    minty = cty - 3,
    maxty = cty + 4;
  
  // Determine list of tiles to draw
  
  std::list<PTerrainTile *> drawtile;
  
  for (int ty = minty; ty < maxty; ++ty) {
    for (int tx = mintx; tx < maxtx; ++tx) {
      drawtile.push_back(getTile(tx,ty));
    }
  }
  
  // Draw terrain
  
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);

  float tgens[] = { scale_tile_inv, 0.0, 0.0, 0.0 };
  float tgent[] = { 0.0, scale_tile_inv, 0.0, 0.0 };
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

  glEnableClientState(GL_VERTEX_ARRAY);

  for (std::list<PTerrainTile *>::iterator t = drawtile.begin(); t != drawtile.end(); t++) {
    //if (frust.isAABBOutside(tileptr->mins, tileptr->maxs))
    //    glColor3f(1,0,0);
    //else
    //    glColor3f(1,1,1);

    tgens[3] = (float) (- (*t)->posx);
    tgent[3] = (float) (- (*t)->posy);

    glTexGenfv(GL_S, GL_OBJECT_PLANE, tgens);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, tgent);

    // Texture
    (*t)->tex.bind();

    // Vertex buffers
    (*t)->vert.bind();
    ind.bind();
    glVertexPointer(3, GL_FLOAT, sizeof(vec3f), (*t)->vert.getPointer(0));

    glDrawRangeElements(GL_TRIANGLE_STRIP, 0, (*t)->numverts,
      numinds, GL_UNSIGNED_SHORT, ind.getPointer(0));
  }

  glDisableClientState(GL_VERTEX_ARRAY);

  PVBuffer::unbind();

  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);

  // Don't apply terrain detail texture to foliage.
  // http://sourceforge.net/p/trigger-rally/discussion/527953/thread/b53361ba/
  glActiveTextureARB(GL_TEXTURE1_ARB);
  glDisable(GL_TEXTURE_2D);
  glActiveTextureARB(GL_TEXTURE0_ARB);

  // Draw foliage
  #if 1
  glAlphaFunc(GL_GEQUAL, 0.5);
  glEnable(GL_ALPHA_TEST);
  glDisable(GL_CULL_FACE);
  glColor3f(1.0f, 1.0f, 1.0f);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  
  for (unsigned int b = 0; b < foliageband.size(); b++) {
    
    foliageband[b].sprite_tex->bind();
    
    for (std::list<PTerrainTile *>::iterator t = drawtile.begin(); t != drawtile.end(); t++) {
      
      if ((*t)->foliage[b].numelem) {
        (*t)->foliage[b].buff[0].bind(); // vert data
        (*t)->foliage[b].buff[1].bind(); // indices
        
        glTexCoordPointer(2, GL_FLOAT, sizeof(PVert_tv), (*t)->foliage[b].buff[0].getPointer(0));
        glVertexPointer(3, GL_FLOAT, sizeof(PVert_tv), (*t)->foliage[b].buff[0].getPointer(sizeof(float)*2));
        
        glDrawRangeElements(GL_TRIANGLES,
          0,(*t)->foliage[b].numvert,(*t)->foliage[b].numelem,
          GL_UNSIGNED_INT,(*t)->foliage[b].buff[1].getPointer(0));
      }
      
      #if 0
      for (std::vector<PTerrainFoliage>::iterator f = (*t)->foliage.begin(); f != (*t)->foliage.end(); f++) {
        
        #if 0
        glBegin(GL_LINES);
        vec3f pos = f->pos;
        glVertex3fv(pos);
        pos += vec3f(0.0f, 0.0f, 2.0f);
        glVertex3fv(pos);
        glEnd();
        #endif
        
        #if 0
        if (!f->tfb->model) continue;
        
        glPushMatrix();
        vec3f &pos = f->pos;
        glTranslatef(pos.x, pos.y, pos.z);
        glScalef(f->tfb->modelscale, f->tfb->modelscale, f->tfb->modelscale);
        ssRender.drawModel(*f->tfb->model, ssEffect, ssTexture);
        glPopMatrix();
        #endif
      }
      #endif
    }
  }
  #endif
  
  PVBuffer::unbind();
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glEnable(GL_CULL_FACE);
  glDisable(GL_ALPHA_TEST);
}

void PTerrain::drawSplat(float x, float y, float scale, float angle)
{
  float *hmd = &hmap[0];
  int cx = totsize;
  int cy = totsize;
  
  x *= scale_hz_inv;
  y *= scale_hz_inv;
  
  scale *= 0.5f;
  
  int miny = (int)(y - scale);
  if ((y - scale) < 0.0f) miny--;
  int maxy = (int)(y + scale) + 1;
  if ((y + scale) < 0.0f) maxy--;
  int minx = (int)(x - scale);
  if ((x - scale) < 0.0f) minx--;
  int maxx = (int)(x + scale) + 2;
  if ((x + scale) < 0.0f) maxx--;
  
  glMatrixMode(GL_TEXTURE);
  
  glPushMatrix();
  glTranslatef(0.5f, 0.5f, 0.0f);
  glRotatef(DEGREES(angle), 0.0f, 0.0f, 1.0f);
  glScalef(0.5f / scale, 0.5f / scale, 1.0f);
  glTranslatef(-x, -y, 0.0f);
  
  for (int y2=miny; y2<maxy; y2++) {
    int yc = y2 & (cy-1),
      ycp1 = (yc+1) & (cy-1),
      yc_cx = yc * cx,
      ycp1_cx = ycp1 * cx;
    
    glBegin(GL_TRIANGLE_STRIP);
    for (int x2=minx; x2<maxx; x2++) {
      int xc = x2 & (cx - 1);
      glTexCoord2i(x2, y2 + 1);
      glVertex3f(x2 * scale_hz, (y2 + 1) * scale_hz, hmd[ycp1_cx + xc]);
      glTexCoord2i(x2, y2);
      glVertex3f(x2 * scale_hz, y2 * scale_hz, hmd[yc_cx + xc]);
    }
    glEnd();
  }
  glPopMatrix();
  
  glMatrixMode(GL_MODELVIEW);
}



