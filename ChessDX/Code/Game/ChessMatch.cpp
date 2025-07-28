#include "Game/ChessMatch.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/ChessBoard.hpp"
#include "Game/ChessPiece.hpp"
#include "Game/ChessPieceDefinition.hpp"
#include "Game/ChessErrorCheck.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Plane3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Network/NetworkSystem.hpp"

#include "ThirdParty/imgui/imgui.h"


ChessMatch::ChessMatch()
{
	g_theEventSystem->SubscribeEventCallbackFunction("ChessMove", ChessMatch::Command_ChessMove);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessBegin", ChessMatch::Command_ChessBegin);
	g_theEventSystem->SubscribeEventCallbackFunction("Echo", ChessMatch::Command_Echo);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessServerInfo", ChessMatch::Command_ChessServerInfo);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessListen", ChessMatch::Command_ChessListen);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessConnect", ChessMatch::Command_ChessConnect);
	g_theEventSystem->SubscribeEventCallbackFunction("RemoteCmd", ChessMatch::Command_RemoteCmd);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessDisconnect", ChessMatch::Command_ChessDisconnect);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessPlayerInfo", ChessMatch::Command_ChessPlayerInfo);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessResign", ChessMatch::Command_ChessResign);
	InitializeBoard();
	InitializePieces();
}

ChessMatch::~ChessMatch()
{
	CleanBoardAndPieces();
	g_theEventSystem->UnsubscribeEventCallbackFunction("ChessResign", ChessMatch::Command_ChessResign);
	g_theEventSystem->UnsubscribeEventCallbackFunction("ChessPlayerInfo", ChessMatch::Command_ChessPlayerInfo);
	g_theEventSystem->UnsubscribeEventCallbackFunction("ChessDisconnect", ChessMatch::Command_ChessDisconnect);
	g_theEventSystem->UnsubscribeEventCallbackFunction("RemoteCmd", ChessMatch::Command_RemoteCmd);
	g_theEventSystem->UnsubscribeEventCallbackFunction("ChessConnect", ChessMatch::Command_ChessConnect);
	g_theEventSystem->UnsubscribeEventCallbackFunction("ChessListen", ChessMatch::Command_ChessListen);
	g_theEventSystem->UnsubscribeEventCallbackFunction("ChessServerInfo", ChessMatch::Command_ChessServerInfo);
	g_theEventSystem->UnsubscribeEventCallbackFunction("Echo", ChessMatch::Command_Echo);
	g_theEventSystem->UnsubscribeEventCallbackFunction("ChessBegin", ChessMatch::Command_ChessBegin);
	g_theEventSystem->UnsubscribeEventCallbackFunction("ChessMove", ChessMatch::Command_ChessMove);
}

void ChessMatch::Update()
{
	SwitchToNextState();
	UpdateCurrentState();
	UpdateMouseBasedPieceMovement();

	float deltaSeconds = (float)g_theGame->m_clock->GetDeltaSeconds();
	m_board->Update(deltaSeconds);

	for (ChessPiece * piece : m_piecesOnBoard)
	{
		if (piece != nullptr) piece->Update(deltaSeconds);
	}

	for (ChessPiece * piece : m_piecesCaught)
	{
		if (piece != nullptr) piece->Update(deltaSeconds);
	}

	ShowImGuiPanel();
}

void ChessMatch::Render() const
{
	m_board->Render();
	
	//for (ChessPiece const* piece : m_piecesOnBoard)
	//{
	//	if (piece != nullptr) piece->Render();
	//}

	for (int tileY = 0; tileY < 8; ++tileY)
	{
		for (int tileX = 0; tileX < 8; ++tileX)
		{
			int tileIndex = tileX + tileY * 8;
			IntVec2 tileCoords = IntVec2(tileX, tileY);
			ChessPiece const* piece = m_piecesOnBoard[tileIndex];
			if (piece == nullptr) continue;

			GameObjectRenderConfig config;
			
			if (tileCoords == m_currentImpactCoords)
			{
				config.m_isTint = true;
			}
			if (tileCoords == m_selectedCoords)
			{
				config.m_isShining = true;
				if (IsCoordsValid(m_currentImpactCoords))
				{
					config.m_isRenderingGhost = true;
					config.m_ghostPosition = GetSquareCenterFromBoardCoords(m_currentImpactCoords);
				}
			}

			piece->Render(config);

		}
	}

	if (IsCoordsValid(m_currentImpactCoords))
	{
		AABB3 currentImpactBox = AABB3(Vec3::ZERO, Vec3(1.f, 1.f, 0.05f));
		currentImpactBox.SetCenter(GetSquareCenterFromBoardCoords(m_currentImpactCoords));
		std::vector<Vertex_PCU> verts;
		AddVertsForAABB3D(verts, currentImpactBox, Rgba8::RED);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(nullptr);

		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
		g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);



		g_theRenderer->DrawVertexArray(verts);
	}



	for (ChessPiece const* piece : m_piecesCaught)
	{
		if (piece != nullptr) piece->Render();
	}
}

void ChessMatch::InitializeBoard()
{
	m_board = new ChessBoard(this);
}

