#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/ChessMatch.hpp"
#include "Game/ChessPieceDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
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
#include "Engine/Network/NetworkSystem.hpp"

#include "ThirdParty/imgui/imgui.h"


bool OnResizeEvent(EventArgs& args)
{
	UNUSED(args);
	g_theGame->HandleResize();
	return false;
}


//-----------------------------------------------------------------------------------------------
Game::Game()
{
	m_clock = new Clock();
	m_playerController = new Player();
	ChessPieceDefinition::InitializeDefinitions();
	
	m_diffuseShader = g_theRenderer->CreateOrGetShader(ShaderConfig("Data/Shaders/Diffuse"), VertexType::VERTEX_PCUTBN);


	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Test_StbiFlippedAndOpenGL.png");


	ShaderConfig tbnShaderConfig;
	tbnShaderConfig.m_name = "Data/Shaders/TBN";
	tbnShaderConfig.m_stages = (ShaderStage)(SHADER_STAGE_VS | SHADER_STAGE_PS | SHADER_STAGE_GS);
	//tbnShaderConfig.m_compilerType = SHADER_COMPILER_DXC;
	//tbnShaderConfig.m_shaderModelVS = "vs_6_6";
	//tbnShaderConfig.m_shaderModelPS = "ps_6_6";
	//tbnShaderConfig.m_shaderModelGS = "gs_6_6";

	m_tbnShader = g_theRenderer->CreateOrGetShader(tbnShaderConfig, VertexType::VERTEX_PCUTBN);

	ResetLighting();
	DebugDrawStartup();
	InitializeSkybox();

	CreateNewMatch();

	SubscribeEventCallbackFunction(WINDOW_RESIZE_EVENT, OnResizeEvent);
}

Game::~Game()
{
	UnsubscribeEventCallbackFunction(WINDOW_RESIZE_EVENT, OnResizeEvent);
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
	ExecuteCommandsFromNetwork();
	UpdateDeveloperCheats();
	UpdatePerFrameConstants();
	m_playerController->Update();
	m_match->Update();

	UpdateCameras();
	ShowImGuiControlPanel();
	DebugDrawUpdate();
}

void Game::Render() const
{
	g_theRenderer->BeginCamera(m_playerController->m_camera);
	//m_skybox->Render(m_playerController->m_camera);
	RenderSkybox();
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
	//SkyboxConfig skyboxConfig;
	//skyboxConfig.m_defaultRenderer = g_theRenderer;
	//skyboxConfig.m_fileName = "Data/Images/skybox_mountain.png";
	//skyboxConfig.m_type = SkyboxType::CUBE_MAP;
	//m_skybox = new Skybox(skyboxConfig);

	//-----------------------------------------------------------------------------------------------
	TextureCubeSixFacesConfig config;
	config.m_name = "Ryfjallet";
	config.m_rightImageFilePath		= "Data/Images/Ryfjallet/posx.jpg";
	config.m_leftImageFilePath		= "Data/Images/Ryfjallet/negx.jpg";
	config.m_upImageFilePath		= "Data/Images/Ryfjallet/posy.jpg";
	config.m_downImageFilePath		= "Data/Images/Ryfjallet/negy.jpg";
	config.m_forwardImageFilePath	= "Data/Images/Ryfjallet/posz.jpg";
	config.m_backwardImageFilePath	= "Data/Images/Ryfjallet/negz.jpg";

	m_skyTexture = g_theRenderer->CreateOrGetTextureCubeFromSixFaces(config);
	m_skyShader = g_theRenderer->CreateOrGetShader(ShaderConfig("Data/Shaders/Sky"), VertexType::VERTEX_PCU);
}

