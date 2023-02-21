#include "GameBoard/GameBoardInterface.h"
#include "Input/Input.h"
#include "Output/Output.h"
#include "Tests/TestEngine.h"

int __cdecl main(int argc, const char* argv[])
{
	//If we have an argument, see if it's one we know what to do with;
	if (argc == 2 && !strcmp(argv[1], "test"))
	{
		if (!strcmp(argv[1], "test"))
		{
			Tests::TestEngine engine;

			engine.RunAllTests(std::cout);
		}
	}
	//Otherwise, run the default program for the test.
	else
	{
		GameBoard::IGameBoardPtr simpleGameBoard = GameBoard::CreateSimpleAliveCellListBoard();

		Input::CreateGameFromStdInput(*simpleGameBoard);

		Output::PrintGameBoardToStdOutput(*simpleGameBoard);
	}
	
	return 0;
}