void ChessMatch::InitializePieces()
{
	m_piecesOnBoard.resize(64);
	m_piecesCaught.reserve(32);


	PieceType rowA[8] = { PieceType::ROOK , PieceType::KNIGHT , PieceType::BISHOP ,PieceType::QUEEN ,PieceType::KING ,PieceType::BISHOP ,PieceType::KNIGHT ,PieceType::ROOK };
	PieceType rowB[8] = { PieceType::PAWN , PieceType::PAWN ,PieceType::PAWN ,PieceType::PAWN ,PieceType::PAWN ,PieceType::PAWN ,PieceType::PAWN ,PieceType::PAWN };

	for (int i = 0; i < 8; ++i)
	{
		m_piecesOnBoard[GetPieceIndexFromBoardCoords(IntVec2(i, 0))] =
			new ChessPiece(this, m_board, rowA[i], PLAYER_WHITE, GetSquareCenterFromBoardCoords(IntVec2(i, 0)));
	}

	for (int i = 0; i < 8; ++i)
	{
		m_piecesOnBoard[GetPieceIndexFromBoardCoords(IntVec2(i, 1))] =
			new ChessPiece(this, m_board, rowB[i], PLAYER_WHITE, GetSquareCenterFromBoardCoords(IntVec2(i, 1)));
	}

	for (int i = 0; i < 8; ++i)
	{
		m_piecesOnBoard[GetPieceIndexFromBoardCoords(IntVec2(i, 7))] =
			new ChessPiece(this, m_board, rowA[i], PLAYER_BLACK, GetSquareCenterFromBoardCoords(IntVec2(i, 7)));
	}

	for (int i = 0; i < 8; ++i)
	{
		m_piecesOnBoard[GetPieceIndexFromBoardCoords(IntVec2(i, 6))] =
			new ChessPiece(this, m_board, rowB[i], PLAYER_BLACK, GetSquareCenterFromBoardCoords(IntVec2(i, 6)));
	}
}

void ChessMatch::CleanBoardAndPieces()
{
	delete m_board;
	m_board = nullptr;

	for (ChessPiece* piece : m_piecesOnBoard)
	{
		delete piece;
	}
	m_piecesOnBoard.clear();

	for (ChessPiece* piece : m_piecesCaught)
	{
		delete piece;
	}
	m_piecesCaught.clear();
}

void ChessMatch::StartNewMatch()
{
	CleanBoardAndPieces();
	InitializeBoard();
	InitializePieces();
}

Vec3 ChessMatch::GetSquareCenterFromBoardCoords(IntVec2 const& coords)
{
	Vec3 result;
	result.x = CHESS_BOARD_SQUARE_SIZE * (static_cast<float>(coords.x) + 0.5f);
	result.y = CHESS_BOARD_SQUARE_SIZE * (static_cast<float>(coords.y) + 0.5f);
	return result;
}

STATIC std::string ChessMatch::GetNotationFromBoardCoords(IntVec2 const& coords)
{
	char first = static_cast<char>(coords.x + 'A');
	char second = static_cast<char>(coords.y + '1');
	std::string result = "";
	result += first;
	result += second;
	return result;
}

STATIC IntVec2 ChessMatch::GetBoardCoordsFromNotation(std::string const& notation)
{
	return IntVec2(notation[0] - 'A', notation[1] - '1');
}

STATIC int ChessMatch::GetPieceIndexFromBoardCoords(IntVec2 const& coords)
{
	return coords.x + coords.y * 8;
}

STATIC IntVec2 ChessMatch::GetBoardCoordsFromWorldPos(Vec3 const& worldPos)
{
	IntVec2 result;
	result.x = RoundDownToInt(worldPos.x);
	result.y = RoundDownToInt(worldPos.y);
	return result;
}

Vec3 ChessMatch::GetNextEmptyWorldPosForCaughtPiece() const
{
	int nextIndex = (int)m_piecesCaught.size();
	Vec3 const offset = Vec3(-2.f, 0.5f * CHESS_BOARD_SQUARE_SIZE, 0.f);
	float deltaY = (float)(nextIndex % 8);
	float deltaX = -(float)(nextIndex / 8);
	return offset + Vec3(CHESS_BOARD_SQUARE_SIZE * deltaX, CHESS_BOARD_SQUARE_SIZE * deltaY, 0.f);
}

char ChessMatch::GetGlyghFromBoardCoords(IntVec2 const& coords) const
{
	ChessPiece* piece = m_piecesOnBoard[GetPieceIndexFromBoardCoords(coords)];
	if (piece == nullptr)
	{
		return '.';
	}
	return piece->m_definition->m_glyph[piece->m_playerSide];
}

PlayerSide ChessMatch::GetCurrentPlayerSide() const
{
	if (m_currentState == MatchState::WHITE_MOVE) return PLAYER_WHITE;
	if (m_currentState == MatchState::BLACK_MOVE) return PLAYER_BLACK;
	return PLAYER_UNKNOWN;
}

void ChessMatch::PrintBoardState() const
{

	g_theDevConsole->AddText(DevConsole::INFO_MAJOR, "  ABCDEFGH  ");
	g_theDevConsole->AddText(DevConsole::INFO_MAJOR, " +--------+ ");

	for (int i = 0; i < 8; ++i)
	{
		std::string txt = Stringf("%d|%c%c%c%c%c%c%c%c|%d", 8 - i,
			GetGlyghFromBoardCoords(IntVec2(0, 7 - i)),
			GetGlyghFromBoardCoords(IntVec2(1, 7 - i)),
			GetGlyghFromBoardCoords(IntVec2(2, 7 - i)),
			GetGlyghFromBoardCoords(IntVec2(3, 7 - i)),
			GetGlyghFromBoardCoords(IntVec2(4, 7 - i)),
			GetGlyghFromBoardCoords(IntVec2(5, 7 - i)),
			GetGlyghFromBoardCoords(IntVec2(6, 7 - i)),
			GetGlyghFromBoardCoords(IntVec2(7, 7 - i)), 8 - i);

		g_theDevConsole->AddText(DevConsole::INFO_MAJOR, txt);
	}
	g_theDevConsole->AddText(DevConsole::INFO_MAJOR, " +--------+ ");
	g_theDevConsole->AddText(DevConsole::INFO_MAJOR, "  ABCDEFGH  ");
}

