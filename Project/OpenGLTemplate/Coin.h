#pragma once
#include "Common.h"
#include "Texture.h"
#include "VertexBufferObjectIndexed.h"
class CCoin
{
public:
    CCoin();
    ~CCoin();
    void Create(string a_sDirectory, string a_sFilename, int slicesIn, float thickness);
    void Render();
    void Release();
private:
    GLuint m_vao;
    CVertexBufferObjectIndexed m_vbo;
    CTexture m_texture;
    string m_directory;
    string m_filename;
    int m_numTriangles;
};