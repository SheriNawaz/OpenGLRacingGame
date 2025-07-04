/*
OpenGL Template for INM376 / IN3005
City University London, School of Mathematics, Computer Science and Engineering
Source code drawn from a number of sources and examples, including contributions from
 - Ben Humphrey (gametutorials.com), Michal Bubner (mbsoftworks.sk), Christophe Riccio (glm.g-truc.net)
 - Christy Quinn, Sam Kellett and others

 For educational use by Department of Computer Science, City University London UK.

 This template contains a skybox, simple terrain, camera, lighting, shaders, texturing

 Potential ways to modify the code:  Add new geometry types, shaders, change the terrain, load new meshes, change the lighting,
 different camera controls, different shaders, etc.

 Template version 4.0a 30/01/2016
 Dr Greg Slabaugh (gregory.slabaugh.1@city.ac.uk)
*/


#include "game.h"


// Setup includes
#include "HighResolutionTimer.h"
#include "GameWindow.h"

// Game includes
#include "Camera.h"
#include "Skybox.h"
#include "Plane.h"
#include "Shaders.h"
#include "FreeTypeFont.h"
#include "MatrixStack.h"
#include "OpenAssetImportMesh.h"
#include "Audio.h"
#include "CatmullRom.h"
#include "Coin.h"
#include "Tyre.h"

// Constructor
Game::Game()
{
	m_pSkybox = NULL;
	m_pCamera = NULL;
	m_pShaderPrograms = NULL;
	m_pPlanarTerrain = NULL;
	m_pFtFont = NULL;
	m_pHighResolutionTimer = NULL;
	m_pAudio = NULL;
	m_pCatmullRom = NULL;
	m_pCarMesh = NULL;
	m_pLightMesh = NULL;
	m_pCoin = NULL;
	m_pTyre = NULL;

	m_carPosition = glm::vec3(15, 1, 100);
	m_dt = 0.0;
	m_framesPerSecond = 0;
	m_frameCount = 0;
	m_elapsedTime = 0.0f;
	m_freeLook = false;
	m_topView = false;
	m_firstPerson = false;
	m_thirdPerson = true;

	m_turnRight = false;
	m_turnLeft = false;
	m_accelerating = false;
	m_decelerating = false;
	m_breaking = false;

	m_carSpeed = 0.0f;
	m_carRotation = 0.0f;
	m_maxSpeed = 100.0f;
	m_acceleration = 10;
	m_deceleration = 20;
	m_turnSpeed = 0.75f;
	m_breakSpeed = 50;

	m_currentDistance = 0.0f;
	m_sidePosition = 0.0f;
	m_furthestSidePosition = 0.8f;

	m_collisionCooldown = 0.0f;
	m_score = 0;
	m_lives = 5;
	m_gameOver = false;

	m_lightsFlickering = true;
	m_lightFlickerRate = 2.0f;

	m_cameraShaking = false;
	m_shakeTime = 0.0f;
	m_shakeDuration = 0.5f;
	m_shakeIntensity = 0.5f;
}

// Destructor
Game::~Game()
{
	//game objects
	delete m_pCamera;
	delete m_pSkybox;
	delete m_pPlanarTerrain;
	delete m_pFtFont;
	delete m_pAudio;
	delete m_pCatmullRom;
	delete m_pCarMesh;
	delete m_pLightMesh;
	delete m_pCoin;
	delete m_pTyre;

	if (m_pShaderPrograms != NULL) {
		for (unsigned int i = 0; i < m_pShaderPrograms->size(); i++)
			delete (*m_pShaderPrograms)[i];
	}
	delete m_pShaderPrograms;

	//setup objects
	delete m_pHighResolutionTimer;
}