void Game::RenderSkybox() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB3D(verts, AABB3(Vec3(-1.f, -1.f, -1.f), Vec3(1.f, 1.f, 1.f)));

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(m_skyShader);
	g_theRenderer->BindTexture(m_skyTexture, 9);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP, 0);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->DrawVertexArray(verts);
}

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


	//-----------------------------------------------------------------------------------------------
	//DebugAddWorldWirePenumbra(Vec3(2.f, 2.f, 2.f), Vec3(1.f, 0.f, 0.f), 2.f, 0.4f, 0.f);
	// Debug Draw Light

	for (int i = 0; i < m_lightConstants.m_numLights; ++i)
	{
		Light const& light = m_lightConstants.m_lights[i];

		Rgba8 color = Rgba8(DenormalizeByte(light.m_color[0]), DenormalizeByte(light.m_color[1]), DenormalizeByte(light.m_color[2]));
		DebugAddWorldSphere(light.m_worldPosition, 0.1f, 0.f, color);

		if (light.m_outerDotThreshold > -0.99f)
		{
			DebugAddWorldWirePenumbraNoneCull(light.m_worldPosition, light.m_spotForwardNormal, light.m_outerRadius, light.m_outerDotThreshold, 0.f, color);
		}

		if (light.m_innerDotThreshold > -0.99f)
		{
			DebugAddWorldWirePenumbraNoneCull(light.m_worldPosition, light.m_spotForwardNormal, light.m_innerRadius, light.m_innerDotThreshold, 0.f, color);
		}
		else
		{
			DebugAddWorldWireSphereNoneCull(light.m_worldPosition, light.m_innerRadius, 0.f, color);
			DebugAddWorldWireSphereNoneCull(light.m_worldPosition, light.m_outerRadius, 0.f, color);
		}

	}

}

void Game::UpdatePerFrameConstants()
{
	PerFrameConstants perFrameConstants;
	IntVec2 clientDimensions = Window::s_mainWindow->GetClientDimensions();
	perFrameConstants.m_resolution = Vec3((float)clientDimensions.x, (float)clientDimensions.y, 1.f);
	perFrameConstants.m_timeSeconds = (float)Clock::GetSystemClock().GetTotalSeconds();

	g_theRenderer->SetPerFrameConstants(perFrameConstants);
}

