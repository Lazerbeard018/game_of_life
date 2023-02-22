#include "Output.h"
#include <fstream>
#include <iostream>

void Output::PrintGameBoardToStream(std::ostream& stream, const GameBoard::IGameBoard& gameBoard)
{
	//Write the header 
	stream << "#Life 1.06" << std::endl;

	//Iterate all the live cells and print em out
	GameBoard::Coord zero{ 0,0 };
	gameBoard.IterateCurrentGenerationAliveCells(zero, [&stream](const GameBoard::Coord& liveCellCoord)
		{
			stream << liveCellCoord.x << " " << liveCellCoord.y << std::endl;
		});
}

void PrintGameBoardToStreamViaScanlines(std::ostream& stream, const GameBoard::IGameBoard& gameBoard)
{
	//Write the header 
	stream << "#Life 1.06" << std::endl;

	//Iterate all the live cells and print em out
	GameBoard::Coord zero{ 0,0 };

	//gameBoard.GetCell();
	gameBoard.IterateCurrentGenerationAliveCells(zero, [&stream](const GameBoard::Coord& liveCellCoord)
		{
			stream << liveCellCoord.x << " " << liveCellCoord.y << std::endl;
		});
}

void Output::PrintGameBoardToStdOutput(const GameBoard::IGameBoard& gameBoard)
{
	PrintGameBoardToStream(std::cout, gameBoard);
}

void Output::PrintGameBoardToFile(std::filesystem::path filename, const GameBoard::IGameBoard& gameBoard)
{
	std::fstream fileStream;
	fileStream.open(filename, std::fstream::out);

	PrintGameBoardToStream(fileStream, gameBoard);

	fileStream.close();
}