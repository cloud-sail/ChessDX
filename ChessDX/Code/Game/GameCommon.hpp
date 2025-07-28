#pragma once
#include "Engine/Core/EngineCommon.hpp"

//-----------------------------------------------------------------------------------------------
class AudioSystem;
class InputSystem;
class Renderer;
class Window;
class Skybox;
class Shader;
class Texture;
class NetworkSystem;

struct Vertex_PCU; 
struct Vertex_PCUTBN;
struct IntVec2;
struct Vec3;
struct FloatRange;

class App;
class Game;
class Player;
class ChessPiece;
class ChessBoard;
class ChessMatch;
struct ChessPieceDefinition;

//-----------------------------------------------------------------------------------------------
extern AudioSystem*		g_theAudio;
extern NetworkSystem*	g_theNetwork;
extern InputSystem*		g_theInput;
extern Renderer*		g_theRenderer;
extern Window*			g_theWindow;
extern App*				g_theApp;
extern Game*			g_theGame;

//-----------------------------------------------------------------------------------------------
extern bool g_isDebugDraw;

//-----------------------------------------------------------------------------------------------
// Gameplay Constants
constexpr float SCREEN_SIZE_X = 1600.f;
constexpr float SCREEN_SIZE_Y = 800.f;

constexpr float CAMERA_MOVE_SPEED = 2.f;
constexpr float CAMERA_YAW_TURN_RATE = 60.f;
constexpr float CAMERA_PITCH_TURN_RATE = 60.f;
constexpr float CAMERA_ROLL_TURN_RATE = 90.f;
constexpr float CAMERA_SPEED_FACTOR = 8.f;

constexpr float CAMERA_MAX_PITCH = 85.f;
constexpr float CAMERA_MAX_ROLL = 45.f;

constexpr float CHESS_BOARD_SQUARE_SIZE = 1.f;
constexpr float CHESS_BOARD_THINKNESS = 0.4f;

constexpr float	CHESS_MOVE_DURATION = 1.f;
constexpr float CHESS_JUMP_HEIGHT = 1.f;
//-----------------------------------------------------------------------------------------------
enum PlayerSide
{
	PLAYER_UNKNOWN = -1,
	PLAYER_WHITE, // White goes first
	PLAYER_BLACK,
	PLAYER_SIDE_NUM
};

enum class PieceType
{
	UNKNOWN = -1,
	KING,	// K
	QUEEN,	// Q
	ROOK,	// R
	BISHOP, // B
	KNIGHT, // N
	PAWN,	// P
	NUM
};

int GetIntSign(int value);

bool IsPlayingLocally();
