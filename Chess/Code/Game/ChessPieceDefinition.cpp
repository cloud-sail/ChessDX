#include "Game/ChessPieceDefinition.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StaticMeshUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB3.hpp"



std::string GetStringFromPieceType(PieceType pieceType)
{
	switch (pieceType)
	{
	case PieceType::KING:
		return "King";
	case PieceType::QUEEN:
		return "Queen";
	case PieceType::ROOK:
		return "Rook";
	case PieceType::BISHOP:
		return "Bishop";
	case PieceType::KNIGHT:
		return "Knight";
	case PieceType::PAWN:
		return "Pawn";
	default:
		return "Unknown";
	}
}

PieceType GetPieceTypeFromString(std::string const& pieceTypeString)
{
	if (pieceTypeString == "King")
	{
		return PieceType::KING;
	}
	else if (pieceTypeString == "Queen")
	{
		return PieceType::QUEEN;
	}
	else if (pieceTypeString == "Rook")
	{
		return PieceType::ROOK;
	}
	else if (pieceTypeString == "Bishop")
	{
		return PieceType::BISHOP;
	}
	else if (pieceTypeString == "Knight")
	{
		return PieceType::KNIGHT;
	}
	else if (pieceTypeString == "Pawn")
	{
		return PieceType::PAWN;
	}
	else
	{
		return PieceType::UNKNOWN;
	}
}

char GetGlyphCharFromPieceTypeAndPlayerSide(PieceType pieceType, PlayerSide side)
{
	if (pieceType == PieceType::KING)
	{
		if (side == PLAYER_WHITE) return 'K';
		if (side == PLAYER_BLACK) return 'k';
	}
	else if (pieceType == PieceType::QUEEN)
	{
		if (side == PLAYER_WHITE) return 'Q';
		if (side == PLAYER_BLACK) return 'q';
	}
	else if (pieceType == PieceType::ROOK)
	{
		if (side == PLAYER_WHITE) return 'R';
		if (side == PLAYER_BLACK) return 'r';
	}
	else if (pieceType == PieceType::BISHOP)
	{
		if (side == PLAYER_WHITE) return 'B';
		if (side == PLAYER_BLACK) return 'b';
	}
	else if (pieceType == PieceType::KNIGHT)
	{
		if (side == PLAYER_WHITE) return 'N';
		if (side == PLAYER_BLACK) return 'n';
	}
	else if (pieceType == PieceType::PAWN)
	{
		if (side == PLAYER_WHITE) return 'P';
		if (side == PLAYER_BLACK) return 'p';
	}

	return '?';
}

void AddDebugVertsForKing(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side)
{
	Rgba8 color = (side == PLAYER_WHITE ? Rgba8::OPAQUE_WHITE : (side == PLAYER_BLACK ? Rgba8(255, 128, 128) : Rgba8::MAGENTA));
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.7f), 0.2f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.7f, 0.8f), 0.25f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.1f), 0.25f, 32, color);
	AddVertsForAABB3D(verts, indexes, AABB3(-0.05f, -0.1f, 0.85f, 0.05f, 0.1f, 0.90f), color);
	AddVertsForAABB3D(verts, indexes, AABB3(-0.05f, -0.025f, 0.8f, 0.05f, 0.025f, 0.95f), color);
}

void AddDebugVertsForQueen(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side)
{
	Rgba8 color = (side == PLAYER_WHITE ? Rgba8::OPAQUE_WHITE : (side == PLAYER_BLACK ? Rgba8(255, 128, 128) : Rgba8::MAGENTA));
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.9f), 0.15f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.65f, 0.7f), 0.18f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.85f, 0.9f), 0.22f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.1f), 0.22f, 32, color);
	AddVertsForSphere3D(verts, indexes, Vec3(0.f, 0.f, 0.92f), 0.05f, color);
}

void AddDebugVertsForRook(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side)
{
	Rgba8 color = (side == PLAYER_WHITE ? Rgba8::OPAQUE_WHITE : (side == PLAYER_BLACK ? Rgba8(255, 128, 128) : Rgba8::MAGENTA));
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.7f), 0.18f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.2f), 0.25f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.55f, 0.7f), 0.25f, 32, color);
}

void AddDebugVertsForBishop(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side)
{
	Rgba8 color = (side == PLAYER_WHITE ? Rgba8::OPAQUE_WHITE : (side == PLAYER_BLACK ? Rgba8(255, 128, 128) : Rgba8::MAGENTA));
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.7f), 0.1f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.10f), 0.2f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.63f, 0.68f), 0.16f, 32, color);
	AddVertsForSphere3D(verts, indexes, Vec3(0.f, 0.f, 0.7f), 0.13f, color);
	AddVertsForSphere3D(verts, indexes, Vec3(0.f, 0.f, 0.86f), 0.05f, color);
}