//std::string ChessMatch::TryToMoveChessPiece(IntVec2 fromCoords, IntVec2 toCoords)
//{
//	// return "";  // means successful move
//	// return "errMessage";
//	// No piece
//	// same position
//
//
//	PlayerSide currentSide = GetCurrentPlayerSide();
//	if (currentSide < 0)
//	{
//		return "Can only use ChessMove command when it is someone's turn to move.";
//	}
//
//	std::string fromNotation = GetNotationFromBoardCoords(fromCoords);
//	std::string toNotation = GetNotationFromBoardCoords(toCoords);
//	ChessPiece* pieceAtFromCoords = m_piecesOnBoard[GetPieceIndexFromBoardCoords(fromCoords)];
//	ChessPiece* pieceAtToCoords = m_piecesOnBoard[GetPieceIndexFromBoardCoords(toCoords)];
//
//	// existing piece at from coords
//	if (pieceAtFromCoords == nullptr)
//	{
//		return Stringf("There is not piece at %s!", fromNotation.c_str());
//	}
//
//	// check if moving piece from correct side
//	if (currentSide != pieceAtFromCoords->m_playerSide)
//	{
//		return Stringf("The %s at %s belongs to %s; it is currently %s's turn.", 
//			GetStringFromPieceType(pieceAtFromCoords->m_definition->m_type).c_str(),
//			fromNotation.c_str(),
//			(pieceAtFromCoords->m_playerSide == PLAYER_WHITE) ? "White" : "Black",
//			(currentSide == PLAYER_WHITE) ? "White" : "Black");
//	}
//
//	// same position
//	if (fromCoords == toCoords)
//	{
//		return Stringf("The from=%s and to=%s coordinates cannot be the same square!",
//			fromNotation.c_str(), toNotation.c_str());
//	}
//	
//	// #ToDo check moving rule
//
//	// check piece type at toCoords
//	if (pieceAtToCoords != nullptr && pieceAtToCoords->m_playerSide == pieceAtFromCoords->m_playerSide)
//	{
//		return Stringf("Cannot move to %s, since it is occupied by your own %s!", 
//			toNotation.c_str(), GetStringFromPieceType(pieceAtToCoords->m_definition->m_type).c_str());
//	}
//
//	// Can Move
//	g_theDevConsole->AddText(DevConsole::INFO_MAJOR, Stringf("%s's %s has been moved from %s to %s",
//		(currentSide == PLAYER_WHITE) ? "White" : "Black",
//		GetStringFromPieceType(pieceAtFromCoords->m_definition->m_type).c_str(),
//		fromNotation.c_str(), toNotation.c_str()));
//
//	// if no pieces at to Coords
//	if (pieceAtToCoords == nullptr)
//	{
//		// Animation
//		Vec3 startPos = GetSquareCenterFromBoardCoords(fromCoords);
//		Vec3 endPos = GetSquareCenterFromBoardCoords(toCoords);
//		pieceAtFromCoords->Move(startPos, endPos);
//		//pieceAtFromCoords->m_position = endPos;
//
//		// Logic
//		m_piecesOnBoard[GetPieceIndexFromBoardCoords(fromCoords)] = nullptr;
//		m_piecesOnBoard[GetPieceIndexFromBoardCoords(toCoords)] = pieceAtFromCoords;
//
//		// Switch State
//		SetNextState((m_currentState == MatchState::WHITE_MOVE) ? MatchState::BLACK_MOVE : MatchState::WHITE_MOVE); // Assume game is playing
//		return "";
//	}
//
//	// the remaining circumstance is capture a opponent's piece.
//
//	// Animation
//	Vec3 startPosMoved = GetSquareCenterFromBoardCoords(fromCoords);
//	Vec3 endPosMoved = GetSquareCenterFromBoardCoords(toCoords);
//	pieceAtFromCoords->Move(startPosMoved, endPosMoved);
//	//pieceAtFromCoords->m_position = endPosMoved;
//
//	Vec3 startPosCaptured = GetSquareCenterFromBoardCoords(toCoords);
//	Vec3 endPosCaptured = GetNextEmptyWorldPosForCaughtPiece();
//	pieceAtToCoords->Move(startPosCaptured, endPosCaptured);
//	//pieceAtToCoords->m_position = endPosCaptured;
//
//	// Logic
//	m_piecesCaught.push_back(pieceAtToCoords);
//	m_piecesOnBoard[GetPieceIndexFromBoardCoords(fromCoords)] = nullptr;
//	m_piecesOnBoard[GetPieceIndexFromBoardCoords(toCoords)] = pieceAtFromCoords;
//
//	// Log to DevConsole
//	g_theDevConsole->AddText(Rgba8(185, 122, 87), Stringf("%s captured %s's %s at %s",
//		(currentSide == PLAYER_WHITE) ? "White" : "Black",
//		(currentSide == PLAYER_WHITE) ? "Black" : "White",
//		GetStringFromPieceType(pieceAtToCoords->m_definition->m_type).c_str(),
//		toNotation.c_str(), toNotation.c_str()));
//
//	// Switch State
//	if (pieceAtToCoords->m_definition->m_type == PieceType::KING)
//	{
//		SetNextState((m_currentState == MatchState::WHITE_MOVE) ? MatchState::WHITE_WIN : MatchState::BLACK_WIN);
//	}
//	else
//	{
//		SetNextState((m_currentState == MatchState::WHITE_MOVE) ? MatchState::BLACK_MOVE : MatchState::WHITE_MOVE); // Assume game is playing
//	}
//
//	return "";
//}

