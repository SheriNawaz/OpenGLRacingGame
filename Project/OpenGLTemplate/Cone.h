#pragma once

#include "Common.h"
#include "Texture.h"
#include "VertexBufferObjectIndexed.h"

// Class for creating and rendering a cone
class CCone
{
public:
    CCone();
    ~CCone();

    // Create the cone with texture
    void Create(string a_sDirectory, string a_sFilename);

    // Render the cone
    void Render();

    // Release resources
    void Release();

private:
    GLuint m_vao;                  // Vertex array object
    CVertexBufferObjectIndexed m_vbo;     // Vertex buffer object
    CTexture m_texture;            // Texture
    string m_directory;            // Directory for resources
    string m_filename;             // Texture filename
    GLsizei m_numIndices;          // Number of indices for rendering
};