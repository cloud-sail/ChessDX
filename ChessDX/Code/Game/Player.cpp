#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/ChessMatch.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"

Player::Player()
{
	m_position = Vec3(-2.f, 0.f, 1.f);

	float aspect = g_gameConfigBlackboard.GetValue("windowAspect", 1.777f);
	m_camera.SetPerspectiveView(aspect, 60.f, 0.1f, 100.0f);
	m_camera.SetCameraToRenderTransform(Mat44::DIRECTX_C2R);
	m_reticleTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Reticle.png");
}

void Player::Update()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F))
	{
		m_isSpectatorMode = !m_isSpectatorMode;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_G))
	{
		m_isPresentingMode = !m_isPresentingMode;
	}


	if (m_isSpectatorMode)
	{
		UpdateSpectatorMode();
	}
	else
	{
		UpdatePlayerMode();
	}


	UpdateCamera();
}

void Player::RenderScreen() const
{
	if (m_isSpectatorMode && !m_isPresentingMode)
	{
		AABB2 reticleBounds = AABB2(Vec2::ZERO, Vec2(32.f, 32.f));
		reticleBounds.SetCenter(Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) * 0.5f);
		std::vector<Vertex_PCU> reticleVerts;
		AddVertsForAABB2D(reticleVerts, reticleBounds, Rgba8::OPAQUE_WHITE);

		g_theRenderer->BindTexture(m_reticleTexture);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->DrawVertexArray(reticleVerts);
	}
}

bool Player::IsSpectatorMode() const
{
	return m_isSpectatorMode;
}

bool Player::IsPresentingMode() const
{
	return m_isPresentingMode;
}

void Player::UpdatePlayerMode()
{
	ChessMatch* match = g_theGame->GetMatch();
	if (match == nullptr) return;

	if (IsPlayingLocally())
	{
		if (match->m_currentState == MatchState::WHITE_MOVE)
		{
			m_position = Vec3(4.f, -1.62f, 4.516f);
			m_orientation = EulerAngles(90.f, 44.875f, 0.f);
		}
		else if (match->m_currentState == MatchState::BLACK_MOVE)
		{
			m_position = Vec3(4.f, 9.57f, 4.516f);
			m_orientation = EulerAngles(-90.f, 44.875f, 0.f);
		}
		else
		{
			m_position = Vec3(4.f, 1.f, 7.25f);
			m_orientation = EulerAngles(90.f, 70.75f, 0.f);
		}
	}
	else
	{
		if (match->m_localPlayerSide == PLAYER_WHITE)
		{
			m_position = Vec3(4.f, -1.62f, 4.516f);
			m_orientation = EulerAngles(90.f, 44.875f, 0.f);
		}
		else if (match->m_localPlayerSide == PLAYER_BLACK)
		{
			m_position = Vec3(4.f, 9.57f, 4.516f);
			m_orientation = EulerAngles(-90.f, 44.875f, 0.f);
		}
		else
		{
			m_position = Vec3(4.f, 1.f, 7.25f);
			m_orientation = EulerAngles(90.f, 70.75f, 0.f);
		}
	}
}

void Player::UpdateSpectatorMode()
{
	// Yaw & Pitch
	Vec2 cursorPositionDelta = g_theInput->GetCursorClientDelta();
	float deltaYaw = -cursorPositionDelta.x * 0.125f;
	float deltaPitch = cursorPositionDelta.y * 0.125f;

	m_orientation.m_yawDegrees += deltaYaw;
	m_orientation.m_pitchDegrees += deltaPitch;

	m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -CAMERA_MAX_PITCH, CAMERA_MAX_PITCH);

	// Move
	float unscaledDeltaSeconds = static_cast<float>(Clock::GetSystemClock().GetDeltaSeconds());
	float const speedMultiplier = (g_theInput->IsKeyDown(KEYCODE_SHIFT)) ? CAMERA_SPEED_FACTOR : 1.f;

	Vec3 moveIntention;
	if (g_theInput->IsKeyDown(KEYCODE_W))
	{
		moveIntention += Vec3(1.f, 0.f, 0.f);
	}
	if (g_theInput->IsKeyDown(KEYCODE_S))
	{
		moveIntention += Vec3(-1.f, 0.f, 0.f);
	}
	if (g_theInput->IsKeyDown(KEYCODE_A))
	{
		moveIntention += Vec3(0.f, 1.f, 0.f);
	}
	if (g_theInput->IsKeyDown(KEYCODE_D))
	{
		moveIntention += Vec3(0.f, -1.f, 0.f);
	}
	moveIntention.ClampLength(1.f);

	Vec3 forwardNormal = Vec3(CosDegrees(m_orientation.m_yawDegrees), SinDegrees(m_orientation.m_yawDegrees), 0.f);
	Vec3 leftNormal = Vec3(-forwardNormal.y, forwardNormal.x, 0.f);
	m_position += (forwardNormal * moveIntention.x + leftNormal * moveIntention.y) * CAMERA_MOVE_SPEED * unscaledDeltaSeconds * speedMultiplier;

	Vec3 elevateIntention;
	if (g_theInput->IsKeyDown(KEYCODE_Q))
	{
		elevateIntention += Vec3(0.f, 0.f, -1.f);
	}
	if (g_theInput->IsKeyDown(KEYCODE_E))
	{
		elevateIntention += Vec3(0.f, 0.f, 1.f);
	}
	elevateIntention.ClampLength(1.f);

	m_position += elevateIntention * CAMERA_MOVE_SPEED * unscaledDeltaSeconds * speedMultiplier;

	if (g_isDebugDraw)
	{
		// Add camera center axis
		Vec3 forwardIBasis, leftJBasis, upKBasis;
		m_orientation.GetAsVectors_IFwd_JLeft_KUp(forwardIBasis, leftJBasis, upKBasis);
		Mat44 cameraCenterAxisTransform;
		cameraCenterAxisTransform.SetTranslation3D(m_position + 0.2f * forwardIBasis);
		DebugAddBasis(cameraCenterAxisTransform, 0.f, 0.006f, 0.0003f);
	}
}

void Player::UpdateCamera()
{
	constexpr float HEIGHT = 3.f;
	constexpr float RADIUS = 6.f;

	if (m_isPresentingMode)
	{
		float currentTime = (float)g_theGame->m_clock->GetTotalSeconds();
		float degrees = currentTime * 15.f;
		m_position = Vec3(4.f, 4.f, 0.f) + Vec3(CosDegrees(degrees)* RADIUS, SinDegrees(degrees) * RADIUS, HEIGHT);
		m_orientation = EulerAngles(180.f + degrees, Atan2Degrees(HEIGHT, RADIUS), 0.f);
		
	}
	m_camera.SetPositionAndOrientation(m_position, m_orientation);

}

void Player::GetRay(Vec3& out_rayStart, Vec3& out_rayFwdNormal, float& out_rayLength) const
{
	// #ToDo Later If using mouse we can change
	out_rayStart = m_position;
	out_rayFwdNormal = Vec3::FORWARD;
	out_rayLength = 0.f;
	if (m_isPresentingMode)
	{
		return;
	}

	if (m_isSpectatorMode)
	{
		out_rayStart = m_position;
		Mat44 orientationMat = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
		out_rayFwdNormal = orientationMat.GetIBasis3D();
		out_rayLength = 10.f;
		return;
	}

	// use mouse
	Vec2 clientUV = g_theInput->GetCursorNormalizedPosition();
	bool ok = m_camera.ScreenPointToRay(out_rayStart, out_rayFwdNormal, clientUV);
	if (ok)
	{
		out_rayLength = 10.f;
	}

}