// Initialisation:  This method only runs once at startup
void Game::Initialise()
{
	// Set the clear colour and depth
	glClearColor(0.02f, 0.02f, 0.04f, 0.5f);
	glClearDepth(1.0f);

	/// Create objects
	m_pCamera = new CCamera;
	m_pSkybox = new CSkybox;
	m_pShaderPrograms = new vector <CShaderProgram*>;
	m_pPlanarTerrain = new CPlane;
	m_pFtFont = new CFreeTypeFont;
	m_pAudio = new CAudio;
	m_pCatmullRom = new CCatmullRom;
	m_pCarMesh = new COpenAssetImportMesh;
	m_pLightMesh = new COpenAssetImportMesh;
	m_pCoin = new CCoin;
	m_pTyre = new CTyre;

	RECT dimensions = m_gameWindow.GetDimensions();

	int width = dimensions.right - dimensions.left;
	int height = dimensions.bottom - dimensions.top;

	// Set the orthographic and perspective projection matrices based on the image size
	m_pCamera->SetOrthographicProjectionMatrix(width, height);
	m_pCamera->SetPerspectiveProjectionMatrix(45.0f, (float)width / (float)height, 0.5f, 5000.0f);

	// Load shaders
	vector<CShader> shShaders;
	vector<string> sShaderFileNames;
	sShaderFileNames.push_back("mainShader.vert");
	sShaderFileNames.push_back("mainShader.frag");
	sShaderFileNames.push_back("textShader.vert");
	sShaderFileNames.push_back("textShader.frag");

	for (int i = 0; i < (int)sShaderFileNames.size(); i++) {
		string sExt = sShaderFileNames[i].substr((int)sShaderFileNames[i].size() - 4, 4);
		int iShaderType;
		if (sExt == "vert") iShaderType = GL_VERTEX_SHADER;
		else if (sExt == "frag") iShaderType = GL_FRAGMENT_SHADER;
		else if (sExt == "geom") iShaderType = GL_GEOMETRY_SHADER;
		else if (sExt == "tcnl") iShaderType = GL_TESS_CONTROL_SHADER;
		else iShaderType = GL_TESS_EVALUATION_SHADER;
		CShader shader;
		shader.LoadShader("resources\\shaders\\" + sShaderFileNames[i], iShaderType);
		shShaders.push_back(shader);
	}

	// Create the main shader program
	CShaderProgram* pMainProgram = new CShaderProgram;
	pMainProgram->CreateProgram();
	pMainProgram->AddShaderToProgram(&shShaders[0]);
	pMainProgram->AddShaderToProgram(&shShaders[1]);
	pMainProgram->LinkProgram();
	m_pShaderPrograms->push_back(pMainProgram);

	// Create a shader program for fonts
	CShaderProgram* pFontProgram = new CShaderProgram;
	pFontProgram->CreateProgram();
	pFontProgram->AddShaderToProgram(&shShaders[2]);
	pFontProgram->AddShaderToProgram(&shShaders[3]);
	pFontProgram->LinkProgram();
	m_pShaderPrograms->push_back(pFontProgram);

	// You can follow this pattern to load additional shaders

	// Create the skybox
	// Skybox downloaded from http://www.akimbo.in/forum/viewtopic.php?f=10&t=9
	m_pSkybox->Create(2500.0f);

	// Create the planar terrain
	m_pPlanarTerrain->Create("resources\\textures\\", "grassfloor01.jpg", 2000.0f, 2000.0f, 50.0f); // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013

	m_pFtFont->LoadSystemFont("arial.ttf", 32);
	m_pFtFont->SetShaderProgram(pFontProgram);

	m_pCarMesh->Load("resources\\models\\Car\\f360.3ds"); // Downloaded from https://www.dropbox.com/scl/fo/7xaqidzsig93run6jlhte/AAKUjeR3RlYSGw08c3cmO_s?dl=0&e=3&preview=f360.zip&rlkey=d598oryxpqykn5ix5q03g6dev From Psionic Games
	m_pLightMesh->Load("resources\\models\\Light\\light.fbx"); // Downloaded from https://sketchfab.com/3d-models/streetlight-low-poly-stylized-f9e86a00421e499bbd1017557772fb14  

	m_pCoin->Create("resources\\textures\\", "gold.png", 20, 0.5f); //Texture from https://freepbr.com/product/hammered-gold-pbr/
	m_pTyre->Create("resources\\textures\\", "tyre.png", 32, 24, 1.0f, 0.3f); // Texture from https://freepbr.com/product/textured-rubber-pbr-material/

	glEnable(GL_CULL_FACE);

	m_pCatmullRom->CreateCentreline();
	m_pCatmullRom->CreateOffsetCurves();
	m_pCatmullRom->CreateTrack("resources\\textures\\", "road.jpg"); // Texture from https://uk.pinterest.com/pin/156781630764428267/ 
}

