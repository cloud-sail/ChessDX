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
	void ShowImGuiControlPanel();
	void ExecuteCommandsFromNetwork();

	void RenderUI() const;

	void DebugDrawStartup();
	void DebugDrawUpdate();

	void InitializeSkybox();
	void RenderSkybox() const;

private:
	Camera m_screenCamera;
	Skybox* m_skybox = nullptr;

	Texture* m_skyTexture = nullptr;
	Shader* m_skyShader = nullptr;

	Player* m_playerController = nullptr;
	ChessMatch* m_match = nullptr;

	std::string m_shaderDebugText = "DebugInt=0; Lit";

//-----------------------------------------------------------------------------------------------
private:
	void UpdatePerFrameConstants();

	void ResetLighting();
	void ApplyLighting();

private:
	LightConstants m_lightConstants;
	float m_lightDirBuffer[MAX_LIGHTS][3]; // for ImGui

public:
	void HandleResize();
};
