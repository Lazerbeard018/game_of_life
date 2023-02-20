#include "GameBoard/GameBoardInterface.h"
#include "Input/Input.h"
#include "Output/Output.h"
#include "Tests/TestEngine.h"
#include <iostream>

int __cdecl main(int argc, const char* argv[])
{
	//If no arguments specified, run the default program for the test.
	if (argc <= 1)
	{
		GameBoard::IGameBoardPtr simpleGameBoard = GameBoard::CreateSimpleAliveCellListBoard();

		Input::CreateGameFromStdInput(simpleGameBoard);

		Output::PrintGameBoardToStdOutput(simpleGameBoard.get());
	}

	//If we have an argument, see if it's one we know what to do with;
	if (argc == 2)
	{
		if (!strcmp(argv[1], "test"))
		{
			Tests::TestEngine engine;

			GameBoard::IGameBoardPtr simpleGameBoard = GameBoard::CreateSimpleAliveCellListBoard();
			engine.RunAllTests(std::cout, simpleGameBoard);
		}
	}
	
	return 0;
}