void AddDebugVertsForKnight(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side)
{
	Rgba8 color = (side == PLAYER_WHITE ? Rgba8::OPAQUE_WHITE : (side == PLAYER_BLACK ? Rgba8(255, 128, 128) : Rgba8::MAGENTA));
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.7f), 0.2f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.1f), 0.25f, 32, color);
	AddVertsForAABB3D(verts, indexes, AABB3(-0.15f, -0.15f, 0.45f, 0.35f, 0.15f, 0.75f), color);
	AddVertsForAABB3D(verts, indexes, AABB3(-0.15f, -0.15f, 0.6f, 0.05f, 0.15f, 0.85f), color);
}

void AddDebugVertsForPawn(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, PlayerSide side)
{
	Rgba8 color = (side == PLAYER_WHITE ? Rgba8::OPAQUE_WHITE : (side == PLAYER_BLACK ? Rgba8(255, 128, 128) : Rgba8::MAGENTA));

	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.5f), 0.13f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.f, 0.10f), 0.2f, 32, color);
	AddVertsForCylinderZ3D(verts, indexes, Vec2::ZERO, FloatRange(0.5f, 0.6f), 0.16f, 32, color);
	AddVertsForSphere3D(verts, indexes, Vec3(0.f, 0.f, 0.7f), 0.13f, color);
}

void ChessPieceDefinition::InitializeDefinitions(const char* path /*= "Data/Definitions/ChessPieceDefinitions.xml"*/)
{
	ClearDefinitions();

	XmlDocument document;
	XmlResult result = document.LoadFile(path);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open xml file: \"%s\"", path));

	XmlElement* rootElement = document.RootElement();
	GUARANTEE_OR_DIE(rootElement, Stringf("No elements in xml file: \"%s\"", path));

	XmlElement* pieceDefElement = rootElement->FirstChildElement();
	while (pieceDefElement != nullptr)
	{
		std::string elementName = pieceDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "ChessPieceDefinition", Stringf("Root child element in %s was <%s>, must be <ChessPieceDefinition>!", path, elementName.c_str()));
		ChessPieceDefinition* newPieceDef = new ChessPieceDefinition();
		
		newPieceDef->LoadFromXmlElement(*pieceDefElement);
		s_definitions[(int)newPieceDef->m_type] = newPieceDef;

		pieceDefElement = pieceDefElement->NextSiblingElement();
	}
}

void ChessPieceDefinition::ClearDefinitions()
{
	for (int i = 0; i < static_cast<int>(PieceType::NUM); ++i) 
	{
		delete s_definitions[i];
		s_definitions[i] = nullptr;
	}
}

ChessPieceDefinition const* ChessPieceDefinition::GetByType(PieceType const& chessPieceType)
{
	return s_definitions[(int)chessPieceType];
}

ChessPieceDefinition::~ChessPieceDefinition()
{
	delete m_vertexBufferByPlayer[PLAYER_WHITE];
	m_vertexBufferByPlayer[PLAYER_WHITE] = nullptr;
	delete m_vertexBufferByPlayer[PLAYER_BLACK];
	m_vertexBufferByPlayer[PLAYER_BLACK] = nullptr;
	delete m_indexBufferByPlayer[PLAYER_WHITE];
	m_indexBufferByPlayer[PLAYER_WHITE] = nullptr;
	delete m_indexBufferByPlayer[PLAYER_BLACK];
	m_indexBufferByPlayer[PLAYER_BLACK] = nullptr;
}

