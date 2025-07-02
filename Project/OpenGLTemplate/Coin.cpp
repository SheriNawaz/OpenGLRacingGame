#include "Common.h"
#define USE_MATH_DEFINES
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#include "Coin.h"
#include <math.h>

CCoin::CCoin()
{
}

CCoin::~CCoin()
{
}

void CCoin::Create(string a_sDirectory, string a_sFilename, int slicesIn, float thickness)
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

	float radius = 1.0f;
	float halfThickness = thickness / 2.0f;
	int vertexCount = 0;

	//Generate vertices that define the sides of the coin
	int edgeVertices = slicesIn;
	for (int i = 0; i <= edgeVertices; i++) {
		float theta = (i / (float)edgeVertices) * 2.0f * (float)M_PI;
		float x = radius * cos(theta);
		float y = radius * sin(theta);

		//Front edge vertex for top rim of coin
		glm::vec3 vFront = glm::vec3(x, y, halfThickness);

		//Texrture coordinate for front edge
		glm::vec2 tFront = glm::vec2(i / (float)edgeVertices, 0.0f);

		//Normal outwards of center
		glm::vec3 nFront = glm::normalize(glm::vec3(x, y, 0.0f));

		//Add front vertex data to VBO
		m_vbo.AddVertexData(&vFront, sizeof(glm::vec3));
		m_vbo.AddVertexData(&tFront, sizeof(glm::vec2));
		m_vbo.AddVertexData(&nFront, sizeof(glm::vec3));
		vertexCount++;

		//Add back vertext data to VBO
		glm::vec3 vBack = glm::vec3(x, y, -halfThickness);
		glm::vec2 tBack = glm::vec2(i / (float)edgeVertices, 1.0f);
		glm::vec3 nBack = glm::normalize(glm::vec3(x, y, 0.0f));
		m_vbo.AddVertexData(&vBack, sizeof(glm::vec3));
		m_vbo.AddVertexData(&tBack, sizeof(glm::vec2));
		m_vbo.AddVertexData(&nBack, sizeof(glm::vec3));
		vertexCount++;
	}

	//Create vertices for front face centre of coin
	glm::vec3 frontCentre = glm::vec3(0.0f, 0.0f, halfThickness);
	glm::vec2 frontCentreTexCoord = glm::vec2(0.5f, 0.5f);
	glm::vec3 frontNormal = glm::vec3(0.0f, 0.0f, 1.0f);
	m_vbo.AddVertexData(&frontCentre, sizeof(glm::vec3));
	m_vbo.AddVertexData(&frontCentreTexCoord, sizeof(glm::vec2));
	m_vbo.AddVertexData(&frontNormal, sizeof(glm::vec3));
	int frontCentreIndex = vertexCount++;

	//Create vertices for back face centre of coin
	glm::vec3 backCentre = glm::vec3(0.0f, 0.0f, -halfThickness);
	glm::vec2 backCentreTexCoord = glm::vec2(0.5f, 0.5f);
	glm::vec3 backNormal = glm::vec3(0.0f, 0.0f, -1.0f);
	m_vbo.AddVertexData(&backCentre, sizeof(glm::vec3));
	m_vbo.AddVertexData(&backCentreTexCoord, sizeof(glm::vec2));
	m_vbo.AddVertexData(&backNormal, sizeof(glm::vec3));
	int backCentreIndex = vertexCount++;

	//Generate vertices for front and back faces
	int faceVertices = slicesIn;
	for (int i = 0; i <= faceVertices; i++) {
		//Get angle for this position
		float theta = (i / (float)faceVertices) * 2.0f * (float)M_PI;
		float x = radius * cos(theta);
		float y = radius * sin(theta);
		//Map textures 
		float texU = (cos(theta) + 1.0f) / 2.0f;
		float texV = (sin(theta) + 1.0f) / 2.0f;

		glm::vec3 vertFront = glm::vec3(x, y, halfThickness);
		glm::vec2 texFront = glm::vec2(texU, texV);
		glm::vec3 normFront = glm::vec3(0.0f, 0.0f, 1.0f);
		m_vbo.AddVertexData(&vertFront, sizeof(glm::vec3));
		m_vbo.AddVertexData(&texFront, sizeof(glm::vec2));
		m_vbo.AddVertexData(&normFront, sizeof(glm::vec3));
		vertexCount++;

		glm::vec3 vertBack = glm::vec3(x, y, -halfThickness);
		glm::vec2 texBack = glm::vec2(texU, texV);
		glm::vec3 normBack = glm::vec3(0.0f, 0.0f, -1.0f);
		m_vbo.AddVertexData(&vertBack, sizeof(glm::vec3));
		m_vbo.AddVertexData(&texBack, sizeof(glm::vec2));
		m_vbo.AddVertexData(&normBack, sizeof(glm::vec3));
		vertexCount++;
	}
	//Generate triangles for side of coin
	m_numTriangles = 0;
	for (int i = 0; i < edgeVertices; i++) {
		unsigned int index0 = i * 2;
		unsigned int index1 = index0 + 1;
		unsigned int index2 = index0 + 2;
		unsigned int index3 = index1 + 2;

		m_vbo.AddIndexData(&index1, sizeof(unsigned int));
		m_vbo.AddIndexData(&index2, sizeof(unsigned int));
		m_vbo.AddIndexData(&index0, sizeof(unsigned int));
		m_numTriangles++;

		m_vbo.AddIndexData(&index3, sizeof(unsigned int));
		m_vbo.AddIndexData(&index2, sizeof(unsigned int));
		m_vbo.AddIndexData(&index1, sizeof(unsigned int));
		m_numTriangles++;
	}
	//Generate indices for front and back of coin
	int frontIndex = (edgeVertices + 1) * 2 + 2;
	for (int i = 0; i < faceVertices; i++) {
		unsigned int index1 = frontCentreIndex;
		unsigned int index2 = frontIndex + i * 2;
		unsigned int index3 = frontIndex + (i + 1) * 2;

		m_vbo.AddIndexData(&index1, sizeof(unsigned int));
		m_vbo.AddIndexData(&index2, sizeof(unsigned int));
		m_vbo.AddIndexData(&index3, sizeof(unsigned int));
		m_numTriangles++;
	}

	int backIndex = frontIndex + 1;
	for (int i = 0; i < faceVertices; i++) {
		unsigned int index1 = backCentreIndex;
		unsigned int index2 = backIndex + (i + 1) * 2;
		unsigned int index3 = backIndex + i * 2;

		m_vbo.AddIndexData(&index1, sizeof(unsigned int));
		m_vbo.AddIndexData(&index2, sizeof(unsigned int));
		m_vbo.AddIndexData(&index3, sizeof(unsigned int));
		m_numTriangles++;
	}

	m_vbo.UploadDataToGPU(GL_STATIC_DRAW);
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void CCoin::Render()
{
	glBindVertexArray(m_vao);
	m_texture.Bind();
	glDrawElements(GL_TRIANGLES, m_numTriangles * 3, GL_UNSIGNED_INT, 0);
}

// Release memory on the GPU 
void CCoin::Release()
{
	m_texture.Release();
	glDeleteVertexArrays(1, &m_vao);
	m_vbo.Release();
}