ChessMoveResult ChessMatch::TryToMoveChessPiece(IntVec2 fromCoords, IntVec2 toCoords, bool isTeleporting, PieceType promotionType /*= PieceType::UNKNOWN*/)
{
	PlayerSide currentSide = GetCurrentPlayerSide();
	if (currentSide < 0)
	{
		return ChessMoveResult::INVALID_GAME_NOT_PLAYING;
	}

	ChessPiece* pieceAtFromCoords = m_piecesOnBoard[GetPieceIndexFromBoardCoords(fromCoords)];
	ChessPiece* pieceAtToCoords = m_piecesOnBoard[GetPieceIndexFromBoardCoords(toCoords)];

	if (pieceAtFromCoords == nullptr)
	{
		return ChessMoveResult::INVALID_MOVE_NO_PIECE;
	}

	if (currentSide != pieceAtFromCoords->m_playerSide)
	{
		return ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE;
	}

	if (fromCoords == toCoords)
	{
		return ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE;
	}

	if (pieceAtToCoords != nullptr && pieceAtToCoords->m_playerSide == pieceAtFromCoords->m_playerSide)
	{
		return ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
	}

	// Rules above is the basic rule, then check isTeleporting
	// which is captured, have a helper function to move piece capture piece, all logic happens in chess piece
	// remember capture first then move(should move check the destination is occupied? for safe?)

	// #ToDo check moving rule



	ChessMoveResult moveResult = ChessMoveResult::UNKNOWN;



	if (isTeleporting)
	{
		MovePiece(fromCoords, toCoords); // cheating, no promotion
		moveResult = ChessMoveResult::VALID_MOVE_NORMAL;
	}
	else
	{
		moveResult = pieceAtFromCoords->TryToMove(fromCoords, toCoords, promotionType);
		if (!IsValid(moveResult))
		{
			return moveResult;
		}
	}

	bool isKingCaptured = false;
	if (pieceAtToCoords != nullptr && pieceAtToCoords->m_definition->m_type == PieceType::KING)
	{
		isKingCaptured = true;
	}

	// Switch State
	if (isKingCaptured)
	{
		SetNextState((m_currentState == MatchState::WHITE_MOVE) ? MatchState::WHITE_WIN : MatchState::BLACK_WIN);
	}
	else
	{
		SetNextState((m_currentState == MatchState::WHITE_MOVE) ? MatchState::BLACK_MOVE : MatchState::WHITE_MOVE); // Assume game is playing
	}
	m_turnNumber++; // Add Turn

	return moveResult;

	/*
	std::string fromNotation = GetNotationFromBoardCoords(fromCoords);
	std::string toNotation = GetNotationFromBoardCoords(toCoords);
	// Can Move
	g_theDevConsole->AddText(DevConsole::INFO_MAJOR, Stringf("%s's %s has been moved from %s to %s",
		(currentSide == PLAYER_WHITE) ? "White" : "Black",
		GetStringFromPieceType(pieceAtFromCoords->m_definition->m_type).c_str(),
		fromNotation.c_str(), toNotation.c_str()));

	// if no pieces at to Coords
	if (pieceAtToCoords == nullptr)
	{
		// Animation
		Vec3 startPos = GetSquareCenterFromBoardCoords(fromCoords);
		Vec3 endPos = GetSquareCenterFromBoardCoords(toCoords);
		pieceAtFromCoords->Move(startPos, endPos);
		//pieceAtFromCoords->m_position = endPos;

		// Logic
		m_piecesOnBoard[GetPieceIndexFromBoardCoords(fromCoords)] = nullptr;
		m_piecesOnBoard[GetPieceIndexFromBoardCoords(toCoords)] = pieceAtFromCoords;

		// Switch State
		SetNextState((m_currentState == MatchState::WHITE_MOVE) ? MatchState::BLACK_MOVE : MatchState::WHITE_MOVE); // Assume game is playing
		return "";
	}

	// the remaining circumstance is capture a opponent's piece.

	// Animation
	Vec3 startPosMoved = GetSquareCenterFromBoardCoords(fromCoords);
	Vec3 endPosMoved = GetSquareCenterFromBoardCoords(toCoords);
	pieceAtFromCoords->Move(startPosMoved, endPosMoved);
	//pieceAtFromCoords->m_position = endPosMoved;

	Vec3 startPosCaptured = GetSquareCenterFromBoardCoords(toCoords);
	Vec3 endPosCaptured = GetNextEmptyWorldPosForCaughtPiece();
	pieceAtToCoords->Move(startPosCaptured, endPosCaptured);
	//pieceAtToCoords->m_position = endPosCaptured;

	// Logic
	m_piecesCaught.push_back(pieceAtToCoords);
	m_piecesOnBoard[GetPieceIndexFromBoardCoords(fromCoords)] = nullptr;
	m_piecesOnBoard[GetPieceIndexFromBoardCoords(toCoords)] = pieceAtFromCoords;

	// Log to DevConsole
	g_theDevConsole->AddText(Rgba8(185, 122, 87), Stringf("%s captured %s's %s at %s",
		(currentSide == PLAYER_WHITE) ? "White" : "Black",
		(currentSide == PLAYER_WHITE) ? "Black" : "White",
		GetStringFromPieceType(pieceAtToCoords->m_definition->m_type).c_str(),
		toNotation.c_str(), toNotation.c_str()));



	*/


}

void ChessMatch::MovePiece(IntVec2 fromCoords, IntVec2 toCoords)
{
	std::string fromNotation = GetNotationFromBoardCoords(fromCoords);
	std::string toNotation = GetNotationFromBoardCoords(toCoords);
	ChessPiece* pieceAtFromCoords = m_piecesOnBoard[GetPieceIndexFromBoardCoords(fromCoords)];
	ChessPiece* pieceAtToCoords = m_piecesOnBoard[GetPieceIndexFromBoardCoords(toCoords)];

	if (pieceAtFromCoords == nullptr)
	{
		ERROR_AND_DIE(Stringf("There is not piece at %s!", fromNotation.c_str()));
	}
	pieceAtFromCoords->m_turnLastMoved = m_turnNumber;

	if (pieceAtToCoords != nullptr)
	{
		CapturePiece(toCoords);
	}

	Vec3 startPosMoved = GetSquareCenterFromBoardCoords(fromCoords);
	Vec3 endPosMoved = GetSquareCenterFromBoardCoords(toCoords);
	pieceAtFromCoords->SetAnimation(startPosMoved, endPosMoved, pieceAtFromCoords->m_definition->m_type == PieceType::KNIGHT || pieceAtFromCoords->m_definition->m_type == PieceType::KING);

	m_piecesOnBoard[GetPieceIndexFromBoardCoords(fromCoords)] = nullptr;
	m_piecesOnBoard[GetPieceIndexFromBoardCoords(toCoords)] = pieceAtFromCoords;

	g_theDevConsole->AddText(DevConsole::INFO_MAJOR, Stringf("%s's %s has been moved from %s to %s",
		(pieceAtFromCoords->m_playerSide == PLAYER_WHITE) ? "White" : "Black",
		GetStringFromPieceType(pieceAtFromCoords->m_definition->m_type).c_str(),
		fromNotation.c_str(), toNotation.c_str()));
}

