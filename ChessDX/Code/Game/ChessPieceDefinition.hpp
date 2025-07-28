#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <string>

class IndexBuffer;
class VertexBuffer;


std::string GetStringFromPieceType(PieceType pieceType);
PieceType GetPieceTypeFromString(std::string const& pieceTypeString);
char GetGlyphCharFromPieceTypeAndPlayerSide(PieceType pieceType, PlayerSide side);

struct ChessPieceDefinition
{
public:
	inline static ChessPieceDefinition* s_definitions[(int)PieceType::NUM] = {};
	static void InitializeDefinitions(const char* path = "Data/Definitions/ChessPieceDefinitions.xml");
	static void ClearDefinitions();
	static ChessPieceDefinition const* GetByType(PieceType const& chessPieceType);

public:
	~ChessPieceDefinition();


	bool LoadFromXmlElement(XmlElement const& element);

	bool TryLoadStaticMesh(std::string const& filePath, PlayerSide side);


	void LoadDebugShape(PlayerSide side);



public:
	PieceType		m_type = PieceType::UNKNOWN;
	std::string		m_name = "UNKNOWN";
	char			m_glyph[2] = {'?', '?'};
	VertexBuffer*	m_vertexBufferByPlayer[2] = {};
	IndexBuffer*	m_indexBufferByPlayer[2] = {};

	Texture*		m_diffuseTextureByPlayer[2] = {};
	Texture*		m_normalTextureByPlayer[2] = {};
	Texture*		m_sgeTextureByPlayer[2] = {};

	Shader* m_shaderByPlayer[2] = {};

	float m_colliderRadius = 0.3f;
	float m_colliderHeight = 0.8f;

};

void AddDebugVertsForKing(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side);
void AddDebugVertsForQueen(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side);
void AddDebugVertsForRook(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side);
void AddDebugVertsForBishop(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side);
void AddDebugVertsForKnight(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side);
void AddDebugVertsForPawn(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side);