// Render method runs repeatedly in a loop
void Game::Render()
{

	// Clear the buffers and enable depth testing (z-buffering)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Set up a matrix stack
	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	// Use the main shader program 
	CShaderProgram* pMainProgram = (*m_pShaderPrograms)[0];
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("sampler0", 0);
	pMainProgram->SetUniform("CubeMapTex", 1);


	// Set the projection matrix
	pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());

	// Call LookAt to create the view matrix and put this on the modelViewMatrix stack. 
	// Store the view matrix and the normal matrix associated with the view matrix for later (they're useful for lighting -- since lighting is done in eye coordinates)
	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());
	glm::mat4 viewMatrix = modelViewMatrixStack.Top();
	glm::mat3 viewNormalMatrix = m_pCamera->ComputeNormalMatrix(viewMatrix);
	glm::vec4 lightPosition1 = glm::vec4(-100, 100, -100, 1); // Position of light source *in world coordinates*
	pMainProgram->SetUniform("light1.position", viewMatrix * lightPosition1); // Position of light source *in eye coordinates*
	pMainProgram->SetUniform("light1.La", glm::vec3(0.06f, 0.06f, 0.08f));  // Increased ambient light for better visibility
	pMainProgram->SetUniform("light1.Ld", glm::vec3(0.0f));                 // No diffuse light
	pMainProgram->SetUniform("light1.Ls", glm::vec3(0.0f));                 // No specular
	pMainProgram->SetUniform("light1.direction", glm::vec3(0.0f, -1.0f, 0.0f));
	pMainProgram->SetUniform("light1.exponent", 1.0f);
	pMainProgram->SetUniform("light1.cutoff", 180.0f);

	// Set material properties for better ambient reflection
	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.35f));       // Higher ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.4f));
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));
	pMainProgram->SetUniform("material1.shininess", 30.0f);
	pMainProgram->SetUniform("numActiveLights", 16);


	// Render the skybox and terrain with full ambient reflectance 
	modelViewMatrixStack.Push();
	pMainProgram->SetUniform("renderSkybox", true);
	// Translate the modelview matrix to the camera eye point so skybox stays centred around camera
	glm::vec3 vEye = m_pCamera->GetPosition();
	modelViewMatrixStack.Translate(vEye);
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pSkybox->Render();
	pMainProgram->SetUniform("renderSkybox", false);
	modelViewMatrixStack.Pop();

	// Render the planar terrain
	modelViewMatrixStack.Push();
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pPlanarTerrain->Render();
	modelViewMatrixStack.Pop();


	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.8f));
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.9f));
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));
	pMainProgram->SetUniform("material1.shininess", 25.0f);

	//Render Car
	modelViewMatrixStack.Push();
	modelViewMatrixStack.Translate(m_carPosition);
	modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), m_carRotation);
	modelViewMatrixStack.RotateX(glm::radians(-90.0f));
	modelViewMatrixStack.Scale(3, 3, 3);
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pCarMesh->Render();
	modelViewMatrixStack.Pop();

	RenderCoinsAlongTrack();
	RenderTyresAlongTrack();
	RenderLightMeshesAlongTrack();

	//Render Track
	modelViewMatrixStack.Push();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
	m_pCatmullRom->RenderTrack();
	modelViewMatrixStack.Pop();

	// Draw the 2D graphics after the 3D graphics
	DisplayFrameRate();

	// Swap buffers to show the rendered image
	SwapBuffers(m_gameWindow.Hdc());

}

