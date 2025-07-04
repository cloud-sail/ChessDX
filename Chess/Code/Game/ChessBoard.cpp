#include "Game/ChessBoard.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Math/AABB3.hpp"

ChessBoard::ChessBoard(ChessMatch* match)
	: ChessObject(match)

{
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(1 * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(1 * sizeof(unsigned int));
	PopulateChessBoardSquares();

	m_diffuseTexture	= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Textures/Bricks_d.png");
	m_normalTexture		= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Textures/Bricks_n.png");
	m_sgeTexture		= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Textures/Bricks_sge.png");
}

ChessBoard::~ChessBoard()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;
	delete m_indexBuffer;
	m_indexBuffer = nullptr;
}

void ChessBoard::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void ChessBoard::Render(GameObjectRenderConfig const& config /*= GameObjectRenderConfig()*/) const
{
	UNUSED(config);
	g_theRenderer->SetModelConstants(GetModelToWorldTransform());
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
}

void ChessBoard::PopulateChessBoardSquares()
{
	std::vector<Vertex_PCUTBN> verts;
	std::vector<unsigned int> indexes;
	for (int rowIndex = 0; rowIndex < 8; ++rowIndex) // y
	{
		for (int columnIndex = 0; columnIndex < 8; ++columnIndex) // x
		{
			Vec3 mins = Vec3(CHESS_BOARD_SQUARE_SIZE * static_cast<float>(columnIndex), CHESS_BOARD_SQUARE_SIZE * static_cast<float>(rowIndex), -CHESS_BOARD_THINKNESS);
			Vec3 maxs = mins + Vec3(CHESS_BOARD_SQUARE_SIZE, CHESS_BOARD_SQUARE_SIZE, CHESS_BOARD_THINKNESS);
			Rgba8 color = ((rowIndex + columnIndex) % 2 == 0) ? Rgba8(50, 50, 50) : Rgba8::OPAQUE_WHITE;
			AddVertsForAABB3D(verts, indexes, AABB3(mins, maxs), color);
		}
	}

	AddVertsForAABB3D(verts, indexes, AABB3(Vec3(-CHESS_BOARD_SQUARE_SIZE * 0.4f, -CHESS_BOARD_SQUARE_SIZE * 0.4f, -0.01f - CHESS_BOARD_THINKNESS),
		Vec3(CHESS_BOARD_SQUARE_SIZE * 8.4f, CHESS_BOARD_SQUARE_SIZE * 8.4f, -0.01f)), Rgba8(140, 92, 66));


	g_theRenderer->CopyCPUToGPU(verts.data(), static_cast<unsigned int>(verts.size()) * m_vertexBuffer->GetStride(), m_vertexBuffer);
	g_theRenderer->CopyCPUToGPU(indexes.data(), static_cast<unsigned int>(indexes.size()) * m_indexBuffer->GetStride(), m_indexBuffer);
}
