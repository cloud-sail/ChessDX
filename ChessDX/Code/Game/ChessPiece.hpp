#pragma once
#include "Game/ChessObject.hpp"


class ChessBoard;
enum class ChessMoveResult;

class ChessPiece : public ChessObject
{
public:
	ChessPiece(ChessMatch* match, ChessBoard* board, PieceType type, PlayerSide side, Vec3 position);
	virtual ~ChessPiece();

	virtual void Update(float deltaSeconds) override;
	virtual void Render(GameObjectRenderConfig const& config = GameObjectRenderConfig()) const override;
	
	void UpdatePosition();

	void SetAnimation(Vec3 const& startPos, Vec3 const& endPos, bool isJumping);

	ChessMoveResult TryToMove(IntVec2 fromCoords, IntVec2 toCoords, PieceType promotionType = PieceType::UNKNOWN);

	void GetZCylinderCollider(Vec2& out_centerXY, FloatRange& out_minMaxZ, float& out_radiusXY) const;

public:
	ChessPieceDefinition const* m_definition = nullptr;
	ChessBoard* m_board = nullptr;
	PlayerSide m_playerSide = PlayerSide::PLAYER_UNKNOWN;


	int m_turnLastMoved = -99;

	// Animation
	Vec3 m_startPosition;
	Vec3 m_endPosition;
	float m_secondsSinceMoved = 0.f;
	bool m_isJumping = false; // Animation type

private:
	bool IsPathBlocked(IntVec2 fromCoords, IntVec2 toCoords);
	void LoadByType(PieceType type);
	PlayerSide GetOpponentPlayerSide() const;
};

