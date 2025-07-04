#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/ChessMatch.hpp"
#include "Game/ChessPieceDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Skybox.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Plane3.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Window/Window.hpp"

//-----------------------------------------------------------------------------------------------
Game::Game()
{
	m_clock = new Clock();
	m_playerController = new Player();
	ChessPieceDefinition::InitializeDefinitions();
	
	m_diffuseShader = g_theRenderer->CreateOrGetShader("Data/Shaders/Diffuse", VertexType::VERTEX_PCUTBN);
	m_tbnShader = g_theRenderer->CreateOrGetShader("Data/Shaders/TBN", VertexType::VERTEX_PCUTBN);
	g_theRenderer->AttachGeometryShader(m_tbnShader, "Data/Shaders/TBN");
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Test_StbiFlippedAndOpenGL.png");

	ResetLighting();
	DebugDrawStartup();
	InitializeSkybox();

	CreateNewMatch();
}

Game::~Game()
{
	ChessPieceDefinition::ClearDefinitions();
	delete m_match;
	m_match = nullptr;
}

void Game::Update()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		g_theApp->HandleQuitRequested();
	}

	UpdateDeveloperCheats();
	UpdatePerFrameConstants();
	m_playerController->Update();
	m_match->Update();


	UpdateCameras();
	UpdateLighting();
	DebugDrawUpdate();
}

void Game::Render() const
{
	g_theRenderer->BeginCamera(m_playerController->m_camera);
	m_skybox->Render(m_playerController->m_camera);
	m_match->Render();
	//TestRender();
	g_theRenderer->EndCamera(m_playerController->m_camera);
	if (g_isDebugDraw) DebugRenderWorld(m_playerController->m_camera);

	g_theRenderer->BeginCamera(m_screenCamera);
	RenderUI();
	m_playerController->RenderScreen();
	g_theRenderer->EndCamera(m_screenCamera);
	DebugRenderScreen(m_screenCamera);
}

bool Game::IsUsingCursorPointer() const
{
	return !m_playerController->IsSpectatorMode();
}

void Game::CreateNewMatch()
{
	delete m_match;
	m_match = new ChessMatch();
}

ChessMatch* Game::GetMatch() const
{
	return m_match;
}

Player* Game::GetPlayerController() const
{
	return m_playerController;
}

void Game::RenderUI() const
{
}

void Game::InitializeSkybox()
{
	SkyboxConfig skyboxConfig;
	skyboxConfig.m_defaultRenderer = g_theRenderer;
	skyboxConfig.m_fileName = "Data/Images/skybox_night.png";
	skyboxConfig.m_type = SkyboxType::CUBE_MAP;
	m_skybox = new Skybox(skyboxConfig);
}

//void Game::TestRender() const
//{
//	g_theRenderer->SetModelConstants();
//	//g_theRenderer->BindTexture(m_testTexture);
//	g_theRenderer->BindTexture(nullptr);
//	g_theRenderer->BindShader(m_diffuseShader);
//	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
//	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
//	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
//	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
//
//	ChessPieceDefinition const* pieceDef = ChessPieceDefinition::GetByType(PieceType::PAWN);
//	VertexBuffer* vertexBuffer = pieceDef->m_vertexBufferByPlayer[PLAYER_BLACK];
//	IndexBuffer* indexBuffer = pieceDef->m_indexBufferByPlayer[PLAYER_BLACK];
//
//	g_theRenderer->DrawIndexedVertexBuffer(vertexBuffer, indexBuffer, indexBuffer->GetCount());
//}

void Game::UpdateCameras()
{
	// Screen camera (for UI, HUD, Attract, etc.)
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
}