void ChessMatch::CapturePiece(IntVec2 coords)
{
	ChessPiece* pieceAtCoords = m_piecesOnBoard[GetPieceIndexFromBoardCoords(coords)];
	std::string notation = GetNotationFromBoardCoords(coords);
	if (pieceAtCoords == nullptr)
	{
		ERROR_AND_DIE(Stringf("There is not piece at %s!", notation.c_str()));
	}
	pieceAtCoords->m_turnLastMoved = m_turnNumber;
	Vec3 startPosCaptured = GetSquareCenterFromBoardCoords(coords);
	Vec3 endPosCaptured = GetNextEmptyWorldPosForCaughtPiece();
	pieceAtCoords->SetAnimation(startPosCaptured, endPosCaptured, true);

	m_piecesCaught.push_back(pieceAtCoords);
	m_piecesOnBoard[GetPieceIndexFromBoardCoords(coords)] = nullptr;

	g_theDevConsole->AddText(Rgba8(185, 122, 87), Stringf("%s's %s has been captured at %s",
		(pieceAtCoords->m_playerSide == PLAYER_WHITE) ? "White" : "Black",
		GetStringFromPieceType(pieceAtCoords->m_definition->m_type).c_str(),
		notation.c_str()));
}

int ChessMatch::GetTurnNumber() const
{
	return m_turnNumber;
}

bool ChessMatch::IsSquareOccupied(IntVec2 coords) const
{
	ChessPiece* pieceAtCoords = m_piecesOnBoard[GetPieceIndexFromBoardCoords(coords)];
	return pieceAtCoords != nullptr;
}

bool ChessMatch::IsSquareUnderAttack(IntVec2 coords, PlayerSide side) const
{
	// #ToDo 
	UNUSED(coords);
	UNUSED(side);
	return false;
}

void ChessMatch::PrintMatchState() const
{
	Rgba8 color = Rgba8(255, 127, 0);
	switch (m_currentState)
	{
	case MatchState::WHITE_MOVE:
		g_theDevConsole->AddText(color, "=================================================");
		g_theDevConsole->AddText(color, "It is White's turn to move");
		break;
	case MatchState::BLACK_MOVE:
		g_theDevConsole->AddText(color, "=================================================");
		g_theDevConsole->AddText(color, "It is Black's turn to move");
		break;
	case MatchState::WHITE_WIN:
		g_theDevConsole->AddText(color, "#################################################");
		g_theDevConsole->AddText(color, "White has won the match!");
		g_theDevConsole->AddText(color, "#################################################");
		break;
	case MatchState::BLACK_WIN:
		g_theDevConsole->AddText(color, "#################################################");
		g_theDevConsole->AddText(color, "Black has won the match!");
		g_theDevConsole->AddText(color, "#################################################");
		break;
	}
}

bool ChessMatch::IsCurrentState(MatchState state) const
{
	return state == m_currentState;
}

void ChessMatch::SwitchToNextState()
{
	if (m_currentState != m_nextState)
	{
		ExitState(m_currentState);
		m_currentState = m_nextState;
		EnterState(m_nextState);
	}
}

void ChessMatch::SetNextState(MatchState newState)
{
	m_nextState = newState;
}

void ChessMatch::EnterState(MatchState state)
{
	switch (state)
	{
	case MatchState::DEFAULT:
		//StartNewMatch();
		break;
	case MatchState::WHITE_MOVE:
		PrintMatchState();
		PrintBoardState();
		break;
	case MatchState::BLACK_MOVE:
		PrintMatchState();
		PrintBoardState();
		break;
	case MatchState::WHITE_WIN:
		PrintMatchState();
		break;
	case MatchState::BLACK_WIN:
		PrintMatchState();

		break;

	}
}

void ChessMatch::ExitState(MatchState state)
{
	switch (state)
	{
	case MatchState::DEFAULT:
		StartNewMatch();
		break;
	case MatchState::WHITE_MOVE:

		break;
	case MatchState::BLACK_MOVE:

		break;
	case MatchState::WHITE_WIN:

		break;
	case MatchState::BLACK_WIN:

		break;

	}
}

void ChessMatch::UpdateCurrentState()
{
	switch (m_currentState)
	{
	case MatchState::DEFAULT:


		SetNextState(MatchState::WHITE_MOVE);
		break;
	case MatchState::WHITE_MOVE:

		break;
	case MatchState::BLACK_MOVE:

		break;
	case MatchState::WHITE_WIN:

		break;
	case MatchState::BLACK_WIN:

		break;

	}
}

void ChessMatch::UpdateMouseBasedPieceMovement()
{
	// No selection state: InValid(SelectedCoords)
	UpdateCurrentImpactCoords();

	UpdateMouseInputForPieceMovement();

	DebugAddMessage(Stringf("Current Impact Square: %s", GetNotationFromBoardCoords(m_currentImpactCoords).c_str()), 0.f);
}

