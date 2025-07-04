#include "Game/ChessErrorCheck.hpp"


bool IsValid(ChessMoveResult result)
{
	switch (result)
	{
		case ChessMoveResult::VALID_MOVE_NORMAL:
		case ChessMoveResult::VALID_MOVE_PROMOTION:
		case ChessMoveResult::VALID_CASTLE_KINGSIDE:
		case ChessMoveResult::VALID_CASTLE_QUEENSIDE:
		case ChessMoveResult::VALID_CAPTURE_NORMAL:
		case ChessMoveResult::VALID_CAPTURE_ENPASSANT:
			return true;

		case ChessMoveResult::INVALID_GAME_NOT_PLAYING:
		case ChessMoveResult::INVALID_MOVE_BAD_LOCATION:
		case ChessMoveResult::INVALID_MOVE_NO_PIECE:
		case ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE:
		case ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE:
		case ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE:
		case ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED:
		case ChessMoveResult::INVALID_MOVE_PATH_BLOCKED:
		case ChessMoveResult::INVALID_MOVE_ENDS_IN_CHECK:
		case ChessMoveResult::INVALID_MOVE_PAWN_WRONG_PROMOTION:
		case ChessMoveResult::INVALID_ENPASSANT_STALE:
		case ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED:
		case ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED:
		case ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED:
		case ChessMoveResult::INVALID_CASTLE_THROUGH_CHECK:
		case ChessMoveResult::INVALID_CASTLE_OUT_OF_CHECK:
			return false;

		default:
			ERROR_AND_DIE("Unhandled ChessMoveResult enum value");
	}
}

const char* GetMoveResultString(ChessMoveResult result)
{
	switch (result)
	{
	case ChessMoveResult::UNKNOWN:							return "Unknown ChessMoveResult!";
	case ChessMoveResult::VALID_MOVE_NORMAL:				return "Valid move";
	case ChessMoveResult::VALID_MOVE_PROMOTION:				return "Valid move, resulting in pawn promotion";
	case ChessMoveResult::VALID_CASTLE_KINGSIDE:			return "Valid move, castling kingside";
	case ChessMoveResult::VALID_CASTLE_QUEENSIDE:			return "Valid move, castling queenside";
	case ChessMoveResult::VALID_CAPTURE_NORMAL: 			return "Valid move, capturing enemy piece";
	case ChessMoveResult::VALID_CAPTURE_ENPASSANT:			return "Valid move, capturing enemy pawn en passant";
	case ChessMoveResult::INVALID_GAME_NOT_PLAYING:			return "Invalid Game State, Can only use ChessMove command when it is someone's turn to move.";
	case ChessMoveResult::INVALID_MOVE_BAD_LOCATION:		return "Invalid move; invalid board location given";
	case ChessMoveResult::INVALID_MOVE_NO_PIECE:			return "Invalid move; no piece at location given";
	case ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE:		return "Invalid move; can't move opponent's piece";
	case ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE:		return "Invalid move; didn't go anywhere";
	case ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE:	return "Invalid move; wrong move shape";
	case ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED:	return "Invalid move; destination is blocked by a piece";
	case ChessMoveResult::INVALID_MOVE_PATH_BLOCKED:		return "Invalid move; path is blocked by other piece";
	case ChessMoveResult::INVALID_MOVE_ENDS_IN_CHECK:		return "Invalid move; can't leave yourself in check";
	case ChessMoveResult::INVALID_MOVE_PAWN_WRONG_PROMOTION:return "Invalid move; pawn reach the end but with wrong promotion type.";
	case ChessMoveResult::INVALID_ENPASSANT_STALE:			return "Invalid move; en passant must immediately follow a pawn double-move";
	case ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED:	return "Invalid castle; king has moved previously";
	case ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED:	return "Invalid castle; that rook has moved previously";
	case ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED:		return "Invalid castle; pieces in-between king and rook";
	case ChessMoveResult::INVALID_CASTLE_THROUGH_CHECK:		return "Invalid castle; king can't move through check";
	case ChessMoveResult::INVALID_CASTLE_OUT_OF_CHECK:		return "Invalid castle; king can't castle out of check";

	default: ERROR_AND_DIE(Stringf("Unhandled ChessMoveResult enum value #%d", (int)result));;
	}
}