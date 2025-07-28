#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

class VertexBuffer;
class IndexBuffer;
class ChessMatch;

struct GameObjectRenderConfig
{
	bool m_isShining = false;
	bool m_isTint = false;
	bool m_isRenderingGhost = false;
	Vec3 m_ghostPosition;
};


class ChessObject
{
public:
	ChessObject(ChessMatch* match);
	virtual ~ChessObject() = default;

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render(GameObjectRenderConfig const& config = GameObjectRenderConfig()) const = 0;
	virtual Mat44 GetModelToWorldTransform() const;

public:
	ChessMatch* m_match = nullptr;

	Vec3 m_position;
	Vec3 m_scale = Vec3(1.f, 1.f, 1.f);
	EulerAngles m_orientation;
	Rgba8 m_color = Rgba8::OPAQUE_WHITE;

	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;

	Texture const* m_diffuseTexture = nullptr;
	Texture const* m_normalTexture = nullptr;
	Texture const* m_sgeTexture = nullptr;
};

