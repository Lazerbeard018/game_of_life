#include "Game.h"

namespace
{
	/// <summary>
	/// Codes the rules for cell death or life for the game of life. Cells only care about information on the cells near it, so
	/// the cell rules can be calculated relative to each cell
	/// </summary>
	/// <param name="alive">If the current cell is alive or dead</param>
	/// <param name="aliveNeighbors">How many relatives are alive or dead</param>
	/// <returns>If the current cell should be alive or dead in the next generation</returns>
	bool GameOfLifeCellRules(bool alive, unsigned char aliveNeighbors)
	{
		if (alive)
		{
			//If an "alive" cell had less than 2 or more than 3 alive neighbors(in any of the 8 surrounding cells), it becomes dead.
			return aliveNeighbors == 2 || aliveNeighbors == 3;
		}
		else
		{
			//If a "dead" cell had *exactly* 3 alive neighbors, it becomes alive.
			return aliveNeighbors == 3;
		}
	}
}

void Game::RunGameOfLifeGeneration(GameBoard::IGameBoard& gameBoard)
{
	gameBoard.IterateCurrentGenerationBoard([](bool alive, unsigned char aliveRelatives, bool& aliveNextGen)
		{
			aliveNextGen = GameOfLifeCellRules(alive, aliveRelatives);
		});
	gameBoard.FinishCurrentGeneration();
}
