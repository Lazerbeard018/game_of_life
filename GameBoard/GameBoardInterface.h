#pragma once
#include "GameBoardCoord.h"
#include <functional>

namespace GameBoard
{
	/// <summary>
	/// This is meant to be a abstract class used as an interface which allows multiple representations of a game board.
	/// It should represent the functinality external users will need to interact with the game board.
	/// </summary>
	class IGameBoard
	{
	public:
		using GameSimFunction = void (*)(bool alive, unsigned char aliveRelatives, bool& aliveNextGeneration);

		/// <summary>
		/// Enables derived classes to be cleaned up properly
		/// </summary>
		virtual ~IGameBoard() {}

		/// <summary>
		/// Makes sure the game board is clear and ready to create a new game
		/// </summary>
		virtual void Clear() = 0;

		/// <summary>
		/// Creates a living "cell" at the position represented by the coordinate passed in
		/// </summary>
		/// <param name="position">const ref to a position on the board, can be any position in the GameBoard Unit space</param>
		virtual void CreateCell(const Coord& position) = 0;

		/// <summary>
		/// Allows us to do some work when we are finished with a generation in the game of life. We may need to do some cleanup between
		/// generations.
		/// </summary>
		virtual void FinishCurrentGeneration() = 0;

		/// <summary>
		/// So this is the meat of the game of life sim as far as the board is concerned. This allows access to however the board decides
		/// to store the game of life board in a way that lets us run the game of life with our rules. 
		/// </summary>
		/// <param name="gameSim">The function that will run the game of life</param>
		virtual void IterateCurrentGenerationBoard(GameSimFunction gameSim) = 0;

		/// <summary>
		/// I was really trying to figure out how to capture the contents of the grid, without directly exposing the
		/// grid's structure and without creating additional memory just to inspect things about the grid, especially since
		/// it is technically possible to fill a huge area with grid squares. Making an iterator to return is a problem since
		/// the iterator's own memory may be different depending on which gameboard I am using, and I rejected the simple solution
		/// to just return a vector of the alive cells coming from the game board because I was concerned that the vector could
		/// become very large in hypothetical situations where the game is left to run for a long time and grows a lot.
		/// This means that iterating the live cells without necessarily knowing anything about the game board is intrusive, we need
		/// to let the board do the iterating and then pass in an inspector which can run code on the live cells.
		/// </summary>
		/// <param name="fn">The function to run only on alive cells in the grid.</param>
		virtual void IterateCurrentGenerationAliveCells(std::function<void(const Coord&)> fn) const = 0;
	};

	using IGameBoardPtr = std::unique_ptr<IGameBoard>;

	/// <summary>
	/// This is a simple board that just has a list of the live cells. It's really not meant to do game of life sim on, 
	/// but it is the most basic way I could think of to return the input of cells back to the output, so we can test
	/// that purely input and output work properly.
	/// 
	/// If I have time.. there are some properties about this board which are nice.. The storage of the alive cells is
	/// sparse in that we only record info about the cells that are alive, which is the only place where we care to
	/// do simulation. It won't be easy to figure out where the empty cells which need to generate new cells are though.
	/// </summary>
	/// <returns>A generic game board we can do game of life sim on</returns>
	IGameBoardPtr CreateSimpleAliveCellListBoard();

	/// <summary>
	/// This is another simple board that I don't expect to use as my final set, however I do think it will be a building block.
	/// This game board is statically allocated and has a grid of a fixed size. As such it is rather inflexible by itself.
	/// It won't support big integers and within this grid we are not being space efficient (we are storing data to account
	/// for empty spaces) but it's very easy to run the rules of the game of life on such a board, since we can iterate all the locations
	/// and compare adjacent spaces.
	/// </summary>
	/// <returns>A generic game board we can do game of life sim on</returns>
	IGameBoardPtr CreateStaticGridBoard64bit();
}