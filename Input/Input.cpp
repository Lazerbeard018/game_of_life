#include "Input.h"
#include <iostream>
#include <fstream>
#include <regex>

namespace
{
	bool AddCellsFromString(const std::string& input, GameBoard::IGameBoard& gameBoard)
	{	
		//Should capture integer values and negative number symbols
		std::regex number_regex("-?\\d+");

		//Chop up ints from raw input
		auto numberBegin = std::sregex_iterator(input.begin(), input.end(), number_regex);

		if (numberBegin == std::sregex_iterator())
		{
			return false;
		}

		//Grab found ints and take pairs to create cells.
		for (std::sregex_iterator numberIt = numberBegin; numberIt != std::sregex_iterator(); ++numberIt)
		{
			long long x = atoll(numberIt->str().c_str());

			++numberIt;

			//Somehow we cut out halfway between reading integer values, so this is malformed input.
			if (numberIt == std::sregex_iterator())
			{
				return false;
			}

			long long y = atoll(numberIt->str().c_str());

			gameBoard.CreateCell({ x, y });
		}

		return true;
	}
}

void Input::CreateGameFromStream(std::istream& stream, GameBoard::IGameBoard& gameBoard)
{
	std::string input;
	std::getline(stream, input);

	//handle header
	if (input == "#Life 1.06")
	{
		std::getline(stream, input);
	}

	while (AddCellsFromString(input, gameBoard))
	{
		std::getline(stream, input);
	}

	//Signal that this version of the board is ready to be read;
	gameBoard.FinishCurrentGeneration();
}

void Input::CreateGameFromStdInput(GameBoard::IGameBoard& gameBoard)
{
	CreateGameFromStream(std::cin, gameBoard);
}

bool Input::CreateGameFromFile(std::filesystem::path filename, GameBoard::IGameBoard& gameBoard)
{
	std::fstream fileStream;
	fileStream.open(filename, std::fstream::in);

	if (!fileStream.is_open())
	{
		return false;
	}

	CreateGameFromStream(fileStream, gameBoard);

	fileStream.close();

	return true;
}