void Game::Update()
{
	//Player inputs change booleans that correspond to a state of what kind of movement the car is performing
	if (m_accelerating && !m_breaking) {
		m_carSpeed += m_acceleration * (m_dt / 1000.0f); //Accelerate
		if (m_carSpeed > m_maxSpeed)
			m_carSpeed = m_maxSpeed;
	}
	else if (m_decelerating && !m_breaking) {
		m_carSpeed -= m_deceleration * (m_dt / 1000.0f); //Slow down at a different rate when decelerating
		if (m_carSpeed < 0.0f)
			m_carSpeed = 0.0f;
	}
	else if (m_breaking) {
		m_carSpeed -= m_breakSpeed * (m_dt / 1000.0f); //Slow down when breaking
		if (m_carSpeed < 0.0f)
			m_carSpeed = 0.0f;
	}

	m_currentDistance += m_carSpeed * (m_dt / 1000.0f); //Update cars distance along the spline using speed of the car

	//Turn left or right using turn speed
	if (m_turnLeft) {
		m_sidePosition -= m_turnSpeed * (m_dt / 1000.0f);
	}
	else if (m_turnRight) {
		m_sidePosition += m_turnSpeed * (m_dt / 1000.0f);
	}

	//Confine the cars side to side position within the track
	if (m_sidePosition < -m_furthestSidePosition)
		m_sidePosition = -m_furthestSidePosition;
	if (m_sidePosition > m_furthestSidePosition)
		m_sidePosition = m_furthestSidePosition;

	//Get position on track
	glm::vec3 centrelinePos;
	m_pCatmullRom->Sample(m_currentDistance, centrelinePos);

	//Get position slightly ahead of track
	glm::vec3 nextPos;
	m_pCatmullRom->Sample(m_currentDistance + 0.1f, nextPos);

	glm::vec3 T = glm::normalize(nextPos - centrelinePos); //Calculate tangent vector along track
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 N = glm::normalize(glm::cross(T, worldUp)); //Calculate normal vector perpendicular to track

	glm::vec3 B = glm::normalize(glm::cross(N, T)); //Calculate biniormal vector

	float trackWidth = 50.0f;
	m_carPosition = centrelinePos + N * (trackWidth * 0.5f * m_sidePosition);
	m_carPosition.y = centrelinePos.y + 0.5f;

	//Rotate car to face the right way on the track
	m_carRotation = atan2(-T.x, -T.z);

	HandleCameraAngles(T, B);
	HandleCameraShake(T, B);

	if (!m_gameOver) {
		CheckCollision();
	}

	if (m_lives <= 0) {
		m_gameOver = true;
	}

	if (m_collisionCooldown > 0.0f) { //Reduce collision cooldown to 0 to allow for collisions with a tyre after another time. i.e. invulnerability period when hitting tyre
		m_collisionCooldown -= m_dt / 1000.0f;
	}

	m_pAudio->Update();
}