bool ChessPieceDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);
	
	m_type = GetPieceTypeFromString(m_name);
	
	m_glyph[PLAYER_WHITE] = GetGlyphCharFromPieceTypeAndPlayerSide(m_type, PLAYER_WHITE);
	m_glyph[PLAYER_BLACK] = GetGlyphCharFromPieceTypeAndPlayerSide(m_type, PLAYER_BLACK);

	// Initialize Buffer
	m_vertexBufferByPlayer[PLAYER_WHITE] = g_theRenderer->CreateVertexBuffer(1 * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_vertexBufferByPlayer[PLAYER_BLACK] = g_theRenderer->CreateVertexBuffer(1 * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_indexBufferByPlayer[PLAYER_WHITE] = g_theRenderer->CreateIndexBuffer(1 * sizeof(unsigned int));
	m_indexBufferByPlayer[PLAYER_BLACK] = g_theRenderer->CreateIndexBuffer(1 * sizeof(unsigned int));



	if (!TryLoadStaticMesh(ParseXmlAttribute(element, "whiteModel", "INVALID_PATH"), PLAYER_WHITE))
	{
		LoadDebugShape(PLAYER_WHITE);
	}

	if (!TryLoadStaticMesh(ParseXmlAttribute(element, "blackModel", "INVALID_PATH"), PLAYER_BLACK))
	{
		LoadDebugShape(PLAYER_BLACK);
	}

	return true;
}


bool ChessPieceDefinition::TryLoadStaticMesh(std::string const& filePath, PlayerSide side)
{
	if (filePath.empty())
	{
		return false;
	}

	XmlDocument modelXML;
	XmlResult result = modelXML.LoadFile(filePath.c_str());
	if (result != tinyxml2::XML_SUCCESS)
	{
		return false;
	}
	XmlElement* rootElement = modelXML.RootElement();
	if (rootElement == nullptr)
	{
		return false;
	}

	std::string shaderName = ParseXmlAttribute(*rootElement, "shader", "Data/Shaders/Diffuse");
	if (shaderName.empty()) shaderName = "Data/Shaders/Diffuse";

	std::string m_diffuseMapFilePath = ParseXmlAttribute(*rootElement, "diffuseMap", "DefaultDiffuse");
	if (m_diffuseMapFilePath.empty()) m_diffuseMapFilePath = "DefaultDiffuse";

	std::string m_normalMapFilePath = ParseXmlAttribute(*rootElement, "normalMap", "DefaultNormal");
	if (m_normalMapFilePath.empty()) m_normalMapFilePath = "DefaultNormal";

	std::string m_specGlossEmitMapFilePath = ParseXmlAttribute(*rootElement, "specGlossEmitMap", "DefaultSpecGlossEmit");
	if (m_specGlossEmitMapFilePath.empty()) m_specGlossEmitMapFilePath = "DefaultSpecGlossEmit";





	m_shaderByPlayer[side] = g_theRenderer->CreateOrGetShader(shaderName.c_str(), VertexType::VERTEX_PCUTBN);
	m_diffuseTextureByPlayer[side] = g_theRenderer->CreateOrGetTextureFromFile(m_diffuseMapFilePath.c_str());
	m_normalTextureByPlayer[side] = g_theRenderer->CreateOrGetTextureFromFile(m_normalMapFilePath.c_str());
	m_sgeTextureByPlayer[side] = g_theRenderer->CreateOrGetTextureFromFile(m_specGlossEmitMapFilePath.c_str());

	std::vector<Vertex_PCUTBN> verts;
	std::vector<unsigned int> indexes;

	bool isSuccess = LoadOBJFromXML(verts, filePath.c_str());
	if (!isSuccess)
	{
		return false;
	}
	
	indexes.reserve(verts.size());
	for (int i = 0; i < (int)verts.size(); ++i)
	{
		indexes.push_back(i);
	}

	g_theRenderer->CopyCPUToGPU(verts.data(), static_cast<unsigned int>(verts.size()) * m_vertexBufferByPlayer[side]->GetStride(), m_vertexBufferByPlayer[side]);
	g_theRenderer->CopyCPUToGPU(indexes.data(), static_cast<unsigned int>(indexes.size()) * m_indexBufferByPlayer[side]->GetStride(), m_indexBufferByPlayer[side]);

	return true;
}

void ChessPieceDefinition::LoadDebugShape(PlayerSide side)
{
	if (side == PLAYER_WHITE)
	{
		m_diffuseTextureByPlayer[PLAYER_WHITE]	= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Textures/Cobblestone_Diffuse.png");
		m_normalTextureByPlayer[PLAYER_WHITE]	= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Textures/Cobblestone_Normal.png");
		m_sgeTextureByPlayer[PLAYER_WHITE]		= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Textures/Cobblestone_SpecGlossEmit.png");
	}
	else if (side == PLAYER_BLACK)
	{
		m_diffuseTextureByPlayer[PLAYER_BLACK]	= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Textures/FunkyBricks_d.png");
		m_normalTextureByPlayer[PLAYER_BLACK]	= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Textures/FunkyBricks_n.png");
		m_sgeTextureByPlayer[PLAYER_BLACK]		= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Textures/FunkyBricks_sge.png");
	}


	std::vector<Vertex_PCUTBN> verts;
	std::vector<unsigned int> indexes;

	if (m_type == PieceType::KING)
	{
		AddDebugVertsForKing(verts, indexes, side);
	}
	else if (m_type == PieceType::QUEEN)
	{
		AddDebugVertsForQueen(verts, indexes, side);
	}
	else if (m_type == PieceType::ROOK)
	{
		AddDebugVertsForRook(verts, indexes, side);
	}
	else if (m_type == PieceType::BISHOP)
	{
		AddDebugVertsForBishop(verts, indexes, side);
	}
	else if (m_type == PieceType::KNIGHT)
	{
		AddDebugVertsForKnight(verts, indexes, side);
	}
	else if (m_type == PieceType::PAWN)
	{
		AddDebugVertsForPawn(verts, indexes, side);
	}

	g_theRenderer->CopyCPUToGPU(verts.data(), static_cast<unsigned int>(verts.size()) * m_vertexBufferByPlayer[side]->GetStride(), m_vertexBufferByPlayer[side]);
	g_theRenderer->CopyCPUToGPU(indexes.data(), static_cast<unsigned int>(indexes.size()) * m_indexBufferByPlayer[side]->GetStride(), m_indexBufferByPlayer[side]);
}
