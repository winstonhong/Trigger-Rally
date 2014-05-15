
// render.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"



PSSRender::PSSRender(PApp &parentApp) : PSubsystem(parentApp)
{
  app.outLog() << "Initialising render subsystem" << std::endl;
}

PSSRender::~PSSRender()
{
  app.outLog() << "Shutting down render subsystem" << std::endl;
}


void PSSRender::tick(float delta, const vec3f &eyepos, const mat44f &eyeori, const vec3f &eyevel)
{
  float unused = delta; unused = eyevel.x;
  
  cam_pos = eyepos;
  cam_orimat = eyeori;
}


void PSSRender::render(PParticleSystem *psys)
{
  vec3f pushx = makevec3f(cam_orimat.row[0]);
  vec3f pushy = makevec3f(cam_orimat.row[1]);
  vec3f vert;
  
  glBlendFunc(psys->blendparam1, psys->blendparam2);
  
  if (psys->tex) psys->tex->bind();
  else glDisable(GL_TEXTURE_2D);
  
  glBegin(GL_QUADS);
  for (unsigned int i=0; i<psys->part.size(); i++) {
    PParticle_s &part = psys->part[i];
    float sizenow = INTERP(psys->endsize, psys->startsize, part.life);
    vec3f pushxt = pushx * sizenow;
    vec3f pushyt = pushy * sizenow;
    vec3f pushx2 = pushxt * part.orix.x + pushyt * part.orix.y;
    vec3f pushy2 = pushxt * part.oriy.x + pushyt * part.oriy.y;

    glColor4f(INTERP(psys->colorend[0], psys->colorstart[0], part.life),
        INTERP(psys->colorend[1], psys->colorstart[1], part.life),
        INTERP(psys->colorend[2], psys->colorstart[2], part.life),
        INTERP(psys->colorend[3], psys->colorstart[3], part.life));

    vert = part.pos - pushx2 - pushy2;
    glTexCoord2i(0,0);
    glVertex3fv(vert);
    vert = part.pos + pushx2 - pushy2;
    glTexCoord2i(1,0);
    glVertex3fv(vert);
    vert = part.pos + pushx2 + pushy2;
    glTexCoord2i(1,1);
    glVertex3fv(vert);
    vert = part.pos - pushx2 + pushy2;
    glTexCoord2i(0,1);
    glVertex3fv(vert);
  }
  glEnd();
  
  if (!psys->tex) glEnable(GL_TEXTURE_2D);
}

void PSSRender::drawModel(PModel &model, PSSEffect &ssEffect, PSSTexture &ssTexture)
{
  for (std::vector<PMesh>::iterator mesh = model.mesh.begin();
    mesh != model.mesh.end();
    mesh++) {
    if (!mesh->effect)
      mesh->effect = ssEffect.loadEffect(mesh->fxname);

    int numPasses = 0;
    if (mesh->effect->renderBegin(&numPasses, ssTexture)) {
      for (int i=0; i<numPasses; i++) {
        mesh->effect->renderPass(i);
        glBegin(GL_TRIANGLES);
        for (unsigned int f=0; f<mesh->face.size(); f++) {
          //glNormal3fv(mesh->face[f].facenormal);

          glNormal3fv(mesh->norm[mesh->face[f].nr[0]]);
          glTexCoord2fv(mesh->texco[mesh->face[f].tc[0]]);
          glVertex3fv(mesh->vert[mesh->face[f].vt[0]]);

          glNormal3fv(mesh->norm[mesh->face[f].nr[1]]);
          glTexCoord2fv(mesh->texco[mesh->face[f].tc[1]]);
          glVertex3fv(mesh->vert[mesh->face[f].vt[1]]);

          glNormal3fv(mesh->norm[mesh->face[f].nr[2]]);
          glTexCoord2fv(mesh->texco[mesh->face[f].tc[2]]);
          glVertex3fv(mesh->vert[mesh->face[f].vt[2]]);
        }
        glEnd();
      }
      mesh->effect->renderEnd();
    }
  }
}

void PSSRender::drawText(const std::string &text, uint32 flags)
{
  const float font_aspect = 0.6f;
  
  glPushMatrix();
  
  if (flags & PTEXT_VTA_CENTER)
    glTranslatef(0.0f, -0.5f, 0.0f);
  else if (flags & PTEXT_VTA_TOP)
    glTranslatef(0.0f, -1.0f, 0.0f);
  
  if (flags & PTEXT_HZA_CENTER)
    glTranslatef(-((float)text.length()) * 0.5f * font_aspect, 0.0f, 0.0f);
  else if (flags & PTEXT_HZA_RIGHT)
    glTranslatef(-((float)text.length()) * font_aspect, 0.0f, 0.0f);
  
  for (std::string::const_iterator c = text.begin(); c != text.end(); c++) {
    float tx = ((float)(*c % 16) + 0.5f - font_aspect*0.5f) / 16.0f, addx = font_aspect / 16.0f;
    float ty = (*c / 16) / 16.0f, addy = 1 / 16.0f;
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(tx,ty);
    glVertex2f(0.0f,0.0f);
    glTexCoord2f(tx+addx,ty);
    glVertex2f(font_aspect,0.0f);
    glTexCoord2f(tx,ty+addy);
    glVertex2f(0.0f,1.0f);
    glTexCoord2f(tx+addx,ty+addy);
    glVertex2f(font_aspect,1.0f);
    glEnd();
    glTranslatef(font_aspect,0,0);
  }
  
  glPopMatrix();
}

vec2f PSSRender::getTextDims(const std::string &text)
{
  const float font_aspect = 0.6f;
  
  return vec2f((float)text.length() * font_aspect, 1.0f);
}


void PParticleSystem::addParticle(const vec3f &pos, const vec3f &linvel)
{
  part.push_back(PParticle_s());
  part.back().pos = pos;
  part.back().linvel = linvel;
  part.back().life = 1.0;

  float ang = randm11 * PI;
  part.back().orix = vec2f(cos(ang),sin(ang));
  part.back().oriy = vec2f(-sin(ang),cos(ang));
}


void PParticleSystem::tick(float delta)
{
  float decr = delta * decay;

  // update life and delete dead particles
  unsigned int j=0;
  for (unsigned int i=0; i<part.size(); i++) {
    part[j] = part[i];
    part[j].life -= decr;
    if (part[j].life > 0.0) j++;
  }
  part.resize(j);
  
  for (unsigned int i=0; i<part.size(); i++) {
    part[i].pos += part[i].linvel * delta;
  }
}



