#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"

class Player
{
public:
	Player();
	~Player() = default;

	void Update();
	void RenderScreen() const;

	bool IsSpectatorMode() const;
	bool IsPresentingMode() const;
	void GetRay(Vec3& out_rayStart, Vec3& out_rayFwdNormal, float& out_rayLength) const;

protected:
	void UpdatePlayerMode();
	void UpdateSpectatorMode();

	void UpdateCamera();


public:
	Camera m_camera;

protected:
	Vec3 m_position;
	EulerAngles m_orientation;

	bool m_isSpectatorMode = false;
	bool m_isPresentingMode = false;

	Texture* m_reticleTexture = nullptr;
};

