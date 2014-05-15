
// vbuffer.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"


#define USE_VBO_ARB (GLEW_ARB_vertex_buffer_object)
//#define USE_VBO_ARB (false)

// emulation of GL buffer binding functionality
//PVBuffer *PVBuffer::bound = null;
//PVBuffer *PVBuffer::boundelem = null;


PVBuffer::~PVBuffer()
{
    unload();
}


void PVBuffer::unload()
{
    if (USE_VBO_ARB) {
        if (buffid) glDeleteBuffersARB(1, &buffid);
        buffid = 0;
    } else {
        if (buffer) delete[] buffer;
        buffer = null;
    }
}


bool PVBuffer::create(int buffsize, contenttype type, usagemode usage, const void *data)
{
    unload();

    pr_type = type;

    if (USE_VBO_ARB) {
        switch (type) {
        default:
        case VertexContent:
            target = GL_ARRAY_BUFFER_ARB;
            break;
        case IndexContent:
            target = GL_ELEMENT_ARRAY_BUFFER_ARB;
            break;
        }

        glGenBuffersARB(1, &buffid);
        bind();

        GLenum gl_usage;
        switch (usage) {
        case StreamUsage:
            gl_usage = GL_STREAM_DRAW_ARB;
            break;
        default:
        case StaticUsage:
            gl_usage = GL_STATIC_DRAW_ARB;
            break;
        case DynamicUsage:
            gl_usage = GL_DYNAMIC_DRAW_ARB;
            break;
        }

        glGetError();

        //int getter;
        //glGetIntegerv(GL_ARRAY_BUFFER_BINDING_ARB,&getter);
        //con_printf("bufferdata with object %i bound\n",getter);

        glBufferDataARB(target, buffsize, data, gl_usage);

        unbind();

        int err = glGetError();
        if (err != GL_NO_ERROR) {
            PUtil::outLog() << buffsize << " byte buffer creation failed, ";
            switch (err) {
            case GL_INVALID_OPERATION:
                PUtil::outLog() << "invalid operation\n";
                break;
            case GL_INVALID_ENUM:
                PUtil::outLog() << "invalid enum\n";
                break;
            case GL_OUT_OF_MEMORY:
                PUtil::outLog() << "out of memory\n";
                break;
            default:
                PUtil::outLog() << "GL error code " << err << "\n";
                break;
            }
            unload();
            buffid = 0;
            return false;
        }
    } else {
        buffer = new uint8 [buffsize];
        if (!buffer) {
            PUtil::outLog() << buffsize << " byte buffer creation failed (out of memory)\n";
            return false;
        }

        memcpy(buffer, data, buffsize);

        //buffersize = buffsize;
    }

    return true;
}

void PVBuffer::update(int offset, int buffsize, const void *data)
{
    if (USE_VBO_ARB) {
        bind();
        glBufferSubDataARB(target, offset, buffsize, data);
        unbind();
    } else {
        memcpy(buffer + offset, data, buffsize);
    }
}

uint8 *PVBuffer::getPointer(int offset)
{
    if (USE_VBO_ARB) {
        return ((uint8*)null + offset);
    } else {
        return ((uint8*)buffer + offset);
    }
}

void PVBuffer::bind()
{
    if (USE_VBO_ARB) {
        glBindBufferARB(target, buffid);
    } else {
        // don't bind if buffer hasn't been allocated
        //switch (pr_type) {
        //case VertexContent: bound = buffer ? this : null; break;
        //case IndexContent: boundelem = buffer ? this : null; break;
        //}
    }
}

// static
void PVBuffer::unbind()
{
    if (USE_VBO_ARB) {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    } else {
        //bound = null;
        //boundelem = null;
    }
}


// RamFile


void PRamFile::write(const void *data, int datasize)
{
    int newextent = cursor + datasize;
    int buffsize = buffer.size();
    if (newextent > buffsize) {
        buffer.resize(newextent);
        if (cursor > buffsize)
            memset(&buffer[buffsize], 0, cursor-buffsize);
    }

    memcpy(&buffer[cursor], data, datasize);
    cursor += datasize;
}




