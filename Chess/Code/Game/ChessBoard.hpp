#pragma once
#include "Game/ChessObject.hpp"

class ChessPiece;

class ChessBoard : public ChessObject
{
public:
	ChessBoard(ChessMatch* match);
	virtual ~ChessBoard();

	virtual void Update(float deltaSeconds) override;
	virtual void Render(GameObjectRenderConfig const& config = GameObjectRenderConfig()) const override;


	void PopulateChessBoardSquares();


protected:



};

