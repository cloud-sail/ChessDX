#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include <string>


enum class ChessMoveResult;

enum class MatchState
{
	DEFAULT, // enter default will reset the match
	WHITE_MOVE,
	BLACK_MOVE,
	WHITE_WIN,
	BLACK_WIN,
};


class ChessMatch
{
public:
	static std::string	GetNotationFromBoardCoords(IntVec2 const& coords); // not validate the input
	static IntVec2		GetBoardCoordsFromNotation(std::string const& notation); // not validate the input
	static Vec3			GetSquareCenterFromBoardCoords(IntVec2 const& coords);
	static int			GetPieceIndexFromBoardCoords(IntVec2 const& coords);
	static IntVec2		GetBoardCoordsFromWorldPos(Vec3 const& worldPos);




	ChessMatch();
	~ChessMatch();

	void Update();
	void Render() const;

	void InitializeBoard();
	void InitializePieces();

	void CleanBoardAndPieces();

	void StartNewMatch();

	Vec3		GetNextEmptyWorldPosForCaughtPiece() const;
	char		GetGlyghFromBoardCoords(IntVec2 const& coords) const;
	PlayerSide	GetCurrentPlayerSide() const;

	void PrintMatchState() const;
	void PrintBoardState() const;

	//std::string TryToMoveChessPiece(IntVec2 fromCoords, IntVec2 toCoords);
	ChessMoveResult TryToMoveChessPiece(IntVec2 fromCoords, IntVec2 toCoords, bool isTeleporting, PieceType promotionType = PieceType::UNKNOWN);

	// Helper function without check
	void MovePiece(IntVec2 fromCoords, IntVec2 toCoords);
	void CapturePiece(IntVec2 coords);

	int GetTurnNumber() const;

	bool IsSquareOccupied(IntVec2 coords) const;
	bool IsSquareUnderAttack(IntVec2 coords, PlayerSide side) const;
public:

	std::vector<ChessPiece*> m_piecesOnBoard; // size 64, do not push_back
	std::vector<ChessPiece*> m_piecesCaught;

	ChessBoard* m_board = nullptr;

public:
	bool IsCurrentState(MatchState state) const;

	void SwitchToNextState();
	void SetNextState(MatchState newState);
	void EnterState(MatchState state);
	void ExitState(MatchState state);

	void UpdateCurrentState();



	MatchState m_currentState = MatchState::DEFAULT;
	MatchState m_nextState = MatchState::DEFAULT;


private:
	int m_turnNumber = 0;


private:
	void UpdateMouseBasedPieceMovement();

	void UpdateCurrentImpactCoords();
	void UpdateMouseInputForPieceMovement();

	bool IsCoordsValid(IntVec2 const& coords) const;

private:
	IntVec2 m_currentImpactCoords	= IntVec2(-1, -1);
	IntVec2 m_selectedCoords		= IntVec2(-1, -1);

private:
	void ShowImGuiPanel();

//-----------------------------------------------------------------------------------------------
// Networking
public:
	static bool	Command_Echo(EventArgs& args); // local
	static bool Command_ChessServerInfo(EventArgs& args); // local
	static bool Command_ChessListen(EventArgs& args); // local
	static bool Command_ChessConnect(EventArgs& args); // local

	static bool Command_ChessDisconnect(EventArgs& args); // remote
	static bool Command_ChessPlayerInfo(EventArgs& args); // remote
	static bool	Command_ChessBegin(EventArgs& args); // remote
	static bool	Command_ChessMove(EventArgs& args); // remote
	static bool Command_ChessResign(EventArgs& args); // remote
	static bool Command_RemoteCmd(EventArgs& args); // local


	void ButtonChessConnect();
	void ButtonChessListen();
	void ButtonChessDisconnect();
	void ButtonChessBegin();
	void ButtonChessResign();

public:
	std::string m_serverIP = "127.0.0.1";
	unsigned short m_serverPort = 3100;

	// Notes: not checking both names are same or correct on both side
	// The person who enter chess begin will be the white
	std::string m_localPlayerName = "UNKNOWN";
	std::string m_remotePlayerName = "UNKNOWN";
	PlayerSide m_localPlayerSide = PLAYER_UNKNOWN;
};

