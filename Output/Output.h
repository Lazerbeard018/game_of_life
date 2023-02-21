#pragma once
#include "../GameBoard/GameBoardInterface.h"

namespace Output
{
	void PrintGameBoardToStream(std::ostream& stream, const GameBoard::IGameBoard& gameBoard);
	void PrintGameBoardToStdOutput(const GameBoard::IGameBoard& gameBoard);
	void PrintGameBoardToFile(std::filesystem::path filename, const GameBoard::IGameBoard& gameBoard);
}