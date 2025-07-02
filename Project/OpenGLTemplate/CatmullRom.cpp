#include "CatmullRom.h"
#define _USE_MATH_DEFINES
#include <math.h>



CCatmullRom::CCatmullRom()
{
    m_vertexCount = 0;
}

CCatmullRom::~CCatmullRom()
{
}

// Perform Catmull Rom spline interpolation between four points, interpolating the space between p1 and p2
glm::vec3 CCatmullRom::Interpolate(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

    glm::vec3 a = p1;
    glm::vec3 b = 0.5f * (-p0 + p2);
    glm::vec3 c = 0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
    glm::vec3 d = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);

    return a + b * t + c * t2 + d * t3;

}

// Determine lengths along the control points, which is the set of control points forming the closed curve
void CCatmullRom::ComputeLengthsAlongControlPoints()
{
    int M = (int)m_controlPoints.size();

    float fAccumulatedLength = 0.0f;
    m_distances.push_back(fAccumulatedLength);
    for (int i = 1; i < M; i++) {
        fAccumulatedLength += glm::distance(m_controlPoints[i - 1], m_controlPoints[i]);
        m_distances.push_back(fAccumulatedLength);
    }

    // Get the distance from the last point to the first
    fAccumulatedLength += glm::distance(m_controlPoints[M - 1], m_controlPoints[0]);
    m_distances.push_back(fAccumulatedLength);
}


// Return the point (and upvector, if control upvectors provided) based on a distance d along the control polygon
bool CCatmullRom::Sample(float d, glm::vec3& p, glm::vec3& up)
{
    if (d < 0)
        return false;

    int M = (int)m_controlPoints.size();
    if (M == 0)
        return false;


    float fTotalLength = m_distances[m_distances.size() - 1];

    // The the current length along the control polygon; handle the case where we've looped around the track
    float fLength = d - (int)(d / fTotalLength) * fTotalLength;

    // Find the current segment
    int j = -1;
    for (int i = 0; i < (int)m_distances.size() - 1; i++) {
        if (fLength >= m_distances[i] && fLength < m_distances[i + 1]) {
            j = i; // found it!
            break;
        }
    }

    if (j == -1)
        return false;

    // Interpolate on current segment -- get t
    float fSegmentLength = m_distances[j + 1] - m_distances[j];
    float t = (fLength - m_distances[j]) / fSegmentLength;

    // Get the indices of the four points along the control polygon for the current segment
    int iPrev = ((j - 1) + M) % M;
    int iCur = j;
    int iNext = (j + 1) % M;
    int iNextNext = (j + 2) % M;

    // Interpolate to get the point (and upvector)
    p = Interpolate(m_controlPoints[iPrev], m_controlPoints[iCur], m_controlPoints[iNext], m_controlPoints[iNextNext], t);
    if (m_controlUpVectors.size() == m_controlPoints.size())
        up = glm::normalize(Interpolate(m_controlUpVectors[iPrev], m_controlUpVectors[iCur], m_controlUpVectors[iNext], m_controlUpVectors[iNextNext], t));

    return true;
}

// Sample a set of control points using an open Catmull-Rom spline, to produce a set of iNumSamples that are (roughly) equally spaced
void CCatmullRom::UniformlySampleControlPoints(int numSamples)
{
    glm::vec3 p, up;

    // Compute the lengths of each segment along the control polygon, and the total length
    ComputeLengthsAlongControlPoints();
    float fTotalLength = m_distances[m_distances.size() - 1];

    // The spacing will be based on the control polygon
    float fSpacing = fTotalLength / numSamples;

    // Call PointAt to sample the spline, to generate the points
    for (int i = 0; i < numSamples; i++) {
        Sample(i * fSpacing, p, up);
        m_centrelinePoints.push_back(p);
        if (m_controlUpVectors.size() > 0)
            m_centrelineUpVectors.push_back(up);

    }


    // Repeat once more for truly equidistant points
    m_controlPoints = m_centrelinePoints;
    m_controlUpVectors = m_centrelineUpVectors;
    m_centrelinePoints.clear();
    m_centrelineUpVectors.clear();
    m_distances.clear();
    ComputeLengthsAlongControlPoints();
    fTotalLength = m_distances[m_distances.size() - 1];
    fSpacing = fTotalLength / numSamples;
    for (int i = 0; i < numSamples; i++) {
        Sample(i * fSpacing, p, up);
        m_centrelinePoints.push_back(p);
        if (m_controlUpVectors.size() > 0)
            m_centrelineUpVectors.push_back(up);
    }
}