void Game::HandleCameraAngles(glm::vec3& T, glm::vec3& B)
{
	if (m_thirdPerson) {
		glm::vec3 cameraOffset = -25.0f * T + glm::vec3(0.0f, 8.0f, 0.0f);
		glm::vec3 cameraPos = m_carPosition + cameraOffset;
		m_pCamera->Set(cameraPos, m_carPosition, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else if (m_firstPerson) {
		glm::vec3 cameraPos = m_carPosition + B * 2.0f;
		glm::vec3 lookAt = m_carPosition + 20.0f * T;
		m_pCamera->Set(cameraPos, lookAt, B);
	}
	else if (m_topView) {
		glm::vec3 cameraPos = m_carPosition + glm::vec3(0.0f, 60.0f, 0.0f);
		m_pCamera->Set(cameraPos, m_carPosition, -T);
	}
	else if (m_freeLook) {
		m_pCamera->Update(m_dt);
	}
}

void Game::HandleCameraShake(glm::vec3& T, glm::vec3& B)
{
	if (m_cameraShaking) {
		m_shakeTime += m_dt / 1000.0f; //Begin timer for how long camera shakes

		if (m_shakeTime < m_shakeDuration) {
			if (!m_freeLook) {
				//Apply a shake factor with a random offset on every angle
				float shakeFactor = m_shakeIntensity * (1.0f - (m_shakeTime / m_shakeDuration));

				float offsetX = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * shakeFactor;
				float offsetY = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * shakeFactor;
				float offsetZ = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * shakeFactor;

				glm::vec3 shakeOffset = glm::vec3(offsetX, offsetY, offsetZ);

				//Apply the shake effect
				if (m_thirdPerson) {
					glm::vec3 cameraOffset = -25.0f * T + glm::vec3(0.0f, 8.0f, 0.0f);
					glm::vec3 cameraPos = m_carPosition + cameraOffset + shakeOffset;
					m_pCamera->Set(cameraPos, m_carPosition, glm::vec3(0.0f, 1.0f, 0.0f));
				}
				else if (m_firstPerson) {
					glm::vec3 cameraPos = m_carPosition + B * 2.0f + shakeOffset;
					glm::vec3 lookAt = m_carPosition + 20.0f * T;
					m_pCamera->Set(cameraPos, lookAt, B);
				}
				else if (m_topView) {
					glm::vec3 cameraPos = m_carPosition + glm::vec3(0.0f, 60.0f, 0.0f) + shakeOffset;
					m_pCamera->Set(cameraPos, m_carPosition, -T);
				}
			}
		}
		else {
			m_cameraShaking = false;
		}
	}
}

void Game::CheckCollision()
{
	float collisionDistance = 15.0f;

	for (size_t i = 0; i < m_coinPositions.size(); i++) {
		if (!m_coinCollected[i]) {
			float distance = glm::distance(m_carPosition, m_coinPositions[i]);
			//If the distance between car and a coin is within a range, use that as a collision. Mark the coin as collected and increase score 

			if (distance < collisionDistance) {
				m_coinCollected[i] = true;
				m_score += 100;
			}
		}
	}

	float tyreCollisionDistance = 15.0f;
	for (size_t i = 0; i < m_tyrePositions.size(); i++) {
		float distance = glm::distance(m_carPosition, m_tyrePositions[i]);
		//If the distance between car and a tyre is within a range, use that as a collision. Begin a timer to prevent lives instantly going down to 0 

		if (distance < tyreCollisionDistance && m_collisionCooldown <= 0.0f) {
			m_lives -= 1;
			m_collisionCooldown = 1.0f;
			m_cameraShaking = true;
			m_shakeTime = 0.0f;
		}
	}
}

void Game::RenderCoinsAlongTrack()
{
	CShaderProgram* pMainProgram = (*m_pShaderPrograms)[0];

	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());

	// Give coins a gold tint
	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.7f, 0.6f, 0.2f));
	pMainProgram->SetUniform("material1.Md", glm::vec3(1.0f, 0.8f, 0.2f));
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f, 0.9f, 0.6f));
	pMainProgram->SetUniform("material1.shininess", 120.0f);

	float trackLength = m_pCatmullRom->GetTrackLength();
	float coinSpacing = 15.0f;
	int numCoins = static_cast<int>(trackLength / coinSpacing);

	float maxLateralOffset = 0.7f;
	float trackWidth = 50.0f;

	// Initialize the coin positions. Used for collision detection
	if (m_coinPositions.empty()) {
		m_coinPositions.resize(numCoins);
		m_coinCollected.resize(numCoins, false);
	}

	for (int i = 0; i < numCoins; i++) {
		//Don't render coin if its collected
		if (m_coinCollected.size() > i && m_coinCollected[i]) {
			continue;
		}

		float distance = i * coinSpacing;
		glm::vec3 coinPosition;
		glm::vec3 up;

		if (m_pCatmullRom->Sample(distance, coinPosition, up)) {

			//Apply a zigzag spread to the coins using sin
			float zigzagFactor = sin(i * 0.5f);
			float lateralOffset = maxLateralOffset * zigzagFactor;

			glm::vec3 nextPos;
			if (m_pCatmullRom->Sample(distance + 0.1f, nextPos)) {
				glm::vec3 T = glm::normalize(nextPos - coinPosition);
				glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 N = glm::normalize(glm::cross(T, worldUp));

				coinPosition += N * (trackWidth * 0.5f * lateralOffset);
			}

			// Raise coins slightly higher to be more visible
			coinPosition.y += 2.5f;

			// Store the coin position for collision detection
			if (m_coinPositions.size() > i) {
				m_coinPositions[i] = coinPosition;
			}

			modelViewMatrixStack.Push();
			modelViewMatrixStack.Translate(coinPosition);

			// Spin animation for coins
			float spinSpeed = 250.0f;
			float spinAngle = fmod(spinSpeed * m_elapsedTime / 1000.0f, 360.0f);
			modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(spinAngle));

			// Wobble animation for coins
			float wobbleAmount = 10.0f;
			float wobbleSpeed = 3.0f;
			float wobbleAngle = glm::radians(wobbleAmount * sin(wobbleSpeed * m_elapsedTime / 1000.0f));
			modelViewMatrixStack.Rotate(glm::vec3(1.0f, 0.0f, 0.0f), wobbleAngle);

			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

			m_pCoin->Render();

			modelViewMatrixStack.Pop();
		}
	}
}

