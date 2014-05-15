
// model.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"


PSSModel::PSSModel(PApp &parentApp) : PSubsystem(parentApp)
{
  app.outLog() << "Initialising model subsystem" << std::endl;
}

PSSModel::~PSSModel()
{
  app.outLog() << "Shutting down model subsystem" << std::endl;

  modlist.clear();
}


PModel *PSSModel::loadModel(const std::string &name)
{
  PModel *mdl = modlist.find(name);
  if (!mdl) {
    try
    {
      mdl = new PModel (name);
    }
    catch (PException e)
    {
      if (PUtil::isDebugLevel(DEBUGLEVEL_ENDUSER))
        PUtil::outLog() << "Failed to load " << name << ": " << e.what () << std::endl;
      return null;
    }
    modlist.add(mdl);
  }
  return mdl;
}


#define dcon_printf(...)



char *strtok2(char *input)
{
  static char *inputstore = null;

  if (input != null) inputstore = input;

  if (!inputstore) return null;

  // eat whitespace
  while (*inputstore == ' ' || *inputstore == '\t' || *inputstore == '\n' || *inputstore == '\r') inputstore++;

  char *tokstart = inputstore;

  if (*inputstore == '\"') {
    tokstart++; inputstore++;
    while ((*inputstore)) {
      if (*inputstore == '\"') {
        *inputstore = 0;
        inputstore++;
        break;
      }
      inputstore++;
    }
  } else {
    while ((*inputstore)) {
      if (*inputstore == ' ' || *inputstore == '\t' || *inputstore == '\n' || *inputstore == '\r') break;
      inputstore++;
    }
  }

  if (*inputstore) {
    char *nullout = inputstore;
    // eat whitespace
    while (*inputstore == ' ' || *inputstore == '\t' || *inputstore == '\n' || *inputstore == '\r') inputstore++;
    *nullout = 0;
    if (!*inputstore)
      inputstore = null;
  } else {
    inputstore = null;
  }

  return tokstart;
}


// PModel



std::pair<vec3f, vec3f> PModel::getExtents() const
{
  vec3f v_min(1000000000.0, 1000000000.0, 1000000000.0),
    v_max(-1000000000.0, -1000000000.0, -1000000000.0);

  for (unsigned int a=0; a<mesh.size(); ++a) {
    for (unsigned int b=0; b<mesh[a].vert.size(); ++b) {
      if (v_min.x > mesh[a].vert[b].x)
        v_min.x = mesh[a].vert[b].x;
      if (v_max.x < mesh[a].vert[b].x)
        v_max.x = mesh[a].vert[b].x;
      if (v_min.y > mesh[a].vert[b].y)
        v_min.y = mesh[a].vert[b].y;
      if (v_max.y < mesh[a].vert[b].y)
        v_max.y = mesh[a].vert[b].y;
      if (v_min.z > mesh[a].vert[b].z)
        v_min.z = mesh[a].vert[b].z;
      if (v_max.z < mesh[a].vert[b].z)
        v_max.z = mesh[a].vert[b].z;
    }
  }

  return std::pair<vec3f, vec3f> (v_min, v_max);
}


struct matl_s {
  std::string filename;
};


PModel::PModel (const std::string &filename, float globalScale)
{
   /* Let's check each model type will load (ASE or OBJ) */
   if(filename.find(".ase") != std::string::npos)
   {
      loadASE(filename, globalScale);
   }
   else
   {
      loadOBJ(filename, globalScale);
   }
}

/*! Load an .obj model from file to the pengine structures.
 * \note: All model faces must be triangles;
 * \note: Not setting the pengine per face normal (mesh.facenormal) as the 
 *        renderer is ignoring it (only setting vertex normals)
 * FIXME: Restriction: Model must have only a single material. 
 *                     See comment bellow on how to fix it.  */
