#pragma once
#include "Common.h"
#include "Texture.h"
#include "VertexBufferObjectIndexed.h"

class CTyre
{
public:
	CTyre();
	~CTyre();

	void Create(string a_sDirectory, string a_sFilename, int mainSegments, int tubeSegments, float mainRadius, float tubeRadius);
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