void Game::RenderTyresAlongTrack()
{
	//Almost identical logic to rendering coins
	CShaderProgram* pMainProgram = (*m_pShaderPrograms)[0];

	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());

	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.2f));
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.6f, 0.6f, 0.6f));
	pMainProgram->SetUniform("material1.Ms", glm::vec3(0.4f));
	pMainProgram->SetUniform("material1.shininess", 10.0f);

	float trackLength = m_pCatmullRom->GetTrackLength();
	float tyreSpacing = 100.0f;
	int numTyres = static_cast<int>(trackLength / tyreSpacing);

	float maxLateralOffset = 0.7f;
	float trackWidth = 50.0f;

	if (m_tyrePositions.empty()) {
		m_tyrePositions.resize(numTyres);
	}

	for (int i = 0; i < numTyres; i++) {
		float distance = i * tyreSpacing + (tyreSpacing / 2.0f); // Offset from coins
		glm::vec3 tyrePosition;
		glm::vec3 up;

		if (m_pCatmullRom->Sample(distance, tyrePosition, up)) {
			float zigzagFactor = sin(i * 0.5f);
			float lateralOffset = maxLateralOffset * -zigzagFactor; //Negative so its placed opposite to coins

			glm::vec3 nextPos;
			if (m_pCatmullRom->Sample(distance + 0.1f, nextPos)) {
				glm::vec3 T = glm::normalize(nextPos - tyrePosition);
				glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 N = glm::normalize(glm::cross(T, worldUp));

				tyrePosition += N * (trackWidth * 0.5f * lateralOffset);
			}

			tyrePosition.y += 1.0f; // Position on the track

			if (m_tyrePositions.size() > i) {
				m_tyrePositions[i] = tyrePosition;
			}

			modelViewMatrixStack.Push();
			modelViewMatrixStack.Translate(tyrePosition);

			// Rotate the tyre to face the track direction
			glm::vec3 nextPosForRotation;
			if (m_pCatmullRom->Sample(distance + 0.1f, nextPosForRotation)) {
				glm::vec3 direction = glm::normalize(nextPosForRotation - tyrePosition);
				float yaw = atan2(-direction.x, -direction.z);
				modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), yaw);

				modelViewMatrixStack.Rotate(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(5.0f));
			}

			modelViewMatrixStack.Scale(6.0f, 6.0f, 6.0f);

			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

			m_pTyre->Render();

			modelViewMatrixStack.Pop();
		}
	}
}