void Game::UpdateDeveloperCheats()
{
	AdjustForPauseAndTimeDistortion();
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		g_isDebugDraw = !g_isDebugDraw;
	}

	//-----------------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed('0'))
	{
		g_theRenderer->SetEngineConstants(0);
		m_shaderDebugText = "DebugInt=0; Lit";
	}
	if (g_theInput->WasKeyJustPressed('1'))
	{
		g_theRenderer->SetEngineConstants(1);
		m_shaderDebugText = "DebugInt=1; Diffuse Texel only";
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		g_theRenderer->SetEngineConstants(2);
		m_shaderDebugText = "DebugInt=2; World Normal with normal map";
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		g_theRenderer->SetEngineConstants(3);
		m_shaderDebugText = "DebugInt=3; World Normal without normal map";
	}
	if (g_theInput->WasKeyJustPressed('4'))
	{
		g_theRenderer->SetEngineConstants(4);
		m_shaderDebugText = "DebugInt=4; Specular light";
	}
	if (g_theInput->WasKeyJustPressed('5'))
	{
		g_theRenderer->SetEngineConstants(5);
		m_shaderDebugText = "DebugInt=5; Emissive";
	}
	if (g_theInput->WasKeyJustPressed('6'))
	{
		g_theRenderer->SetEngineConstants(6);
		m_shaderDebugText = "DebugInt=6; Diffuse with light";
	}
	if (g_theInput->WasKeyJustPressed('7'))
	{
		g_theRenderer->SetEngineConstants(7);
		m_shaderDebugText = "DebugInt=7; Input World Tangent";
	}
	if (g_theInput->WasKeyJustPressed('8'))
	{
		g_theRenderer->SetEngineConstants(8);
		m_shaderDebugText = "DebugInt=8; Input World Bitangent";
	}
	if (g_theInput->WasKeyJustPressed('9'))
	{
		g_theRenderer->SetEngineConstants(9);
		m_shaderDebugText = "DebugInt=9; -";
	}


}

void Game::AdjustForPauseAndTimeDistortion()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_P))
	{
		m_clock->TogglePause();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_O))
	{
		m_clock->StepSingleFrame();
	}
	
	bool isSlowMo = g_theInput->IsKeyDown(KEYCODE_T);

	m_clock->SetTimeScale(isSlowMo ? 0.1 : 1.0);
}


//-----------------------------------------------------------------------------------------------
void Game::DebugDrawStartup()
{
	DebugAddWorldBasis(Mat44(), -1.f);
}

void Game::DebugDrawUpdate()
{
	// Game Clock Data
	float totalSeconds = (float)m_clock->GetTotalSeconds();
	float frameRate = (float)m_clock->GetFrameRate();
	float timeScale = (float)m_clock->GetTimeScale();

	if (frameRate == 0.f)
	{
		DebugAddScreenText(Stringf("Time: %.2f FPS: inf Scale: %.2f", totalSeconds, timeScale),
			AABB2(Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) * 0.65f, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) * 0.99f),
			15.f, Vec2(1.f, 1.f), 0.f, 0.7f);
	}
	else
	{
		DebugAddScreenText(Stringf("Time: %.2f FPS: %8.02f Scale: %.2f", totalSeconds, frameRate, timeScale),
			AABB2(Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) * 0.65f, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) * 0.99f),
			15.f, Vec2(1.f, 1.f), 0.f, 0.7f);
	}

	// Game State
	char const* GAME_STATE_TEXT = "Present Mode(G) %s | Camera Mode(F) %s | GameState: %s";

	std::string stateStr;
	if (m_match)
	{
		switch (m_match->m_currentState)
		{
		case MatchState::DEFAULT:
			stateStr = "Default";
			break;
		case MatchState::WHITE_MOVE:
			stateStr = "White's Turn";
			break;
		case MatchState::BLACK_MOVE:
			stateStr = "Black's Turn";
			break;
		case MatchState::WHITE_WIN:
			stateStr = "White wins";
			break;
		case MatchState::BLACK_WIN:
			stateStr = "Black wins";
			break;
		default:
			break;
		}
	}

	std::string gameInfo = Stringf(GAME_STATE_TEXT, 
		(m_playerController->IsPresentingMode() ? "ON " : "OFF"),
		(m_playerController->IsSpectatorMode() ? "Free" : "Auto"),
		stateStr.c_str());
	DebugAddMessage(gameInfo, 0.f, Rgba8::GREEN);

	DebugAddMessage(m_shaderDebugText, 0.f, Rgba8::YELLOW);
}