void CCatmullRom::SetControlPoints()
{
    // Set control points (m_controlPoints) here, or load from disk

    m_controlPoints.clear();

    m_controlPoints.push_back(glm::vec3(200, 1, 0));
    m_controlPoints.push_back(glm::vec3(300, 5, 60));
    m_controlPoints.push_back(glm::vec3(340, 8, 140));
    m_controlPoints.push_back(glm::vec3(320, 3, 240));
    m_controlPoints.push_back(glm::vec3(260, 1, 300));
    m_controlPoints.push_back(glm::vec3(180, 6, 320));
    m_controlPoints.push_back(glm::vec3(100, 2, 300));
    m_controlPoints.push_back(glm::vec3(0, 1, 280));
    m_controlPoints.push_back(glm::vec3(-100, 1, 280));
    m_controlPoints.push_back(glm::vec3(-200, 4, 300));
    m_controlPoints.push_back(glm::vec3(-260, 10, 240));
    m_controlPoints.push_back(glm::vec3(-280, 5, 160));
    m_controlPoints.push_back(glm::vec3(-240, 3, 80));
    m_controlPoints.push_back(glm::vec3(-260, 1, 0));
    m_controlPoints.push_back(glm::vec3(-240, 5, -80));
    m_controlPoints.push_back(glm::vec3(-200, 7, -120));
    m_controlPoints.push_back(glm::vec3(-100, 3, -140));
    m_controlPoints.push_back(glm::vec3(0, 1, -120));
    m_controlPoints.push_back(glm::vec3(40, 4, -40));
    m_controlPoints.push_back(glm::vec3(20, 6, 40));
    m_controlPoints.push_back(glm::vec3(40, 3, 120));
    m_controlPoints.push_back(glm::vec3(100, 1, 140));
    m_controlPoints.push_back(glm::vec3(140, 2, 80));
    m_controlPoints.push_back(glm::vec3(180, 1, 20));
}