void Game::RenderLightMeshesAlongTrack()
{
	CShaderProgram* pMainProgram = (*m_pShaderPrograms)[0];

	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());
	glm::mat4 viewMatrix = modelViewMatrixStack.Top();
	glm::mat3 viewNormalMatrix = m_pCamera->ComputeNormalMatrix(viewMatrix);

	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.8f));
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.8f));
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));
	pMainProgram->SetUniform("material1.shininess", 40.0f);

	float trackLength = m_pCatmullRom->GetTrackLength();
	float lightSpacing = 30.0f; // Spacing between lights
	int numLights = static_cast<int>(trackLength / lightSpacing);

	// Limit to maximum supported lights
	if (numLights > 16) {
		lightSpacing = trackLength / 16.0f;
		numLights = 16;
	}

	float trackWidth = 50.0f;
	float lightOffset = 1.2f; // Spacing from track edge

	// Initialize light positions 
	if (m_lightPositions.empty() || m_lightPositions.size() != numLights) {
		m_lightPositions.resize(numLights);
	}

	pMainProgram->SetUniform("numActiveLights", numLights);

	// Initialise all lights to off
	for (int i = 0; i < 16; i++) {
		//Strng accesses array element in shaders and can be used to adjust the different parameters
		std::string lightIdx = "trackLights[" + std::to_string(i) + "]";
		pMainProgram->SetUniform(lightIdx + ".position", viewMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		pMainProgram->SetUniform(lightIdx + ".direction", glm::vec3(0.0f, -1.0f, 0.0f));
		pMainProgram->SetUniform(lightIdx + ".La", glm::vec3(0.0f));
		pMainProgram->SetUniform(lightIdx + ".Ld", glm::vec3(0.0f));
		pMainProgram->SetUniform(lightIdx + ".Ls", glm::vec3(0.0f));
		pMainProgram->SetUniform(lightIdx + ".exponent", 10.0f);
		pMainProgram->SetUniform(lightIdx + ".cutoff", 25.0f);
	}

	for (int i = 0; i < numLights; i++) {
		// Calculate position along track 
		float distance = i * lightSpacing;
		glm::vec3 lightPosition;
		glm::vec3 up;

		if (m_pCatmullRom->Sample(distance, lightPosition, up)) {
			// Alternate between left and right of the track
			float side = (i % 2 == 0) ? 1.0f : -1.0f;

			// Get the next point for track direction
			glm::vec3 nextPos;
			if (m_pCatmullRom->Sample(distance + 0.1f, nextPos)) {
				// Calculate track vectors
				glm::vec3 T = glm::normalize(nextPos - lightPosition);
				glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 N = glm::normalize(glm::cross(T, worldUp));

				// Position light post at the side of the track
				glm::vec3 postPosition = lightPosition + N * (trackWidth * 0.5f * lightOffset * side);
				postPosition.y += 0.5f;

				// Position spotlight at the base of the light mesh
				glm::vec3 finalLightPosition = postPosition;

				if (m_lightPositions.size() > i) {
					m_lightPositions[i] = finalLightPosition;
				}

				// Target light onto track
				glm::vec3 targetPointOnTrack = lightPosition + N * (trackWidth * 0.2f * side * -1.0f);
				targetPointOnTrack.y += 0.1f; // Just above track surface

				// Calculate direction from light to target point
				glm::vec3 lightDirection = glm::normalize(targetPointOnTrack - finalLightPosition);

				// Apply colours to lights
				glm::vec3 lightColour;
				float baseIntensity = 50.0f;
				switch (i % 4) {
				case 0: lightColour = glm::vec3(1.0f, 0.2f, 0.1f) * baseIntensity; break; // Red
				case 1: lightColour = glm::vec3(0.2f, 0.2f, 1.0f) * baseIntensity; break; // Blue
				case 2: lightColour = glm::vec3(1.0f, 0.7f, 0.1f) * baseIntensity; break; // Yellow
				case 3: lightColour = glm::vec3(0.2f, 1.0f, 0.3f) * baseIntensity; break; // Green
				}

				bool lightOn = true;

				float flickerMultiplier = 1.0f;

				//Flicker lights
				if (m_lightsFlickering) {
					float t = m_elapsedTime / 1000.0f; // Get time in seconds

					//Numbers used when multiplying are random to generate a random-looking, distributed range of values
					//This specific light has a pseudo-random value assigned to it based on current time
					float flickerChance = glm::fract(glm::sin(float(i) * 19.7f + t * 2.5f) * 57683.2f);
					//10% chance for light to flicker
					if (flickerChance < 0.1f) {
						float innerNoise = glm::fract(glm::cos(float(i) * 36.8f + t * 18.0f) * 23941.7f); //Generate random number for intensity of flicker
						flickerMultiplier = glm::mix(0.3f, 0.7f, innerNoise); //Interpolate between 0.3 and 0.7 based on inner noise. this ensures that lights don't turn off when flickering, more so dims and undims
					}
				}

				if (lightOn) {
					//String accesses array element in shaders and can be used to adjust the different parameters
					std::string lightIdx = "trackLights[" + std::to_string(i) + "]";

					pMainProgram->SetUniform(lightIdx + ".position", viewMatrix * glm::vec4(finalLightPosition, 1.0f));
					glm::vec3 lightDirEyeSpace = viewNormalMatrix * lightDirection;
					pMainProgram->SetUniform(lightIdx + ".direction", glm::normalize(lightDirEyeSpace));

					pMainProgram->SetUniform(lightIdx + ".La", glm::vec3(0.1f));
					pMainProgram->SetUniform(lightIdx + ".Ld", lightColour * flickerMultiplier); //Apply the flicking multiplier to the light which constantly changes
					pMainProgram->SetUniform(lightIdx + ".Ls", lightColour * 1.5f * flickerMultiplier);

					//Spotlight parameters
					pMainProgram->SetUniform(lightIdx + ".exponent", 0.5f);
					pMainProgram->SetUniform(lightIdx + ".cutoff", 75.0f);
				}

				modelViewMatrixStack.Push();
				modelViewMatrixStack.Translate(finalLightPosition);
				glm::vec3 forward = glm::normalize(targetPointOnTrack - finalLightPosition);

				// Face light towards track
				modelViewMatrixStack.RotateY(atan2(forward.x, forward.z));

				//Tilt light downwards
				modelViewMatrixStack.RotateX(glm::radians(-78.0f));

				modelViewMatrixStack.Scale(10.0f, 10.0f, 10.0f);

				pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
				pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

				m_pLightMesh->Render();
				modelViewMatrixStack.Pop();
			}
		}
	}
}

