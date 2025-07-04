#include "Common.h"

#define _USE_MATH_DEFINES
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#include "Cone.h"
#include <math.h>

CCone::CCone()
{
}

CCone::~CCone()
{
}

void CCone::Create(string a_sDirectory, string a_sFilename)
{
    // Load texture if filename is provided
    m_texture.Load(a_sDirectory + a_sFilename);

    m_directory = a_sDirectory;
    m_filename = a_sFilename;

    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    m_vbo.Create();
    m_vbo.Bind();

    // Define parameters for the cone
    const int NUM_SEGMENTS = 32;
    const float RADIUS = 5.0f;
    const float HEIGHT = 10.0f;

    const int NUM_VERTICES = NUM_SEGMENTS + 2;

    std::vector<glm::vec3> vertices(NUM_VERTICES);
    std::vector<glm::vec2> texCoords(NUM_VERTICES);
    std::vector<glm::vec3> normals(NUM_VERTICES);
    std::vector<unsigned int> indices;

    // Set the apex (tip) of the cone
    vertices[0] = glm::vec3(0.0f, HEIGHT, 0.0f);
    texCoords[0] = glm::vec2(0.5f, 1.0f);
    normals[0] = glm::vec3(0.0f, 1.0f, 0.0f);

    // Set the center of the base
    vertices[1] = glm::vec3(0.0f, 0.0f, 0.0f);
    texCoords[1] = glm::vec2(0.5f, 0.5f);
    normals[1] = glm::vec3(0.0f, -1.0f, 0.0f);

    // Create the circle base vertices
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        float theta = 2.0f * M_PI * i / NUM_SEGMENTS;
        float x = RADIUS * cos(theta);
        float z = RADIUS * sin(theta);

        // Vertex position
        vertices[i + 2] = glm::vec3(x, 0.0f, z);

        // Texture coordinate
        texCoords[i + 2] = glm::vec2(cos(theta) * 0.5f + 0.5f, sin(theta) * 0.5f + 0.5f);

        // Normal for the base (pointing down)
        normals[i + 2] = glm::vec3(0.0f, -1.0f, 0.0f);

        // Create indices for the base triangles
        indices.push_back(1);  // Center of base
        indices.push_back(2 + i);
        indices.push_back(2 + (i + 1) % NUM_SEGMENTS);

        // Create indices for the side triangles
        indices.push_back(0);  // Apex
        indices.push_back(2 + i);
        indices.push_back(2 + (i + 1) % NUM_SEGMENTS);
    }

    // Calculate normals for the sides of the cone
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        int idx = i + 2;
        glm::vec3 sideVector = vertices[idx] - vertices[0];
        glm::vec3 nextSideVector = vertices[2 + (i + 1) % NUM_SEGMENTS] - vertices[0];

        // Cross product for the normal
        glm::vec3 normalVector = glm::normalize(glm::cross(nextSideVector, sideVector));

        // Update the normal for this vertex
        normals[idx] = normalVector;
    }

    // Upload vertex data to GPU
    for (int i = 0; i < NUM_VERTICES; i++) {
        m_vbo.AddVertexData(&vertices[i], sizeof(glm::vec3));
        m_vbo.AddVertexData(&texCoords[i], sizeof(glm::vec2));
        m_vbo.AddVertexData(&normals[i], sizeof(glm::vec3));
    }

    // Upload index data to GPU
    for (size_t i = 0; i < indices.size(); i++) {
        m_vbo.AddIndexData(&indices[i], sizeof(unsigned int));
    }

    m_vbo.UploadDataToGPU(GL_STATIC_DRAW);

    GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
    m_numIndices = static_cast<GLsizei>(indices.size());
}

void CCone::Render()
{
    glBindVertexArray(m_vao);
    m_texture.Bind();
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
}

// Release memory on the GPU
void CCone::Release()
{
    m_texture.Release();
    glDeleteVertexArrays(1, &m_vao);
    m_vbo.Release();
}