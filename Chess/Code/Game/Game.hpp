#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"

//-----------------------------------------------------------------------------------------------
class Clock;

// ----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();
	void Update();
	void Render() const;


public:
	Clock* m_clock = nullptr;


public:
	bool IsUsingCursorPointer() const;
	void CreateNewMatch();
	ChessMatch* GetMatch() const;
	Player* GetPlayerController() const;

public:
	Shader* m_diffuseShader = nullptr;
	Texture* m_testTexture = nullptr;
	Shader* m_tbnShader = nullptr;

private:
	void UpdateCameras();
	void UpdateDeveloperCheats();
	void AdjustForPauseAndTimeDistortion();

	void RenderUI() const;

	void DebugDrawStartup();
	void DebugDrawUpdate();

	void InitializeSkybox();

	//void TestRender() const;

private:
	Camera m_screenCamera;
	Skybox* m_skybox = nullptr;
	Player* m_playerController = nullptr;
	ChessMatch* m_match = nullptr;

	std::string m_shaderDebugText = "DebugInt=0; Lit";

//-----------------------------------------------------------------------------------------------
private:
	void UpdatePerFrameConstants();

	void ResetLighting();
	void ApplyLighting();
	void UpdateLighting();

private:
	Rgba8 m_sunColor;
	Vec3 m_sunDirection;

	int m_numLights = 2;
	Light m_pointLight;
	Light m_spotLight;

};
