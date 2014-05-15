
// vbuffer.h [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)



#define BUFFER_OFFSET(i) ((char *)null + (i))


class PVBuffer {
public:
    enum contenttype {
        VertexContent, IndexContent
    };
    enum usagemode {
        StreamUsage, StaticUsage, DynamicUsage
    };

protected:
    GLuint buffid;
    GLenum target;

    contenttype pr_type;

    // emulation:

    uint8 *buffer;
    //int buffersize; // not used anyway

    // in order to implement vbuffer emulation, need to add
    // DrawRangeElements type call to this class

    //static PVBuffer *bound, *boundelem;

public:
    PVBuffer() { buffid = 0; buffer = null; }
    ~PVBuffer();

    bool create(int buffsize, contenttype type, usagemode usage, const void *data = null);
    void update(int offset, int buffsize, const void *data);
    void unload();

    uint8 *getPointer(int offset);

    void bind();

    static void unbind();
};


class PRamFile {
protected:
    std::vector<uint8> buffer;
    int cursor;

public:
    PRamFile() { cursor = 0; }

    void write(const void *data, int datasize);

    void seek_cur(int pos) { seek_set(pos + cursor); }
    void seek_end(int pos) { seek_set(pos + buffer.size()); }
    void seek_set(int pos) { cursor = (pos >= 0) ? pos : 0; }
    int tell() { return cursor; }

    void clear() { buffer.clear(); cursor = 0; }

    void *getData() { return &buffer[0]; }
    int getSize() { return buffer.size(); }
};