void Game::ShowImGuiControlPanel()
{
	if (ImGui::Begin("Light Control Panel"))
	{
		if (ImGui::Button("Reset Lighting")) {
			ResetLighting();
		}
		//-----------------------------------------------------------------------------------------------
		ImGui::SeparatorText("Sun");
		ImGui::ColorEdit4("Sun Color", m_lightConstants.m_sunColor);
		static float sunDir[3] = { 1.f, 2.f, -1.f };
		if (ImGui::DragFloat3("Sun Direction", sunDir, 0.02f, -10.f, 10.f, "%.2f"))
		{
			Vec3 newSunDir = Vec3(sunDir[0], sunDir[1], sunDir[2]);
			m_lightConstants.m_sunNormal = newSunDir.GetNormalized();
		}

		//-----------------------------------------------------------------------------------------------
		ImGui::SeparatorText("Lights");
		ImGui::SliderInt("Number of Lights", &m_lightConstants.m_numLights, 0, MAX_LIGHTS);

		for (int i = 0; i < m_lightConstants.m_numLights; ++i)
		{
			ImGui::PushID(i);
			Light& light = m_lightConstants.m_lights[i];

			if (ImGui::CollapsingHeader(Stringf("Light %d", i).c_str()))
			{
				ImGui::ColorEdit4("Color", light.m_color);
				//ImGui::InputFloat3("World Position", (float*)&(light.m_worldPosition), "%.2f");
				ImGui::DragFloat3("World Position", (float*)&(light.m_worldPosition), 0.1f, -10.f, 18.f, "%.2f");


				if (ImGui::DragFloat3("Forward Normal", m_lightDirBuffer[i], 0.01f))
				{
					Vec3 newDir = Vec3(m_lightDirBuffer[i][0], m_lightDirBuffer[i][1], m_lightDirBuffer[i][2]);
					light.m_spotForwardNormal = newDir.GetNormalized();
				}

				ImGui::SliderFloat("Ambience", &light.m_ambience, 0.0f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
				ImGui::SliderFloat("Inner Radius", &light.m_innerRadius, 0.0f, 10.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
				ImGui::SliderFloat("Outer Radius", &light.m_outerRadius, light.m_innerRadius + 0.001f, 12.0f);

				ImGui::SliderFloat("Inner Penumbra Dot", &light.m_innerDotThreshold, -1.0f, 1.0f);
				ImGui::SliderFloat("Outer Penumbra Dot", &light.m_outerDotThreshold, -1.0f, 1.0f);
			}

			ImGui::PopID();
		}

	}
	ImGui::End();

	ApplyLighting();
}

void Game::ExecuteCommandsFromNetwork()
{
	std::vector<std::string> remoteCommands = g_theNetwork->RetrieveIncomingStrings();
	
	for (int i = 0; i < (int)remoteCommands.size(); ++i)
	{
		g_theDevConsole->Execute(remoteCommands[i] + " remote=true");
	}
}

void Game::ResetLighting()
{
	m_lightConstants.m_sunColor[0] = 1.f;
	m_lightConstants.m_sunColor[1] = 1.f;
	m_lightConstants.m_sunColor[2] = 1.f;
	m_lightConstants.m_sunColor[3] = 0.7f;

	m_lightConstants.m_sunNormal = Vec3(1.f, 2.f, -1.f).GetNormalized();

	m_lightConstants.m_numLights = 3;

	// Point Light
	Light pointLight;
	pointLight.SetColor(0.f, 1.f, 1.f, 0.8f);
	pointLight.m_worldPosition = Vec3(4.f, 4.f, 2.f);
	pointLight.m_innerRadius = 4.f;
	pointLight.m_outerRadius = 6.f;

	m_lightConstants.m_lights[0] = pointLight;

	Light spotLight1;
	spotLight1.SetColor(1.f, 1.f, 0.f, 0.7f);
	spotLight1.m_worldPosition = Vec3(4.f, -1.62f, 4.516f);
	spotLight1.m_spotForwardNormal = Vec3(0.f, 1.f, -1.f).GetNormalized();
	spotLight1.m_innerRadius = 5.f;
	spotLight1.m_outerRadius = 10.f;
	spotLight1.m_innerDotThreshold = 0.9f;
	spotLight1.m_outerDotThreshold = 0.7f;

	m_lightConstants.m_lights[1] = spotLight1;

	Light spotLight2;
	spotLight2.SetColor(0.514f, 0.2706f, 1.f, 0.7f);
	spotLight2.m_worldPosition = Vec3(4.f, 9.57f, 4.516f);
	spotLight2.m_spotForwardNormal = Vec3(0.f, -1.f, -1.f).GetNormalized();
	spotLight2.m_innerRadius = 5.f;
	spotLight2.m_outerRadius = 10.f;
	spotLight2.m_innerDotThreshold = 0.9f;
	spotLight2.m_outerDotThreshold = 0.7f;

	m_lightConstants.m_lights[2] = spotLight2;

	for (int i = 0; i < m_lightConstants.m_numLights; ++i) {
		m_lightDirBuffer[i][0] = m_lightConstants.m_lights[i].m_spotForwardNormal.x;
		m_lightDirBuffer[i][1] = m_lightConstants.m_lights[i].m_spotForwardNormal.y;
		m_lightDirBuffer[i][2] = m_lightConstants.m_lights[i].m_spotForwardNormal.z;
	}

	ApplyLighting();
}

void Game::ApplyLighting()
{
	g_theRenderer->SetLightConstants(m_lightConstants);
}


void Game::HandleResize()
{
	m_playerController->m_camera.SetPerspectiveView(Window::s_mainWindow->GetAspectRatio(), 60.f, 0.1f, 100.f);
}