void ChessMatch::UpdateCurrentImpactCoords()
{
	// Reset Impact Coord Every Frame
	m_currentImpactCoords = IntVec2(-1, -1);
	RaycastResult3D finalRaycastResult;

	// Get Ray
	Player* playerController = g_theGame->GetPlayerController();
	Vec3 rayStart;
	Vec3 rayFwdNormal;
	float rayLength;
	playerController->GetRay(rayStart, rayFwdNormal, rayLength);

	// Raycast to board
	Plane3 chessBoardPlane = Plane3(Vec3::UP, 0.f);
	if (chessBoardPlane.IsPointInFrontOf(rayStart))
	{
		RaycastResult3D result = RaycastVsPlane3D(rayStart, rayFwdNormal, rayLength, chessBoardPlane);
		if (result.m_didImpact)
		{
			IntVec2 hitCoords = GetBoardCoordsFromWorldPos(result.m_impactPos);
			if (IsCoordsValid(hitCoords))
			{
				if (!finalRaycastResult.m_didImpact)
				{
					finalRaycastResult = result;
					m_currentImpactCoords = hitCoords;
				}
				else
				{
					if (result.m_impactDist < finalRaycastResult.m_impactDist)
					{
						finalRaycastResult = result;
						m_currentImpactCoords = hitCoords;
					}
				}
			}

		}
	}

	// Raycast to piece
	for (int tileY = 0; tileY < 8; ++tileY)
	{
		for (int tileX = 0; tileX < 8; ++tileX)
		{
			int tileIndex = tileX + tileY * 8;
			IntVec2 tileCoords = IntVec2(tileX, tileY);
			ChessPiece* piece = m_piecesOnBoard[tileIndex];
			if (piece == nullptr) continue;
			Vec2 centerXY;
			FloatRange minMaxZ;
			float radiusXY;
			piece->GetZCylinderCollider(centerXY, minMaxZ, radiusXY);
			RaycastResult3D result = RaycastVsCylinderZ3D(rayStart, rayFwdNormal, rayLength, centerXY, minMaxZ, radiusXY);
			if (result.m_didImpact)
			{
				if (!finalRaycastResult.m_didImpact)
				{
					finalRaycastResult = result;
					m_currentImpactCoords = tileCoords;
				}
				else
				{
					if (result.m_impactDist < finalRaycastResult.m_impactDist)
					{
						finalRaycastResult = result;
						m_currentImpactCoords = tileCoords;
					}
				}
			}
		}
	}
}

void ChessMatch::UpdateMouseInputForPieceMovement()
{
	if (!IsCoordsValid(m_selectedCoords))
	{
		// no selection
		if (IsCoordsValid(m_currentImpactCoords))
		{
			if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
			{
				// Check if it is current player's piece				
				PlayerSide currentSide = GetCurrentPlayerSide();
				if (currentSide < 0)
				{
					return;
				}

				ChessPiece* pieceAtFromCoords = m_piecesOnBoard[GetPieceIndexFromBoardCoords(m_currentImpactCoords)];

				if (pieceAtFromCoords == nullptr)
				{
					return;
				}

				if (currentSide != pieceAtFromCoords->m_playerSide)
				{
					return;
				}

				
				// Select
				m_selectedCoords = m_currentImpactCoords;

			}
		}

	}
	else
	{
		// Selected
		if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
		{
			// Deselect
			m_selectedCoords = IntVec2(-1, -1);
			return;
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) && IsCoordsValid(m_currentImpactCoords))
		{
			// Move
			char const* command = "ChessMove from=%s to=%s teleport=%s";
			bool isTeleporting = g_theInput->IsKeyDown(KEYCODE_CONTROL);
			std::string fromNotation = GetNotationFromBoardCoords(m_selectedCoords);
			std::string toNotation = GetNotationFromBoardCoords(m_currentImpactCoords);

			g_theDevConsole->Execute(Stringf(command, fromNotation.c_str(), toNotation.c_str(), isTeleporting ? "true" : "false"));
			// Deselect
			m_selectedCoords = IntVec2(-1, -1);
			return;
		}
	}
}

bool ChessMatch::IsCoordsValid(IntVec2 const& coords) const
{
	return coords.x >= 0 && coords.x < 8 && coords.y >= 0 && coords.y < 8;
}