void CCatmullRom::CreateCentreline()
{
    // Call Set Control Points
    SetControlPoints();

    // Call UniformlySampleControlPoints with the number of samples required
    UniformlySampleControlPoints(500);

    // Create a VAO called m_vaoCentreline and a VBO to get the points onto the graphics card
    glGenVertexArrays(1, &m_vaoCentreline);
    glBindVertexArray(m_vaoCentreline);
    CVertexBufferObject vbo;
    vbo.Create();
    vbo.Bind();

    //Default texture coordinates and normals for all points
    glm::vec2 texCoord(0.0f, 0.0f);
    glm::vec3 normal(0.0f, 1.0f, 0.0f);

    //Add all centreline points to VBO
    for (unsigned int i = 0; i < m_centrelinePoints.size(); i++) {
        glm::vec3 v = m_centrelinePoints[i];
        vbo.AddData(&v, sizeof(glm::vec3));
        vbo.AddData(&texCoord, sizeof(glm::vec2));
        vbo.AddData(&normal, sizeof(glm::vec3));
    }

    vbo.UploadDataToGPU(GL_STATIC_DRAW);
    GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
        (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void CCatmullRom::CreateOffsetCurves()
{
    float trackWidth = 50.0f;

    m_leftOffsetPoints.clear();
    m_rightOffsetPoints.clear();

    //Calculate offset points for each cenrelinePoint
    for (unsigned int i = 0; i < m_centrelinePoints.size(); i++) {
        //Current and next point
        glm::vec3 p = m_centrelinePoints[i];
        glm::vec3 pNext = m_centrelinePoints[(i + 1) % m_centrelinePoints.size()];

        glm::vec3 T = glm::normalize(pNext - p); //Tangent vector in direction to travel
        glm::vec3 N = glm::normalize(glm::vec3(-T.z, 0.0f, T.x)); // Normal vector

        //Calculate left and right offset points
        glm::vec3 l = p - (trackWidth / 2.0f) * N;
        glm::vec3 r = p + (trackWidth / 2.0f) * N;

        m_leftOffsetPoints.push_back(l);
        m_rightOffsetPoints.push_back(r);
    }

    glGenVertexArrays(1, &m_vaoLeftOffsetCurve);
    glBindVertexArray(m_vaoLeftOffsetCurve);
    CVertexBufferObject vboLeft;
    vboLeft.Create();
    vboLeft.Bind();

    glm::vec2 texCoord(0.0f, 0.0f);
    glm::vec3 normal(0.0f, 1.0f, 0.0f);


    //Add all left points to VBO
    for (unsigned int i = 0; i < m_leftOffsetPoints.size(); i++) {
        glm::vec3 v = m_leftOffsetPoints[i];
        vboLeft.AddData(&v, sizeof(glm::vec3));
        vboLeft.AddData(&texCoord, sizeof(glm::vec2));
        vboLeft.AddData(&normal, sizeof(glm::vec3));
    }

    vboLeft.UploadDataToGPU(GL_STATIC_DRAW);
    GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
        (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

    glGenVertexArrays(1, &m_vaoRightOffsetCurve);
    glBindVertexArray(m_vaoRightOffsetCurve);
    CVertexBufferObject vboRight;
    vboRight.Create();
    vboRight.Bind();

    //Add all right points to vbo
    for (unsigned int i = 0; i < m_rightOffsetPoints.size(); i++) {
        glm::vec3 v = m_rightOffsetPoints[i];
        vboRight.AddData(&v, sizeof(glm::vec3));
        vboRight.AddData(&texCoord, sizeof(glm::vec2));
        vboRight.AddData(&normal, sizeof(glm::vec3));
    }

    vboRight.UploadDataToGPU(GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
        (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void CCatmullRom::CreateTrack(string directory, string filename)
{
    //Load track texture
    m_texture.Load(directory + filename, true);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenVertexArrays(1, &m_vaoTrack);
    glBindVertexArray(m_vaoTrack);

    CVertexBufferObject vboTrack;
    vboTrack.Create();
    vboTrack.Bind();

    //Default normals pointing up
    glm::vec3 normal(0.0f, 1.0f, 0.0f);

    unsigned int numPoints = m_leftOffsetPoints.size();

    //Create triangles that connect left and right sides of the track
    for (unsigned int i = 0; i < numPoints; i++) {
        float texCoordS = (float)i / 10.0f; //Texture coordinates, texture repeats every 10 points

        unsigned int nextIndex = (i + 1) % numPoints;

        //Add left vertex
        glm::vec3 leftPoint = m_leftOffsetPoints[i];
        glm::vec2 leftTexCoord(0.0f, texCoordS);
        vboTrack.AddData(&leftPoint, sizeof(glm::vec3));
        vboTrack.AddData(&leftTexCoord, sizeof(glm::vec2));
        vboTrack.AddData(&normal, sizeof(glm::vec3));

        // Add right vertex
        glm::vec3 rightPoint = m_rightOffsetPoints[i];
        glm::vec2 rightTexCoord(1.0f, texCoordS);
        vboTrack.AddData(&rightPoint, sizeof(glm::vec3));
        vboTrack.AddData(&rightTexCoord, sizeof(glm::vec2));
        vboTrack.AddData(&normal, sizeof(glm::vec3));
    }

    //Extra pair of vertices at end to close loop
    glm::vec3 leftPoint = m_leftOffsetPoints[0];
    glm::vec2 leftTexCoord(0.0f, (float)numPoints / 10.0f);
    vboTrack.AddData(&leftPoint, sizeof(glm::vec3));
    vboTrack.AddData(&leftTexCoord, sizeof(glm::vec2));
    vboTrack.AddData(&normal, sizeof(glm::vec3));

    glm::vec3 rightPoint = m_rightOffsetPoints[0];
    glm::vec2 rightTexCoord(1.0f, (float)numPoints / 10.0f);
    vboTrack.AddData(&rightPoint, sizeof(glm::vec3));
    vboTrack.AddData(&rightTexCoord, sizeof(glm::vec2));
    vboTrack.AddData(&normal, sizeof(glm::vec3));

    m_vertexCount = 2 * numPoints + 2;

    vboTrack.UploadDataToGPU(GL_STATIC_DRAW);

    GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
        (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void CCatmullRom::RenderCentreline()
{
    // Bind the VAO m_vaoCentreline and render it
    glLineWidth(5.0f);
    glBindVertexArray(m_vaoCentreline);
    glDrawArrays(GL_POINT, 0, m_centrelinePoints.size());
    glDrawArrays(GL_LINE_LOOP, 0, m_centrelinePoints.size());

}

void CCatmullRom::RenderOffsetCurves()
{
    // Bind the VAO m_vaoLeftOffsetCurve and render it
    glLineWidth(3.0f);
    glBindVertexArray(m_vaoLeftOffsetCurve);
    glDrawArrays(GL_POINTS, 0, m_leftOffsetPoints.size());
    glDrawArrays(GL_LINE_LOOP, 0, m_leftOffsetPoints.size());

    // Bind the VAO m_vaoRightOffsetCurve and render it
    glBindVertexArray(m_vaoRightOffsetCurve);
    glDrawArrays(GL_POINTS, 0, m_rightOffsetPoints.size());
    glDrawArrays(GL_LINE_LOOP, 0, m_rightOffsetPoints.size());
}

void CCatmullRom::RenderTrack()
{
    // Bind the VAO m_vaoTrack and render it
    glBindVertexArray(m_vaoTrack);
    m_texture.Bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertexCount);
}

int CCatmullRom::CurrentLap(float d)
{
    return (int)(d / m_distances.back());
}

glm::vec3 CCatmullRom::_dummy_vector(0.0f, 0.0f, 0.0f);

float CCatmullRom::GetTrackLength()
{
    // Return the total length of the track (the last value in the distances array)
    if (m_distances.size() > 0)
        return m_distances.back();
    return 0.0f;
}