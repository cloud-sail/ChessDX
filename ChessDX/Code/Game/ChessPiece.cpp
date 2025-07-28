#include "Game/ChessPiece.hpp"
#include "Game/ChessPieceDefinition.hpp"
#include "Game/Game.hpp"
#include "Game/ChessMatch.hpp"
#include "Game/ChessErrorCheck.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"

ChessPiece::ChessPiece(ChessMatch* match, ChessBoard* board, PieceType type, PlayerSide side, Vec3 position)
	: ChessObject(match)
	, m_board(board)
	, m_playerSide(side)
{
	m_position = position;
	m_startPosition = position;
	m_endPosition = position;
	m_orientation = EulerAngles(90.f + 180.f * static_cast<float>(m_playerSide), 0.f, 0.f);

	LoadByType(type);

}

ChessPiece::~ChessPiece()
{

}

void ChessPiece::Update(float deltaSeconds)
{
	m_secondsSinceMoved += deltaSeconds;

	UpdatePosition();
}

void ChessPiece::Render(GameObjectRenderConfig const& config /*= GameObjectRenderConfig()*/) const
{
	UNUSED(config);

	Rgba8 color = Rgba8::OPAQUE_WHITE;
	if (config.m_isTint)
	{
		color = Rgba8::BLUE;
	}
	if (config.m_isShining)
	{
		float currentSeconds = (float)g_theGame->m_clock->GetTotalSeconds();
		color.ScaleRGB(LinearSine(currentSeconds, 1.f) * 0.5f + 0.5f);
	}


	g_theRenderer->SetModelConstants(GetModelToWorldTransform(), color);
	g_theRenderer->BindShader(g_theGame->m_diffuseShader);
	g_theRenderer->BindTexture(m_diffuseTexture, 0);
	g_theRenderer->BindTexture(m_normalTexture, 1);
	g_theRenderer->BindTexture(m_sgeTexture, 2);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP, 0);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP, 1);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP, 2);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

	g_theRenderer->DrawIndexedVertexBuffer(m_vertexBuffer, m_indexBuffer, m_indexBuffer->GetCount());

	if (g_isDebugDraw)
	{
		g_theRenderer->BindShader(g_theGame->m_tbnShader);
		g_theRenderer->DrawIndexedVertexBuffer(m_vertexBuffer, m_indexBuffer, m_indexBuffer->GetCount());
	}

	if (config.m_isRenderingGhost)
	{
		Mat44 result = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
		result.AppendScaleNonUniform3D(m_scale);
		result.SetTranslation3D(config.m_ghostPosition);

		g_theRenderer->SetModelConstants(result, Rgba8(255, 255, 255, 128));
		g_theRenderer->BindShader(g_theGame->m_diffuseShader);
		g_theRenderer->BindTexture(m_diffuseTexture, 0);
		g_theRenderer->BindTexture(m_normalTexture, 1);
		g_theRenderer->BindTexture(m_sgeTexture, 2);
		g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP, 0);
		g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP, 1);
		g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP, 2);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

		g_theRenderer->DrawIndexedVertexBuffer(m_vertexBuffer, m_indexBuffer, m_indexBuffer->GetCount());
	}



}

void ChessPiece::UpdatePosition()
{
	if (m_secondsSinceMoved >= CHESS_MOVE_DURATION || m_turnLastMoved != m_match->GetTurnNumber() - 1)
	{
		m_position = m_endPosition;
		return;
	}

	float t = m_secondsSinceMoved / CHESS_MOVE_DURATION;
	Vec3 newPosition = Interpolate(m_startPosition, m_endPosition, SmoothStep3(t));

	if (m_isJumping)
	{
		float deltaHeight = CHESS_JUMP_HEIGHT * (1.f - (2.f * t - 1.f) * (2.f * t - 1.f));
		newPosition.z += deltaHeight;
	}

	m_position = newPosition;
}

void ChessPiece::SetAnimation(Vec3 const& startPos, Vec3 const& endPos, bool isJumping)
{
	m_isJumping = isJumping;
	m_startPosition = startPos;
	m_endPosition = endPos;
	m_secondsSinceMoved = 0.f;
}

