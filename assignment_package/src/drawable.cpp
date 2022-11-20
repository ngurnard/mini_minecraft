#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_countOpq(-1), m_countTra(-1), m_bufIdxOpq(), m_bufIdxTra(),
      m_bufPos(), m_bufNor(), m_bufCol(), m_bufUV(),
      m_bufInterleavedOpq(-1), m_bufInterleavedTra(-1),
      m_idxOpqGenerated(false), m_idxTraGenerated(false),
      m_posGenerated(false), m_norGenerated(false), m_colGenerated(false),
      m_interleavedOpqGenerated(false), m_interleavedTraGenerated(false),
      m_uvGenerated(false), mp_context(context),
      opaquePass(true)
{}

Drawable::~Drawable()
{}


void Drawable::destroyVBOdata()
{
    mp_context->glDeleteBuffers(1, &m_bufIdxOpq);
    mp_context->glDeleteBuffers(1, &m_bufIdxTra);
    mp_context->glDeleteBuffers(1, &m_bufPos);
    mp_context->glDeleteBuffers(1, &m_bufNor);
    mp_context->glDeleteBuffers(1, &m_bufCol);
    mp_context->glDeleteBuffers(1, &m_bufUV);
    mp_context->glDeleteBuffers(1, &m_bufInterleavedOpq);
    mp_context->glDeleteBuffers(1, &m_bufInterleavedTra);
    m_idxOpqGenerated = m_idxTraGenerated = m_posGenerated = m_norGenerated =
        m_colGenerated = m_uvGenerated = m_interleavedOpqGenerated = m_interleavedTraGenerated = false;
    m_countOpq = m_countTra = -1;
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    if (opaquePass) {
        return m_countOpq;
    } else {
        return m_countTra;
    }
}

void Drawable::generateIdx()
{
    // Create a VBO on our GPU and store its handle in bufIdxOpq or bufIdxTra
    if (opaquePass) {
        m_idxOpqGenerated = true;
        mp_context->glGenBuffers(1, &m_bufIdxOpq);
    } else {
        m_idxTraGenerated = true;
        mp_context->glGenBuffers(1, &m_bufIdxTra);
    }
}

void Drawable::generatePos()
{
    m_posGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufPos);
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    mp_context->glGenBuffers(1, &m_bufNor);
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

void Drawable::generateInterleavedList()
{
    // Create a VBO on our GPU and store its handle in bufInterleaved lists
    // depending on whethere we're building opaque or transparent VBO
    if (opaquePass) {
        m_interleavedOpqGenerated = true;
        mp_context->glGenBuffers(1, &m_bufInterleavedOpq);
    } else {
        m_interleavedTraGenerated = true;
        mp_context->glGenBuffers(1, &m_bufInterleavedTra);
    }
}

void Drawable::generateUV()
{
    m_uvGenerated = true;
    // Create a VBO on our GPU and store its handle in bufInterleavedList
    mp_context->glGenBuffers(1, &m_bufUV);
}

bool Drawable::bindIdx()
{
    if (opaquePass) {
        if(m_idxOpqGenerated) {
            mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpq);
        }
        return m_idxOpqGenerated;

    } else {
        if(m_idxTraGenerated) {
            mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTra);
        }
        return m_idxTraGenerated;
    }
}

bool Drawable::bindPos()
{
    if(m_posGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindInterleavedList()
{
    if (opaquePass) {
        if(m_interleavedOpqGenerated){
            mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterleavedOpq);
        }
        return m_interleavedOpqGenerated;

    } else {
        if(m_interleavedTraGenerated){
            mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterleavedTra);
        }
        return m_interleavedTraGenerated;
    }
}

bool Drawable::bindUV()
{
    if(m_uvGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    }
    return m_uvGenerated;
}

InstancedDrawable::InstancedDrawable(OpenGLContext *context)
    : Drawable(context), m_numInstances(0), m_bufPosOffset(-1), m_offsetGenerated(false)
{}

InstancedDrawable::~InstancedDrawable(){}

int InstancedDrawable::instanceCount() const {
    return m_numInstances;
}

void InstancedDrawable::generateOffsetBuf() {
    m_offsetGenerated = true;
    mp_context->glGenBuffers(1, &m_bufPosOffset);
}

bool InstancedDrawable::bindOffsetBuf() {
    if(m_offsetGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPosOffset);
    }
    return m_offsetGenerated;
}


void InstancedDrawable::clearOffsetBuf() {
    if(m_offsetGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufPosOffset);
        m_offsetGenerated = false;
    }
}
void InstancedDrawable::clearColorBuf() {
    if(m_colGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufCol);
        m_colGenerated = false;
    }
}
