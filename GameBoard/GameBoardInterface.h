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
		using GameSimFn = void (*)(bool alive, unsigned char aliveRelatives, bool& aliveNextGeneration);
		using BoardIteratorFn = std::function<void(const Coord&)>;

		/// <summary>
		/// Enables derived classes to be cleaned up properly
		/// </summary>
		virtual ~IGameBoard() {}

		/// <summary>
		/// Makes sure the game board is clear and ready to create a new game
		/// </summary>
		virtual void Clear() = 0;

		/// <summary>
		/// Tells us if all the cells on this board are dead. The cheapest simulation is the one you don't have to run and if a board is dead
		/// there's not much use in keeping the board.
		/// </summary>
		/// <returns>If the board is empty or not</returns>
		virtual bool Empty() = 0;

		/// <summary>
		/// Gets the status of a particular cell in a gameboard in a finished generation. This should be thread
		/// safe because this section of memory should be used as read-only for this generation
		/// </summary>
		/// <param name="position">const ref to a position on the board</param>
		virtual bool GetCell(const Coord& position) const = 0;

		/// <summary>
		/// Gets the status of a particular cell in a gameboard, at the generation currently being written.
		/// This operation likely wouldn't be thread safe if we threaded computation of sub-boards as it is 
		/// reading from the section of memory we are currently writing to
		/// </summary>
		/// <param name="position">const ref to a position on the board</param>
		virtual bool GetCurrentCell(const Coord& position) const = 0;

		/// <summary>
		/// Sets the status of a particular cell in a gameboard for the current generation
		/// </summary>
		/// <param name="position">const ref to a position on the board</param>
		/// <param name="value">the value to set the position to</param>
		virtual void SetCell(const Coord& position, bool value) = 0;

		/// <summary>
		/// Gets the size in cells a given game board can support. All boards have limits, even our "infinite" ones.
		/// </summary>
		/// <returns></returns>
		virtual Unit MaximumBoardLength() = 0;

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
		virtual void IterateCurrentGenerationBoard(GameSimFn gameSim) = 0;

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
		/// <param name="parentCoord">Allows recursive grids to offset from local coordinates</param>
		/// <param name="fn">The function to run only on alive cells in the grid.</param>
		virtual void IterateCurrentGenerationAliveCells(const Coord& parentCoord, BoardIteratorFn fn) const = 0;
	};

	using IGameBoardPtr = std::unique_ptr<IGameBoard>;
	using GameBoardCreationFn = IGameBoardPtr(*)();

	/// <summary>
	/// This is a simple board that just has a list of the live cells. It's really not meant to do game of life sim on, 
	/// but it is the most basic way I could think of to return the input of cells back to the output, so we can test
	/// that purely input and output work properly.
	/// 
	/// If I have time.. there are some properties about this board which are nice.. The storage of the alive cells is
	/// sparse in that we only record info about the cells that are alive, which is the only place where we care to
	/// do simulation. It won't be easy to figure out where the empty cells which need to generate new cells are though.
	/// 
	/// Thinking about it more if you use this board as the basis and have a recursive stack of multi grid boards, you
	/// you'd end up with something close to a quad tree. This would certainly be the lightest on memory assuming the board
	/// is sparsely populated in areas where we'd make the grid, however we still end up with the problem that the game of life
	/// cares about empty spaces as much as the occupied ones. Not unsolvable but to make it performance competitive with
	/// a grid centric approach I think would require a lot more effort and fairly large (2000x2000) grids can be created for
	/// than .5 MB
	/// </summary>
	/// <returns>A generic game board we can do game of life sim on</returns>
	IGameBoardPtr CreateSimpleAliveCellListBoard();

	IGameBoardPtr CreateStaticGridBoard6();
	IGameBoardPtr CreateStaticGridBoard1000();
	IGameBoardPtr CreateStaticGridBoard4000();
	IGameBoardPtr CreateStaticGridBoard10000();

	IGameBoardPtr CreateMultiGridBoard(GameBoardCreationFn subBoardCreationFn);


	// Other board types I was thinking about...
	// -Definitely doing something more like a real quadtree so a deeper hierarchy of multi-boards and at the bottom is something like the alive list
	//		splits would happen after the alive list in an area got too big rather than because a board was reaching its boundaries
	// -I wanted to experiment with a primitive board type similar to the static grid, but stored extra information I called "minesweeper mode".
	//		It's based off an observation that you only need 3 bits to know if a cell will be alive or dead in the next generation because once you get to
	//		4 the cell in that square is for sure dead, so instead of testing each location every generation against its adjacent neighbor you
	//      could instead keep a running tally of the live cells as you go past. I wasn't sure if this was that many less calculations though
	//		especially since the grid test can be performed with a single bitwise AND.
	// -Something like a "worm mode" where I just store adjacency information and nothing else. This is based on the idea that most game boards, even
	//		in populated areas will still have lots of empty space, since maximum 3/8 of the adjacent locations can be filled if the cell will live next
	//		generation. I'm pretty sure that my MultiGridBoard using a StaticGridBoard of gridSize = 1 is pretty much this, but I'm sure there's a more
	//		efficient direct implementation to give it a fair shot, mainly because as it is I'm taking a bit hit by so many virtual calls due to using
	//		this interface. This might be a legitimately good alternative if you end up with uniformly distributed cells across a very large area and you're 
	//		running into space concerns.
	// -An "amoeba mode" that tries to find a bounding box around cell groupings and creates a cell space that is tightly fit to those cells. This seemed
	//		interesting but the process of dividing and rejoining bounding boxes as they shift and move around seemed like a pain and computationally expensive
	//		compared to the alternatives. I'd definitely need to store the amoebas in a quadtree so maybe this is sort of an optimization on the original quadtree
	//		idea
	// -My "cheater mode" which is why I asked the question about how many generations we would strictly need to support. Based on the rules, I dont think
	//		it is possible for a given colony of cells to grow or move faster than 1 cell in a cardinal direction per generation. So if we did know the number
	//		of generations we expected to do before starting, we could make grids that matched the bounding box of our starting dataset with an additional padding
	//		equal to the expected number of generations we want to do.
	//		It's certainly not a good algorithm if this was meant to be used for a live simulation or a large number of generations,
	//		but it's closer to something you'd end up doing on pipeline code, as you usually can extract well-defined starting conditions.
	//		If you *can* get these guarantees, it would be easier to scope the memory and computation to where you know it will be used. It's 
	//		certainly not fancy, but it's simple, testable, and provides the solution needed to get the job done. The important part there is 
	//		making sure the framework is extensible so that if conditions do change it's relatively easy to go back and implement something more 
	//		robust if needed
}