ChessMoveResult ChessPiece::TryToMove(IntVec2 fromCoords, IntVec2 toCoords, PieceType promotionType /*= PieceType::UNKNOWN*/)
{
	if (fromCoords == toCoords)
	{
		return ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE;
	}

	int taxiCabLength = (toCoords - fromCoords).GetTaxicabLength();
	bool hasNeverMoved = m_turnLastMoved < 0;
	int forwardDirection = (m_playerSide == PlayerSide::PLAYER_WHITE) ? 1 : -1;
	int endRank = (m_playerSide == PlayerSide::PLAYER_WHITE) ? 7 : 0;
	int startRank = (m_playerSide == PlayerSide::PLAYER_WHITE) ? 0 : 7;

	bool isForward = ((toCoords.y - fromCoords.y) * forwardDirection) > 0;
	bool isAxial = ((toCoords.x - fromCoords.x) == 0) || ((toCoords.y - fromCoords.y) == 0);
	bool isDiagonal = abs(fromCoords.x - toCoords.x) == abs(fromCoords.y - toCoords.y);
	bool isBlocked = IsPathBlocked(fromCoords, toCoords);

	if (m_definition->m_type == PieceType::KING)
	{
		IntVec2 delta = toCoords - fromCoords;
		if (abs(delta.x) > 1 || abs(delta.y) > 1)
		{
			// Check castling
			if (m_turnLastMoved >= 0 || fromCoords.y != startRank)
			{
				return ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED;
			}
			if (delta.y != 0 || abs(delta.x) != 2)
			{
				return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
			}
			if (m_match->IsSquareUnderAttack(fromCoords, GetOpponentPlayerSide()))
			{
				return ChessMoveResult::INVALID_CASTLE_OUT_OF_CHECK;
			}

			IntVec2 rookCoords = IntVec2((delta.x > 0) ? 7 : 0, startRank);
			ChessPiece* rookPiece = m_match->m_piecesOnBoard[m_match->GetPieceIndexFromBoardCoords(rookCoords)];
			if (rookPiece == nullptr)
			{
				return ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED;
			}
			if (rookPiece->m_playerSide != m_playerSide || rookPiece->m_definition->m_type != PieceType::ROOK || rookPiece->m_turnLastMoved >= 0)
			{
				return ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED;
			}
			IntVec2 step = IntVec2((delta.x > 0) ? 1 : -1, 0);
			IntVec2 tempCoords = fromCoords + step;
			while (tempCoords != rookCoords)
			{
				if (m_match->IsSquareOccupied(tempCoords))
				{
					return ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED;
				}

				tempCoords = tempCoords + step;
			}

			if (m_match->IsSquareUnderAttack(fromCoords + step, GetOpponentPlayerSide()) || m_match->IsSquareUnderAttack(fromCoords + step + step, GetOpponentPlayerSide()))
			{
				return ChessMoveResult::INVALID_CASTLE_THROUGH_CHECK;
			}
			
			m_match->MovePiece(fromCoords, fromCoords + step + step);
			m_match->MovePiece(rookCoords, fromCoords + step);
			//m_turnLastMoved = m_match->GetTurnNumber();
			//rookPiece->m_turnLastMoved = m_match->GetTurnNumber();
			if (delta.x > 0)
			{
				return ChessMoveResult::VALID_CASTLE_KINGSIDE;
			}
			else
			{
				return ChessMoveResult::VALID_CASTLE_QUEENSIDE;
			}
		}
		if (m_match->IsSquareUnderAttack(toCoords, GetOpponentPlayerSide()))
		{
			return ChessMoveResult::INVALID_MOVE_ENDS_IN_CHECK;
		}
		m_match->MovePiece(fromCoords, toCoords);
		//m_turnLastMoved = m_match->GetTurnNumber();
		return ChessMoveResult::VALID_MOVE_NORMAL;
	}
	else if (m_definition->m_type == PieceType::QUEEN)
	{
		if (!isAxial && !isDiagonal)
		{
			return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		}
		if (isBlocked)
		{
			return ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
		}
		m_match->MovePiece(fromCoords, toCoords);
		//m_turnLastMoved = m_match->GetTurnNumber();
		return ChessMoveResult::VALID_MOVE_NORMAL;
	}
	else if (m_definition->m_type == PieceType::ROOK)
	{
		if (!isAxial)
		{
			return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		}
		if (isBlocked)
		{
			return ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
		}
		m_match->MovePiece(fromCoords, toCoords);
		//m_turnLastMoved = m_match->GetTurnNumber();
		return ChessMoveResult::VALID_MOVE_NORMAL;
	}
	else if (m_definition->m_type == PieceType::BISHOP)
	{
		if (!isDiagonal)
		{
			return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		}
		if (isBlocked)
		{
			return ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
		}
		m_match->MovePiece(fromCoords, toCoords);
		//m_turnLastMoved = m_match->GetTurnNumber();
		return ChessMoveResult::VALID_MOVE_NORMAL;
	}
	else if (m_definition->m_type == PieceType::KNIGHT)
	{
		if (taxiCabLength != 3)
		{
			return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		}
		m_match->MovePiece(fromCoords, toCoords);
		//m_turnLastMoved = m_match->GetTurnNumber();
		return ChessMoveResult::VALID_MOVE_NORMAL;
	}
	else if (m_definition->m_type == PieceType::PAWN)
	{
		// pawn must go forward
		if (!isForward)
		{
			return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		}
		IntVec2 delta = toCoords - fromCoords;
		int x = GetIntSign(toCoords.x - fromCoords.x);
		int y = GetIntSign(toCoords.y - fromCoords.y);
		IntVec2 step = IntVec2(x, y);
		if (delta.x == 0)
		{
			// Move Forward
			int forwardY = abs(delta.y);
			if (forwardY == 0 || forwardY > 2)
			{
				return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
			}
			if (forwardY == 2)
			{
				if (!hasNeverMoved)
				{
					return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				}

				if (m_match->IsSquareOccupied(fromCoords + step))
				{
					return ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
				}
			}
			if (m_match->IsSquareOccupied(toCoords))
			{
				return ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
			}
			// Then you can move
		}
		else
		{
			// Move Diagonal
			if (abs(delta.x) != 1 || abs(delta.y) != 1)
			{
				return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
			}
			if (!m_match->IsSquareOccupied(fromCoords + step))
			{
				// Check en passant
				int enpassantRank = (m_playerSide == PlayerSide::PLAYER_WHITE) ? 4 : 3;
				if (enpassantRank != fromCoords.y)
				{
					return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				}
				IntVec2 capturedCoords = fromCoords + IntVec2(delta.x, 0);
				ChessPiece* capturedPiece = m_match->m_piecesOnBoard[m_match->GetPieceIndexFromBoardCoords(capturedCoords)];
				if (capturedPiece == nullptr)
				{
					return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				}
				if (capturedPiece->m_definition->m_type != PieceType::PAWN || capturedPiece->m_playerSide == m_playerSide)
				{
					// not pawn, not opponent
					return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				}
				if (capturedPiece->m_turnLastMoved != (m_match->GetTurnNumber() - 1))
				{
					return ChessMoveResult::INVALID_ENPASSANT_STALE;
				}
				m_match->CapturePiece(capturedCoords);
			}

		}
		// Check Promotion type
		if (toCoords.y == endRank)
		{
			if (promotionType != PieceType::QUEEN && promotionType != PieceType::BISHOP && promotionType != PieceType::ROOK && promotionType != PieceType::KNIGHT)
			{
				return ChessMoveResult::INVALID_MOVE_PAWN_WRONG_PROMOTION;
			}
			LoadByType(promotionType);
		}

		m_match->MovePiece(fromCoords, toCoords);
		//m_turnLastMoved = m_match->GetTurnNumber();
		return ChessMoveResult::VALID_MOVE_NORMAL;

	}

	return ChessMoveResult::UNKNOWN;
}

