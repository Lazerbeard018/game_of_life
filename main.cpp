#include "GameBoard/GameBoardInterface.h"
#include "Game/Game.h"
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
		GameBoard::IGameBoardPtr multiGridBoard = GameBoard::CreateMultiGridBoard(&GameBoard::CreateStaticGridBoard1000);

		Input::CreateGameFromStdInput(*multiGridBoard);

		for (int i = 0; i < 10; ++i)
		{
			Game::RunGameOfLifeGeneration(*multiGridBoard);
		}

		Output::PrintGameBoardToStdOutput(*multiGridBoard);
	}
	
	return 0;
}