void PModel::loadOBJ(const std::string &filename, float globalScale)
{
   std::vector<vec3f> normals;  /**< Vector with all normals readed from file */
   char buff[1000];             /**< File buffer */
   PHYSFS_file* pfile;          /**< The real .obj file */
   std::string tok;             /**< Readed token from line */
   std::string value;           /**< Readed value from line */
   int curVert=-1;              /**< Current readed vertex */
   int curFace=-1;              /**< Current readed face */
   int curNormal=-1;            /**< Current readed normal */
   int curUV=-1;                /**< Current readed uvmap */
   int objNumber=0;             /**< Number of objects declared */
   PMesh* curMesh;              /**< Current loading mesh */
   vec3f v3;                    /**< Vector to parse from lines */
   vec2f v2;                    /**< Vector to parse from lines */
   
   /* Initing debug message */
   if(PUtil::isDebugLevel(DEBUGLEVEL_TEST))
   {
      PUtil::outLog() << "Loading OBJ model \"" << filename 
         << "\"" << std::endl;
   }

   /* Open the .obj file */
   pfile = PHYSFS_openRead(filename.c_str());
   if(pfile == NULL)
   {
      throw MakePException(filename + ", PhysFS: " + PHYSFS_getLastError());
   }

   /* Create the single mesh (.obj isn't a multimesh file) */
   mesh.push_back(PMesh());
   curMesh = &mesh.back();

   /* Loop throught all file */
   while(PUtil::fgets2(buff,1000,pfile))
   {
      if(PUtil::getToken(buff, tok, value))
      {
         if(tok[0] == '#')
         {
            /* Comment. Just ignore. */
         }
         else if(tok == "v")
         {
            /* Vertex declaration */
            curVert++;
            curMesh->vert.resize(curVert+1);
            if(sscanf(value.c_str(), "%f %f %f", 
                     &v3.x, &v3.y, &v3.z) == 3)
            { 
               curMesh->vert[curVert] = v3 * globalScale;
            }
         }
         else if(tok == "vn")
         {
            /* Vertex Normal declaraction */
            curNormal++;
            curMesh->norm.resize(curNormal+1);
            if(sscanf(value.c_str(), "%f %f %f",
                     &v3.x, &v3.y, &v3.z) == 3)
            {
               curMesh->norm[curNormal] = v3;
               curMesh->norm[curNormal].normalize();
            }
         }
         else if(tok == "vt")
         {
            /* Vertex st texture coordinate */
            curUV++;
            curMesh->texco.resize(curUV+1);
            if(sscanf(value.c_str(), "%f %f", &v2.x, &v2.y) == 2)
            {
               curMesh->texco[curUV] = v2;
            }
         }
         else if(tok == "f")
         {
            /* Face (triangle) declaration */
            int v[3],uv[3],vn[3];
            curFace++;
            curMesh->face.resize(curFace+1);
            if(sscanf(value.c_str(), "%d/%d/%d %d/%d/%d %d/%d/%d", 
                &v[0], &uv[0], &vn[0], 
                &v[1], &uv[1], &vn[1],
                &v[2], &uv[2], &vn[2]) == 9)
            {
               /* NOTE: all index are dec by 1, as .obj range is
                * [1,total] and pengine vector is [0,total) */

               /* Set Vertex Index */
               curMesh->face[curFace].vt[0] = v[0]-1;
               curMesh->face[curFace].vt[1] = v[1]-1;
               curMesh->face[curFace].vt[2] = v[2]-1;
               /* Set UV Index  */
               curMesh->face[curFace].tc[0] = uv[0]-1;
               curMesh->face[curFace].tc[1] = uv[1]-1;
               curMesh->face[curFace].tc[2] = uv[2]-1;
               /* Set Normal Index  */
               curMesh->face[curFace].nr[0] = vn[0]-1;
               curMesh->face[curFace].nr[1] = vn[1]-1;
               curMesh->face[curFace].nr[2] = vn[2]-1;
            
            }
         }
         else if(tok == "mtllib")
         {
            /* Material Library declaration (mtllib) */
            curMesh->fxname = PUtil::assemblePath(value/*"focus_tex.fx"*/, 
                  filename);
         }
         else if(tok == "o")
         {
            /* Object name. Just ignore. */
            objNumber++;
            if(objNumber > 1)
            {
               PUtil::outLog() << "Warning: Object file \"" << filename 
                  << "\" has more than one object defined!" << std::endl;

            }
         }
         else if(tok == "usemtl")
         {
            /* Face material usage. (usemtl). 
             * FIXME: Ignoring, as the pengine renderer is 
             * using only a single "fx" per mesh.
             *
             * A bad fix should just duplicate each distinct material faces
             * as different meshes.
             *
             * A good fix should rewrite the renderer (at ./app.cpp) to 
             * change materials on a single mesh as needed, allowing multiple 
             * material meshes. 
             *
             * I'm do either of them, but just mark it as a restriction to
             * .obj files on trigger. Someone must remove this restriction 
             * latter */
         }
         else if(tok == "s")
         {
            /* Smooth toggle. Ignoring. */
         }
         else
         {
            PUtil::outLog () << "Warning: unknow token \"" << tok
               << "\" in file \"" << filename << "\"" << std::endl;
         }
      }
   }

   /* Verify if normals were defined */
   if(curNormal == -1)
   {
      PUtil::outLog() << "Warning: Object file \"" << filename 
         << "\" had no normals defined!" << std::endl;

   }

   /* Finally, close file and done. */
   PHYSFS_close(pfile);
   name = filename;
}