void ChessPiece::GetZCylinderCollider(Vec2& out_centerXY, FloatRange& out_minMaxZ, float& out_radiusXY) const
{
	out_centerXY = Vec2(m_position.x, m_position.y);
	out_minMaxZ = FloatRange(m_position.z, m_position.z + m_definition->m_colliderHeight);
	out_radiusXY = m_definition->m_colliderRadius;
}

bool ChessPiece::IsPathBlocked(IntVec2 fromCoords, IntVec2 toCoords)
{
	bool isAxial = ((toCoords.x - fromCoords.x) == 0) || ((toCoords.y - fromCoords.y) == 0);
	bool isDiagonal = abs(fromCoords.x - toCoords.x) == abs(fromCoords.y - toCoords.y);
	if (!isAxial && !isDiagonal)
	{
		return false;
	}

	int x = GetIntSign(toCoords.x - fromCoords.x);
	int y = GetIntSign(toCoords.y - fromCoords.y);
	IntVec2 step = IntVec2(x, y);
	IntVec2 tempPos = fromCoords + step;
	int tries = 0;
	while (tempPos != toCoords)
	{
		if (m_match->IsSquareOccupied(tempPos))
		{
			return true;
		}

		tempPos = tempPos + step;
		tries++;
		if (tries > 15)
		{
			ERROR_AND_DIE("Infinite loops");
		}
	}
	return false;
}

void ChessPiece::LoadByType(PieceType type)
{
	m_definition = ChessPieceDefinition::GetByType(type);
	m_vertexBuffer = m_definition->m_vertexBufferByPlayer[m_playerSide];
	m_indexBuffer = m_definition->m_indexBufferByPlayer[m_playerSide];
	m_diffuseTexture = m_definition->m_diffuseTextureByPlayer[m_playerSide];
	m_normalTexture = m_definition->m_normalTextureByPlayer[m_playerSide];
	m_sgeTexture = m_definition->m_sgeTextureByPlayer[m_playerSide];
}

PlayerSide ChessPiece::GetOpponentPlayerSide() const
{
	return (m_playerSide == PlayerSide::PLAYER_WHITE) ? PlayerSide::PLAYER_BLACK : PlayerSide::PLAYER_WHITE;
}