void Game::DisplayFrameRate()
{
	CShaderProgram* fontProgram = (*m_pShaderPrograms)[1];

	RECT dimensions = m_gameWindow.GetDimensions();
	int height = dimensions.bottom - dimensions.top;

	// Increase the elapsed time and frame counter
	m_elapsedTime += m_dt;
	m_frameCount++;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
	if (m_elapsedTime > 1000)
	{
		m_elapsedTime = 0;
		m_framesPerSecond = m_frameCount;

		// Reset the frames per second
		m_frameCount = 0;
	}

	if (m_framesPerSecond > 0) {
		// Use the font shader program and render the text
		fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		m_pFtFont->Render(20, height - 20, 20, "FPS: %d", m_framesPerSecond); //Display FPS

		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
		m_pFtFont->Render(20, height - 50, 20, "Score: %d", m_score); //Display Score
		fontProgram->SetUniform("vColour", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		m_pFtFont->Render(20, height - 80, 20, "Lives: %d", m_lives); //Display Lives
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

		m_pFtFont->Render(20, height - 110, 20, "Current Lap: %d", m_pCatmullRom->CurrentLap(m_currentDistance) + 1); //Display Current Lap
		fontProgram->SetUniform("vColour", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

		if (m_gameOver) {
			fontProgram->SetUniform("vColour", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
			m_pFtFont->Render(150, height - 20, 20, "GAME OVER");//Display game over if condition is met
		}
	}
}

// The game loop runs repeatedly until game over
void Game::GameLoop()
{
	/*
	// Fixed timer
	dDt = pHighResolutionTimer->Elapsed();
	if (dDt > 1000.0 / (double) Game::FPS) {
		pHighResolutionTimer->Start();
		Update();
		Render();
	}
	*/


	// Variable timer
	m_pHighResolutionTimer->Start();
	Update();
	Render();
	m_dt = m_pHighResolutionTimer->Elapsed();


}

WPARAM Game::Execute()
{
	m_pHighResolutionTimer = new CHighResolutionTimer;
	m_gameWindow.Init(m_hInstance);

	if (!m_gameWindow.Hdc()) {
		return 1;
	}

	Initialise();

	m_pHighResolutionTimer->Start();


	MSG msg;

	while (1) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (m_appActive) {
			GameLoop();
		}
		else Sleep(200); // Do not consume processor power if application isn't active
	}

	m_gameWindow.Deinit();

	return(msg.wParam);
}

LRESULT Game::ProcessEvents(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	LRESULT result = 0;

	switch (message) {


	case WM_ACTIVATE:
	{
		switch (LOWORD(w_param))
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			m_appActive = true;
			m_pHighResolutionTimer->Start();
			break;
		case WA_INACTIVE:
			m_appActive = false;
			break;
		}
		break;
	}

	case WM_SIZE:
		RECT dimensions;
		GetClientRect(window, &dimensions);
		m_gameWindow.SetDimensions(dimensions);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(window, &ps);
		EndPaint(window, &ps);
		break;

	case WM_KEYDOWN:
		switch (w_param) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case '2':
			m_freeLook = true;
			m_firstPerson = false;
			m_thirdPerson = false;
			m_topView = false;
			break;
		case '3':
			m_freeLook = false;
			m_firstPerson = false;
			m_thirdPerson = true;
			m_topView = false;
			break;
		case '4':
			m_freeLook = false;
			m_firstPerson = true;
			m_thirdPerson = false;
			m_topView = false;
			break;
		case '5':
			m_freeLook = false;
			m_firstPerson = false;
			m_thirdPerson = false;
			m_topView = true;
			break;
		case 'W':
			m_accelerating = true;
			break;
		case 'S':
			m_breaking = true;
			break;
		case 'A':
			m_turnLeft = true;
			m_turnRight = false;
			break;
		case 'D':
			m_turnLeft = false;
			m_turnRight = true;
			break;
		}
		break;
	case WM_KEYUP:
		switch (w_param) {
		case 'W':
			m_decelerating = true;
			m_accelerating = false;
			break;
		case 'S':
			m_breaking = false;
			break;
		case 'A':
			m_turnLeft = false;
			break;
		case 'D':
			m_turnRight = false;
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		result = DefWindowProc(window, message, w_param, l_param);
		break;
	}

	return result;
}

Game& Game::GetInstance()
{
	static Game instance;

	return instance;
}

void Game::SetHinstance(HINSTANCE hinstance)
{
	m_hInstance = hinstance;
}

LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	return Game::GetInstance().ProcessEvents(window, message, w_param, l_param);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, PSTR, int)
{
	Game& game = Game::GetInstance();
	game.SetHinstance(hinstance);

	return game.Execute();
}