void Game::UpdatePerFrameConstants()
{
	PerFrameConstants perFrameConstants;
	IntVec2 clientDimensions = Window::s_mainWindow->GetClientDimensions();
	perFrameConstants.m_resolution = Vec3((float)clientDimensions.x, (float)clientDimensions.y, 1.f);
	perFrameConstants.m_timeSeconds = (float)Clock::GetSystemClock().GetTotalSeconds();

	g_theRenderer->SetPerFrameConstants(perFrameConstants);
}

void Game::ResetLighting()
{
	m_sunColor = Rgba8(255, 255, 255, 128);
	m_sunDirection = Vec3(1.f, 2.f, -1.f);

	m_pointLight.SetColor(0.f, 1.f, 1.f, 0.8f);
	m_pointLight.m_worldPosition = Vec3(4.f, 4.f, 2.f);
	m_pointLight.m_innerRadius = 3.f;
	m_pointLight.m_outerRadius = 5.f;

	m_spotLight.SetColor(0.f, 1.f, 0.f, 1.f);
	m_spotLight.m_worldPosition = Vec3(6.f, 6.f, 0.5f);
	m_spotLight.m_spotForwardNormal = Vec3(-1.f, -1.f, -0.2f).GetNormalized();
	m_spotLight.m_innerRadius = 5.f;
	m_spotLight.m_outerRadius = 10.f;
	m_spotLight.m_innerDotThreshold = 0.9f;
	m_spotLight.m_outerDotThreshold = 0.7f;

	ApplyLighting();
}

void Game::ApplyLighting()
{
	LightConstants lightConstants;
	m_sunColor.GetAsFloats(lightConstants.m_sunColor);
	lightConstants.m_sunNormal = m_sunDirection.GetNormalized();
	lightConstants.m_numLights = 2;
	lightConstants.m_lights[0] = m_pointLight;
	lightConstants.m_lights[1] = m_spotLight;

	g_theRenderer->SetLightConstants(lightConstants);
}

void Game::UpdateLighting()
{
	//bool needUpdate = false;
	//if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	//{
	//	needUpdate = true;
	//	m_sunDirection.x = m_sunDirection.x - 1.f;
	//	DebugAddMessage(Stringf("Sun direction x: %.1f", m_sunDirection.x), 2.f);
	//}
	//if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	//{
	//	needUpdate = true;
	//	m_sunDirection.x = m_sunDirection.x + 1.f;
	//	DebugAddMessage(Stringf("Sun direction x: %.1f", m_sunDirection.x), 2.f);
	//}
	//if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	//{
	//	needUpdate = true;
	//	m_sunDirection.y = m_sunDirection.y - 1.f;
	//	DebugAddMessage(Stringf("Sun direction y: %.1f", m_sunDirection.y), 2.f);
	//}
	//if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	//{
	//	needUpdate = true;
	//	m_sunDirection.y = m_sunDirection.y + 1.f;
	//	DebugAddMessage(Stringf("Sun direction y: %.1f", m_sunDirection.y), 2.f);
	//}
	//if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	//{
	//	needUpdate = true;
	//	m_sunIntensity -= 0.05f;
	//	m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
	//	DebugAddMessage(Stringf("Sun intensity: %.2f", m_sunIntensity), 2.f);
	//}
	//if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	//{
	//	needUpdate = true;
	//	m_sunIntensity += 0.05f;
	//	m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
	//	DebugAddMessage(Stringf("Sun intensity: %.2f", m_sunIntensity), 2.f);
	//}
	//if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	//{
	//	needUpdate = true;
	//	m_ambientIntensity -= 0.05f;
	//	m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
	//	DebugAddMessage(Stringf("Ambient Intensity: %.2f", m_ambientIntensity), 2.f);

	//}
	//if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	//{
	//	needUpdate = true;
	//	m_ambientIntensity += 0.05f;
	//	m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
	//	DebugAddMessage(Stringf("Ambient Intensity: %.2f", m_ambientIntensity), 2.f);
	//}

	//if (needUpdate)
	//{
	//	ApplyLighting();
	//}
}
