#include "Common.h"
#define USE_MATH_DEFINES
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#include "Tyre.h"
#include <math.h>

CTyre::CTyre()
{
}

CTyre::~CTyre()
{
}

void CTyre::Create(string a_sDirectory, string a_sFilename, int mainSegments, int tubeSegments, float mainRadius, float tubeRadius)
{
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

	int vertexCount = 0;

	//Generate tyre verticies

	for (int i = 0; i <= mainSegments; i++) {
		//Calculate position on main circle
		float u = i / (float)mainSegments;
		float mainAngle = u * 2.0f * (float)M_PI;

		//Calculate centre of tube for tyre
		float centreX = mainRadius * cos(mainAngle);
		float centreY = 0.0f;
		float centreZ = mainRadius * sin(mainAngle);
		glm::vec3 centre(centreX, centreY, centreZ);

		//Direction from tyre centre to tube centre
		glm::vec3 toCentre = glm::normalize(glm::vec3(centreX, 0.0f, centreZ));

		//Generate vertices around tube cross-section
		for (int j = 0; j <= tubeSegments; j++) {
			float v = j / (float)tubeSegments; //Calculate position on tube circle
			float tubeAngle = v * 2.0f * (float)M_PI;

			//Off=set from tube centre
			float tubeX = cos(tubeAngle);
			float tubeY = sin(tubeAngle);

			//Calculate normals
			glm::vec3 normal = glm::vec3(tubeX * toCentre.x, tubeY, tubeX * toCentre.z);
			normal = glm::normalize(normal);

			//Final vertex positions
			glm::vec3 position = centre + tubeRadius * normal;

			glm::vec2 texCoord = glm::vec2(u, v);

			// Add the vertex data to the VBO
			m_vbo.AddVertexData(&position, sizeof(glm::vec3));
			m_vbo.AddVertexData(&texCoord, sizeof(glm::vec2));
			m_vbo.AddVertexData(&normal, sizeof(glm::vec3));

			vertexCount++;
		}
	}

	// Generate the indices for the tyre's triangles
	m_numTriangles = 0;
	int verticesPerRow = tubeSegments + 1;

	for (int i = 0; i < mainSegments; i++) {
		for (int j = 0; j < tubeSegments; j++) {
			unsigned int v0 = i * verticesPerRow + j;
			unsigned int v1 = (i + 1) * verticesPerRow + j;
			unsigned int v2 = i * verticesPerRow + (j + 1);
			unsigned int v3 = (i + 1) * verticesPerRow + (j + 1);

			//Using counter clockwise winding to calculate indices

			// First triangle 
			m_vbo.AddIndexData(&v2, sizeof(unsigned int));
			m_vbo.AddIndexData(&v1, sizeof(unsigned int));
			m_vbo.AddIndexData(&v0, sizeof(unsigned int));
			m_numTriangles++;

			// Second triangle 
			m_vbo.AddIndexData(&v2, sizeof(unsigned int));
			m_vbo.AddIndexData(&v3, sizeof(unsigned int));
			m_vbo.AddIndexData(&v1, sizeof(unsigned int));
			m_numTriangles++;
		}
	}

	m_vbo.UploadDataToGPU(GL_STATIC_DRAW);
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);

	// Set up vertex attribute pointers
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);  // Position
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));  // Texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));  // Normal
}

void CTyre::Render()
{
	glBindVertexArray(m_vao);
	m_texture.Bind();
	glDrawElements(GL_TRIANGLES, m_numTriangles * 3, GL_UNSIGNED_INT, 0);
}

void CTyre::Release()
{
	m_texture.Release();
	glDeleteVertexArrays(1, &m_vao);
	m_vbo.Release();
}