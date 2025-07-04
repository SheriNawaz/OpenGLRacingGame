#pragma once

#include "Common.h"
#include "GameWindow.h"
#include <math.h>

// Classes used in game.  For a new class, declare it here and provide a pointer to an object of this class below.  Then, in Game.cpp, 
// include the header.  In the Game constructor, set the pointer to NULL and in Game::Initialise, create a new object.  Don't forget to 
// delete the object in the destructor.   
class CCamera;
class CSkybox;
class CShader;
class CShaderProgram;
class CPlane;
class CFreeTypeFont;
class CHighResolutionTimer;
class CSphere;
class COpenAssetImportMesh;
class CAudio;
class CCatmullRom;
class CCoin;
class CTyre;

class Game {
private:
	// Three main methods used in the game.  Initialise runs once, while Update and Render run repeatedly in the game loop.
	void Initialise();
	void Update();
	void HandleCameraAngles(glm::vec3& T, glm::vec3& B);
	void HandleCameraShake(glm::vec3& T, glm::vec3& B);
	void StartCameraShake();
	void RenderCoinsAlongTrack();
	void RenderTyresAlongTrack();
	void Render();

	// Pointers to game objects.  They will get allocated in Game::Initialise()
	CSkybox* m_pSkybox;
	CCamera* m_pCamera;
	vector <CShaderProgram*>* m_pShaderPrograms;
	CPlane* m_pPlanarTerrain;
	CFreeTypeFont* m_pFtFont;
	COpenAssetImportMesh* m_pBarrelMesh;
	COpenAssetImportMesh* m_pHorseMesh;
	COpenAssetImportMesh* m_pCarMesh;
	COpenAssetImportMesh* m_pLightMesh;
	CSphere* m_pSphere;
	CHighResolutionTimer* m_pHighResolutionTimer;
	CAudio* m_pAudio;
	CCatmullRom* m_pCatmullRom;
	CCoin* m_pCoin;
	CTyre* m_pTyre;

	// Some other member variables
	double m_dt;
	int m_framesPerSecond;
	bool m_appActive;
	glm::vec3 m_carPosition;

	bool m_freeLook;
	bool m_topView;
	bool m_firstPerson;
	bool m_thirdPerson;

	bool m_turnRight;
	bool m_turnLeft;
	bool m_accelerating;
	bool m_decelerating;
	bool m_breaking;

	float m_carSpeed;
	float m_carRotation;
	float m_maxSpeed;
	float m_acceleration;
	float m_deceleration;
	float m_turnSpeed;
	float m_breakSpeed;

	float m_currentDistance;
	float m_sidePosition;
	float m_furthestSidePosition;
	std::vector<glm::vec3> m_coinPositions;
	std::vector<bool> m_coinCollected;

	std::vector<glm::vec3> m_tyrePositions;
	std::vector<bool> m_tyreHit;

	std::vector<glm::vec3> m_lightPositions;
	bool m_lightsFlickering;
	float m_lightFlickerRate;

	float m_collisionCooldown;
	int m_score;
	int m_lives;
	bool m_gameOver;

	void CheckCollision();
	void RenderLightMeshesAlongTrack();

	bool m_cameraShaking;
	float m_shakeTime;
	float m_shakeDuration;
	float m_shakeIntensity;

public:
	Game();
	~Game();
	static Game& GetInstance();
	LRESULT ProcessEvents(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
	void SetHinstance(HINSTANCE hinstance);
	WPARAM Execute();

private:
	static const int FPS = 60;
	void DisplayFrameRate();
	void GameLoop();
	GameWindow m_gameWindow;
	HINSTANCE m_hInstance;
	int m_frameCount;
	double m_elapsedTime;


};