void ChessMatch::ShowImGuiPanel()
{
	if (ImGui::Begin("Network Match Control Panel"))
	{

		NetState netState = g_theNetwork->GetState();

		switch (netState)
		{
		case NetState::INACTIVE:
			ImGui::Text("NetworkState: Inactive");
			break;
		case NetState::IDLE:
			ImGui::Text("NetworkState: Idle");
			break;
		case NetState::SERVER_LISTENING:
			ImGui::Text("NetworkState: Server Listening");
			break;
		case NetState::CLIENT_CONNECTING:
			ImGui::Text("NetworkState: Client Connecting");
			break;
		case NetState::CLIENT_CONNECTED:
			ImGui::Text("NetworkState: Client Connected");
			break;
		default:
			ImGui::Text("NetworkState: UNKNOWN");
			break;
		}


		ImGui::Text("Server IP: %s", m_serverIP.c_str());

		int serverPort = m_serverPort;
		if (netState == NetState::IDLE)
		{
			static char ipBuffer[128];
			ImGui::SetNextItemWidth(200.0f);
			ImGui::InputText("##new_ip", ipBuffer, sizeof(ipBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Apply IP"))
			{
				m_serverIP = ipBuffer;
			}
			if (ImGui::InputInt("Server Port", &serverPort))
			{
				if (serverPort <= 65535 && serverPort >= 0)
				{
					m_serverPort = (unsigned short)serverPort;
				}
			}
		}
		else
		{
			ImGui::Text("Server Port: %d", serverPort);
		}

		if (ImGui::Button("Listen")) 
		{
			ButtonChessListen();
		}
		ImGui::SameLine();
		if (ImGui::Button("Connect")) 
		{
			ButtonChessConnect();
		}
		ImGui::SameLine();
		if (ImGui::Button("Disconnect")) 
		{
			ButtonChessDisconnect();
		}

		if (ImGui::Button("Begin Play"))
		{
			ButtonChessBegin();
		}
		ImGui::SameLine();
		if (ImGui::Button("Resign"))
		{
			ButtonChessResign();
		}

		ImGui::Text("Local Player: %s", m_localPlayerName.c_str());
		ImGui::Text("Remote Player: %s", m_remotePlayerName.c_str());
		
		switch (m_localPlayerSide)
		{
		case PLAYER_WHITE:
			ImGui::Text("Local Player Side: White");
			break;
		case PLAYER_BLACK:
			ImGui::Text("Local Player Side: Black");
			break;
		default:
			ImGui::Text("Local Player Side: Unknown");
			break;
		}

	}
	ImGui::End();
}

bool ChessMatch::Command_Echo(EventArgs& args)
{
	g_theDevConsole->AddText(DevConsole::INFO_TEXT, args.GetValue("text", "EMPTYECHO"));

	return true;
}

bool ChessMatch::Command_ChessServerInfo(EventArgs& args)
{
	std::string ip = args.GetValue("ip", "??");
	if (ip != "??")
	{
		if (g_theNetwork->GetState() != NetState::IDLE)
		{
			g_theDevConsole->AddText(DevConsole::ERROR, "Cannot change Server IP, Network System is not in idle state!");
			return true;
		}
		g_theGame->GetMatch()->m_serverIP = ip;
	}

	int port = args.GetValue("port", -1);
	if (port <= 65535 && port >= 0)
	{
		if (g_theNetwork->GetState() != NetState::IDLE)
		{
			g_theDevConsole->AddText(DevConsole::ERROR, "Cannot change Server Port, Network System is not in idle state!");
			return true;
		}
		g_theGame->GetMatch()->m_serverPort = (unsigned short)port;
	}

	g_theDevConsole->AddText(DevConsole::INFO_MINOR, Stringf("Current Server IP: %s", g_theGame->GetMatch()->m_serverIP.c_str()));
	g_theDevConsole->AddText(DevConsole::INFO_MINOR, Stringf("Current Server Port: %d", g_theGame->GetMatch()->m_serverPort));

	return true;
}

bool ChessMatch::Command_ChessListen(EventArgs& args)
{
	if (g_theNetwork->GetState() != NetState::IDLE)
	{
		g_theDevConsole->AddText(DevConsole::ERROR, "Cannot Start Server,  Network System is not in idle state!");
		return true;
	}
	int port = args.GetValue("port", -1);
	if (port <= 65535 && port >= 0)
	{
		g_theGame->GetMatch()->m_serverPort = (unsigned short)port;
	}

	bool ok = g_theNetwork->StartServer(g_theGame->GetMatch()->m_serverPort);
	if (ok)
	{
		g_theDevConsole->AddText(DevConsole::INFO_MAJOR, Stringf("Server start listening at port %d", g_theGame->GetMatch()->m_serverPort));
	}
	else
	{
		g_theDevConsole->AddText(DevConsole::ERROR, "Cannot Start Server! Check Debugger Print");
	}
	return true;
}

bool ChessMatch::Command_ChessConnect(EventArgs& args)
{
	if (g_theNetwork->GetState() != NetState::IDLE)
	{
		g_theDevConsole->AddText(DevConsole::ERROR, "Cannot Start Client,  Network System is not in idle state!");
		return true;
	}

	std::string ip = args.GetValue("ip", "??");
	if (ip != "??")
	{
		g_theGame->GetMatch()->m_serverIP = ip;
	}

	int port = args.GetValue("port", -1);
	if (port <= 65535 && port >= 0)
	{
		g_theGame->GetMatch()->m_serverPort = (unsigned short)port;
	}

	bool ok = g_theNetwork->StartClient(g_theGame->GetMatch()->m_serverIP, g_theGame->GetMatch()->m_serverPort);
	if (ok)
	{
		g_theDevConsole->AddText(DevConsole::INFO_MAJOR, Stringf("Client start connecting %s:%d", g_theGame->GetMatch()->m_serverIP.c_str(), g_theGame->GetMatch()->m_serverPort));
	}
	else
	{
		g_theDevConsole->AddText(DevConsole::ERROR, "Cannot Start Client! Check Debugger Print");
	}
	return true;
}

bool ChessMatch::Command_ChessDisconnect(EventArgs& args)
{
	NetState netstate = g_theNetwork->GetState();
	if (netstate != NetState::SERVER_LISTENING && netstate != NetState::CLIENT_CONNECTED)
	{
		g_theDevConsole->AddText(DevConsole::ERROR, "Cannot disconnect, state is not listening or connected");
		return true;
	}

	if (!args.GetValue("remote", false))
	{
		std::string reason = args.GetValue("reason", "");
		std::string remoteCmd = "Remotecmd cmd=ChessDisconnect";
		if (reason != "")
		{
			remoteCmd += " reason=" + reason;
		}
		g_theDevConsole->Execute(remoteCmd);
	}

	g_theNetwork->m_pendingDisconnected = true;

	return true;
}

bool ChessMatch::Command_ChessPlayerInfo(EventArgs& args)
{
	if (!args.GetValue("remote", false))
	{
		g_theGame->GetMatch()->m_localPlayerName = args.GetValue("name", "UNKNOWN");
		std::string remoteCmd = "Remotecmd cmd=ChessPlayerInfo name=" + g_theGame->GetMatch()->m_localPlayerName;
		g_theDevConsole->Execute(remoteCmd);
	}
	else
	{
		g_theGame->GetMatch()->m_remotePlayerName = args.GetValue("name", "UNKNOWN");
	}

	return true;
}

bool ChessMatch::Command_ChessMove(EventArgs& args)
{
	std::string fromNotation = args.GetValue("from", "??");
	std::string toNotation = args.GetValue("to", "??");

	if (fromNotation == "??" || toNotation == "??")
	{
		g_theDevConsole->AddText(DevConsole::ERROR, "Illegal chess move! Must have from= and to= arguments.");
		g_theDevConsole->AddText(DevConsole::WARNING, "	Example: ChessMove from=e2 to=e4");
		return true;
	}

	if (fromNotation.length() != 2)
	{
		g_theDevConsole->AddText(DevConsole::ERROR, Stringf("Illegal \"from\" square \"%s\"! Must be a two-letter [Column][Rank] ", fromNotation.c_str()));
		g_theDevConsole->AddText(DevConsole::WARNING, "	Example: E2, E4; A1 is bottom left and H8 is top-right");
		return true;
	}

	if (toNotation.length() != 2)
	{
		g_theDevConsole->AddText(DevConsole::ERROR, Stringf("Illegal \"to\" square \"%s\"! Must be a two-letter [Column][Rank] ", toNotation.c_str()));
		g_theDevConsole->AddText(DevConsole::WARNING, "	Example: E2, E4; A1 is bottom left and H8 is top-right");
		return true;
	}

	IntVec2 fromCoords;
	IntVec2 toCoords;

	fromCoords.x = std::toupper(fromNotation[0]) - 'A';
	fromCoords.y = fromNotation[1] - '1';
	toCoords.x = std::toupper(toNotation[0]) - 'A';
	toCoords.y = toNotation[1] - '1';

	if (fromCoords.x < 0 || fromCoords.x >= 8 || fromCoords.y < 0 || fromCoords.y >=8)
	{
		g_theDevConsole->AddText(DevConsole::ERROR, Stringf("Illegal \"from\" square \"%s\"! Must be a two-letter [Column][Rank] ", fromNotation.c_str()));
		g_theDevConsole->AddText(DevConsole::WARNING, "	Example: E2, E4; A1 is bottom left and H8 is top-right");
		return true;
	}

	if (toCoords.x < 0 || toCoords.x >= 8 || toCoords.y < 0 || toCoords.y >= 8)
	{
		g_theDevConsole->AddText(DevConsole::ERROR, Stringf("Illegal \"to\" square \"%s\"! Must be a two-letter [Column][Rank] ", toNotation.c_str()));
		g_theDevConsole->AddText(DevConsole::WARNING, "	Example: E2, E4; A1 is bottom left and H8 is top-right");
		return true;
	}

	bool isTeleporting = args.GetValue("teleport", false);
	PieceType promotionType = GetPieceTypeFromString(args.GetValue("promoteTo", ""));


	// Check if valid for local player to move
	// first check if it is network mode
	// then check local player side == current player
	// or if it is remote, check reverse
	// 

	if (!IsPlayingLocally())
	{
		PlayerSide currentSide = g_theGame->GetMatch()->GetCurrentPlayerSide();

		if (!args.GetValue("remote", false)) // local cmd
		{
			if (g_theGame->GetMatch()->m_localPlayerSide != currentSide)
			{
				DebugAddMessage("Not Your Move!", 3.f, Rgba8::RED);
				return true;
			}
		}
		else // remote cmd
		{
			if (g_theGame->GetMatch()->m_localPlayerSide == currentSide)
			{
				DebugAddMessage("Your opponent is trying to move in your turn!", 3.f, Rgba8::RED);
				return true;
			}
		}
	}


	ChessMoveResult result = g_theGame->GetMatch()->TryToMoveChessPiece(fromCoords, toCoords, isTeleporting, promotionType);
	if (!IsValid(result))
	{
		//	give arguments? namedstrings
		g_theDevConsole->AddText(DevConsole::ERROR, GetMoveResultString(result));
		DebugAddMessage("An invalid move command has been entered", 3.f, Rgba8::RED);
		return true;
	}

	if (IsPlayingLocally())
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------
	if (!args.GetValue("remote", false))
	{
		std::string remoteCmd = "Remotecmd cmd=ChessMove";

		for (const auto& kv : args.m_keyValuePairs)
		{
			remoteCmd += " " + kv.first + "=" + kv.second;
		}
		g_theDevConsole->Execute(remoteCmd);
	}

	return true;
}

bool ChessMatch::Command_ChessResign(EventArgs& args)
{
	if (!IsPlayingLocally())
	{
		if (g_theGame->GetMatch()->m_localPlayerSide == PLAYER_UNKNOWN)
		{
			return true;
		}

		if (!args.GetValue("remote", false))
		{

			g_theGame->GetMatch()->SetNextState((g_theGame->GetMatch()->m_localPlayerSide == PLAYER_BLACK) ? MatchState::WHITE_WIN : MatchState::BLACK_WIN);
			std::string remoteCmd = "Remotecmd cmd=ChessResign";
			g_theDevConsole->Execute(remoteCmd);
		}
		else
		{
			g_theGame->GetMatch()->SetNextState((g_theGame->GetMatch()->m_localPlayerSide == PLAYER_WHITE) ? MatchState::WHITE_WIN : MatchState::BLACK_WIN);
		}
	}

	return true;
}

bool ChessMatch::Command_RemoteCmd(EventArgs& args)
{
	std::string cmd = args.GetValue("cmd", "");
	if (cmd == "")
	{
		g_theDevConsole->AddText(DevConsole::ERROR, "No cmd");
		return true;
	}
	std::string commandString = cmd;

	for (const auto& kv : args.m_keyValuePairs) 
	{
		if (kv.first != "cmd")
		{
			commandString += " " + kv.first + "=" + kv.second;
		}
	}

	g_theNetwork->QueueOutgoingString(commandString);
	return true;
}

void ChessMatch::ButtonChessConnect()
{
	g_theDevConsole->Execute("ChessConnect");
}

void ChessMatch::ButtonChessListen()
{
	g_theDevConsole->Execute("ChessListen");

}

void ChessMatch::ButtonChessDisconnect()
{
	g_theDevConsole->Execute("ChessDisconnect");

}

void ChessMatch::ButtonChessBegin()
{
	g_theDevConsole->Execute("ChessBegin");
}

void ChessMatch::ButtonChessResign()
{
	g_theDevConsole->Execute("ChessResign");
}

bool ChessMatch::Command_ChessBegin(EventArgs& args)
{
	UNUSED(args);

	if (IsPlayingLocally())
	{
		g_theGame->GetMatch()->SetNextState(MatchState::DEFAULT);
	}
	else
	{
		g_theGame->GetMatch()->SetNextState(MatchState::DEFAULT);

		if (!args.GetValue("remote", false))
		{
			g_theGame->GetMatch()->m_localPlayerSide = PLAYER_WHITE;
			std::string remoteCmd = "Remotecmd cmd=ChessBegin";
			g_theDevConsole->Execute(remoteCmd);
		}
		else
		{
			g_theGame->GetMatch()->m_localPlayerSide = PLAYER_BLACK;
		}
	}

	return true;
}