void PModel::loadASE (const std::string &filename, float globalScale)
{
  if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
    PUtil::outLog() << "Loading ASE model \"" << filename << "\"" << std::endl;

  PHYSFS_file *pfile = PHYSFS_openRead(filename.c_str());
  if (pfile == null) {
    throw MakePException (filename + ", PhysFS: " + PHYSFS_getLastError());
  }

  std::vector<matl_s> matlist;

  char buff[1000],buff2[1000],buff3[1000];
  std::vector<char*> tok;

  int unknowndepth = 0;

#define TOKENIZE_LINE_AND_CHECK \
    { \
      char *thistok; \
      tok.clear(); \
      thistok = strtok2(buff); \
      while (thistok) { \
        tok.push_back(thistok); \
        thistok = strtok2(NULL); \
      } \
    } \
    if (!tok.size()) continue; \
    if (unknowndepth > 0) { \
      if (!strcmp(tok[0],"}")) \
        unknowndepth--; \
      continue; \
    }

  while (PUtil::fgets2(buff,1000,pfile)) {
    dcon_printf("\"%s\"<br>\n",buff);
    TOKENIZE_LINE_AND_CHECK
    if (tok.size() == 2 && !strcmp(tok[1],"{")) {
      if (!strcmp(tok[0],"*MATERIAL_LIST")) {
        dcon_printf("*MATERIAL_LIST<br>\n");
        while (PUtil::fgets2(buff,1000,pfile)) {
          strcpy(buff2,buff);
          TOKENIZE_LINE_AND_CHECK
          if (!strcmp(tok[0],"}")) break;
#if 0
          if (!strcmp(tok[0],"*MATERIAL")) {
            dcon_printf("*MATERIAL<br>\n");
            int matind = atoi(tok[1]);
            dcon_printf("index = %i, matlist.size() = %i<br>\n",matind,matlist.size());
            if (matind < 0) continue;
            if (matind >= matlist.size()) matlist.resize(matind+1);
            while (fgets2(buff,1000,pfile)) {
              strcpy(buff2,buff);
              TOKENIZE_LINE_AND_CHECK
              if (!strcmp(tok[0],"}")) break;
              if (!strcmp(tok[0],"*MAP_DIFFUSE")) {
                dcon_printf("*MAP_DIFFUSE<br>\n");
                while (fgets2(buff,1000,pfile)) {
                  strcpy(buff2,buff);
                  TOKENIZE_LINE_AND_CHECK
                  if (!strcmp(tok[0],"}")) break;
                  if (sscanf(buff2," *BITMAP \"%[^\"]\"",buff3) == 1) {
                    matlist[matind].filename = PUtil::assemblePath(buff3, filename);
                  }
                }
              }
            }
          }
#else
          unsigned int matind;
          if (sscanf(buff2," *MATERIAL %u \"%[^\"]\"",&matind,buff3) == 2) {
            //if (matind >= 0) {
              if (matind >= matlist.size()) matlist.resize(matind+1);
              matlist[matind].filename = PUtil::assemblePath(buff3, filename);
            //}
          }
#endif
        }
      } else if (!strcmp(tok[0],"*GEOMOBJECT")) {
        dcon_printf("*GEOMOBJECT<br>\n");
        vec3f tm[4];
        int tempi;
        PMesh *curmesh = null;
        tm[0] = vec3f(1,0,0);
        tm[1] = vec3f(0,1,0);
        tm[2] = vec3f(0,0,1);
        tm[3] = vec3f::zero();
#define DO_TM2(v) (vec3f((v)*tm[0],(v)*tm[1],(v)*tm[2]))
        while (PUtil::fgets2(buff,1000,pfile)) {
          strcpy(buff2,buff);
          TOKENIZE_LINE_AND_CHECK
          if (!strcmp(tok[0],"}")) break;
          if (tok.size() == 2 && !strcmp(tok[1],"{")) {
            if (!strcmp(tok[0],"*MESH")) {
              dcon_printf("*MESH<br>\n");
              mesh.push_back(PMesh());
              curmesh = &mesh.back();
              curmesh->effect = null;
              while (PUtil::fgets2(buff,1000,pfile)) {
                TOKENIZE_LINE_AND_CHECK
                if (!strcmp(tok[0],"}")) break;
                if (tok.size() == 2 && !strcmp(tok[1],"{")) {
                  if (!strcmp(tok[0],"*MESH_VERTEX_LIST")) {
                    unsigned int vnum;
                    vec3f vpos;
                    while (PUtil::fgets2(buff,1000,pfile)) {
                      strcpy(buff2,buff);
                      TOKENIZE_LINE_AND_CHECK
                      if (!strcmp(tok[0],"}")) break;
                      if (sscanf(buff2," *MESH_VERTEX %i %f %f %f",&vnum,&vpos.x,&vpos.y,&vpos.z) == 4) {
                        if (vnum < curmesh->vert.size())
                          curmesh->vert[vnum] = vpos * globalScale;
                      }
                    }
                  } else if (!strcmp(tok[0],"*MESH_TVERTLIST")) {
                    unsigned int vnum;
                    vec2f vco;
                    while (PUtil::fgets2(buff,1000,pfile)) {
                      strcpy(buff2,buff);
                      TOKENIZE_LINE_AND_CHECK
                      if (!strcmp(tok[0],"}")) break;
                      if (sscanf(buff2," *MESH_TVERT %i %f %f",&vnum,&vco.x,&vco.y) == 3) {
                        if (vnum < curmesh->texco.size()) {
                          curmesh->texco[vnum] = vco;
                          //curmesh->texco[vnum].y *= -1.0;
                        }
                      }
                    }
                  } else if (!strcmp(tok[0],"*MESH_FACE_LIST")) {
                    unsigned int fnum;
                    int fvt[3];
                    while (PUtil::fgets2(buff,1000,pfile)) {
                      strcpy(buff2,buff);
                      TOKENIZE_LINE_AND_CHECK
                      if (!strcmp(tok[0],"}")) break;
                      if (sscanf(buff2," *MESH_FACE %i: A: %i B: %i C: %i",&fnum,&fvt[0],&fvt[1],&fvt[2]) == 4) {
                        if (fnum < curmesh->face.size()) {
                          curmesh->face[fnum].vt[0] = fvt[0];
                          curmesh->face[fnum].vt[1] = fvt[1];
                          curmesh->face[fnum].vt[2] = fvt[2];
                        }
                      }
                    }
                  } else if (!strcmp(tok[0],"*MESH_TFACELIST")) {
                    unsigned int fnum;
                    int fvt[3];
                    while (PUtil::fgets2(buff,1000,pfile)) {
                      strcpy(buff2,buff);
                      TOKENIZE_LINE_AND_CHECK
                      if (!strcmp(tok[0],"}")) break;
                      if (sscanf(buff2," *MESH_TFACE %i %i %i %i",&fnum,&fvt[0],&fvt[1],&fvt[2]) == 4) {
                        if (fnum < curmesh->face.size()) {
                          curmesh->face[fnum].tc[0] = fvt[0];
                          curmesh->face[fnum].tc[1] = fvt[1];
                          curmesh->face[fnum].tc[2] = fvt[2];
                        }
                      }
                    }
                  } else if (!strcmp(tok[0],"*MESH_NORMALS")) {
                    unsigned int fnum, vnum;
                    vec3f nrm;
                    while (PUtil::fgets2(buff,1000,pfile)) {
                      strcpy(buff2,buff);
                      TOKENIZE_LINE_AND_CHECK
                      if (!strcmp(tok[0],"}")) break;
                      if (sscanf(buff2," *MESH_FACENORMAL %i %f %f %f",&fnum,&nrm.x,&nrm.y,&nrm.z) == 4) {
                        if (fnum < curmesh->face.size()) {
                          curmesh->face[fnum].facenormal = DO_TM2(nrm);
                          curmesh->face[fnum].facenormal.normalize();
                          curmesh->face[fnum].nr[0] = fnum*3+0;
                          curmesh->face[fnum].nr[1] = fnum*3+1;
                          curmesh->face[fnum].nr[2] = fnum*3+2;
                        }
                      } else if (sscanf(buff2," *MESH_VERTEXNORMAL %i %f %f %f",&vnum,&nrm.x,&nrm.y,&nrm.z) == 4) {
                        if (vnum == curmesh->face[fnum].vt[0]) {
                          curmesh->norm[fnum*3+0] = DO_TM2(nrm);
                          curmesh->norm[fnum*3+0].normalize();
                        } else if (vnum == curmesh->face[fnum].vt[1]) {
                          curmesh->norm[fnum*3+1] = DO_TM2(nrm);
                          curmesh->norm[fnum*3+1].normalize();
                        } else if (vnum == curmesh->face[fnum].vt[2]) {
                          curmesh->norm[fnum*3+2] = DO_TM2(nrm);
                          curmesh->norm[fnum*3+2].normalize();
                        }
                      }
                    }
                  } else {
                    unknowndepth++;
                  }
                } else if (!strcmp(tok[0],"*MESH_NUMVERTEX")) {
                  curmesh->vert.resize(atoi(tok[1]));
                } else if (!strcmp(tok[0],"*MESH_NUMFACES")) {
                  curmesh->face.resize(atoi(tok[1]));
                  curmesh->norm.resize(atoi(tok[1])*3); // 3 normals per face
                } else if (!strcmp(tok[0],"*MESH_NUMTVERTEX")) {
                  curmesh->texco.resize(atoi(tok[1]));
                }
              }
            } else if (!strcmp(tok[0],"*NODE_TM")) {
              dcon_printf("*NODE_TM<br>\n");
              while (PUtil::fgets2(buff,1000,pfile)) {
                strcpy(buff2,buff);
                TOKENIZE_LINE_AND_CHECK
                if (!strcmp(tok[0],"}")) {
                  // end of NODE_TM, do some processing
                  vec3f st[3] = { tm[0], tm[1], tm[2] };
                  tm[0] = vec3f(st[0].x, st[1].x, st[2].x);
                  tm[1] = vec3f(st[0].y, st[1].y, st[2].y);
                  tm[2] = vec3f(st[0].z, st[1].z, st[2].z);
                  break;
                }
                if (sscanf(buff2," *TM_ROW0 %f %f %f",&tm[0].x,&tm[0].y,&tm[0].z) == 3) {
                  // do nothing!
                } else if (sscanf(buff2," *TM_ROW1 %f %f %f",&tm[1].x,&tm[1].y,&tm[1].z) == 3) {
                  // do nothing!
                } else if (sscanf(buff2," *TM_ROW2 %f %f %f",&tm[2].x,&tm[2].y,&tm[2].z) == 3) {
                  // do nothing!
                } else if (sscanf(buff2," *TM_ROW3 %f %f %f",&tm[3].x,&tm[3].y,&tm[3].z) == 3) {
                  // do nothing!
                }
              }
            } else {
              unknowndepth++;
            }
          } else if (sscanf(buff2," *MATERIAL_REF %i",&tempi) == 1) {
            dcon_printf("*MATERIAL_REF<br>\n");
            if (!curmesh) {
              PUtil::outLog () << "warning: material ref before mesh in \"" << filename << "\"" << std::endl;
              continue;
            }
            if (0 <= tempi && tempi < (int)matlist.size()) {
              curmesh->fxname = matlist[tempi].filename;
            }
          }
        }
      } else {
        unknowndepth++;
      }
    }
  }

  PHYSFS_close(pfile);

  name = filename;
}


