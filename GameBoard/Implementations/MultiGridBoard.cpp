#include "../GameBoardInterface.h"
#include <map>
#include <map>

using namespace GameBoard;

namespace
{
	struct ConnectedGrid
	{
		ConnectedGrid(IGameBoardPtr _board) : board(std::move(_board)), north(nullptr), south(nullptr), east(nullptr), west(nullptr)
		{

		}

		/// <summary>
		/// If this grid is going out, since the pointers to neighbors is bidirectional visit all alive neighbors and unhook yourself
		/// </summary>
		~ConnectedGrid()
		{
			//Remove yourself from your northern neighbor's south link
			if (north != nullptr)
			{
				north->south = nullptr;
			}

			//Remove yourself from your eastern neighbor's west link
			if (east != nullptr)
			{
				east->west = nullptr;
			}

			//Remove yourself from your southern neighbor's north link
			if (south != nullptr)
			{
				south->north = nullptr;
			}

			//Remove yourself from your western neighbor's east link
			if (west != nullptr)
			{
				west->east = nullptr;
			}
		}

		ConnectedGrid* north;
		ConnectedGrid* east;
		ConnectedGrid* south;
		ConnectedGrid* west;

		const IGameBoardPtr board;
	};

	/// <summary>
	/// Pushes board edge cell information into neighbors' padding
	/// </summary>
	/// <param name="grid">The connected grid we will push into neighbors' padding</param>
	/// <param name="gridSize">The size of the square grid</param>
	void CopyBoardEdgesToNeighbors(ConnectedGrid& grid, Unit gridSize)
	{
		//Wanted to make padding more configurable, but doing so would require more thought for what happens in the corner regions
		constexpr int padding = 1;

		//Fill the bottom row of padding with the top row of this grid
		if(grid.north != nullptr)
		{
			const Unit myY = 0;
			const Unit theirY = gridSize;
			for (int x = 0; x < gridSize; ++x)
			{
				bool value = grid.board->GetCurrentCell(Coord{ x, myY });
				grid.north->board->SetCell(Coord{ x, theirY }, value);
			}
		}

		//Fill the top row of padding with the bottom row of this grid
		if (grid.south != nullptr)
		{
			const Unit myY = gridSize-1;
			const Unit theirY = -1;
			for (int x = 0; x < gridSize; ++x)
			{
				bool value = grid.board->GetCurrentCell(Coord{ x, myY });
				grid.south->board->SetCell(Coord{ x, theirY }, value);
			}
		}

		//Fill the left column of padding with the right column of this grid
		if (grid.east != nullptr)
		{
			const Unit myX = gridSize - 1;
			const Unit theirX = -1;
			for (int y = 0; y < gridSize; ++y)
			{
				bool value = grid.board->GetCurrentCell(Coord{ myX, y });
				grid.east->board->SetCell(Coord{ theirX, y }, value);
			}
		}

		//Fill the right column of padding with the left column of this grid
		if (grid.west != nullptr)
		{
			const Unit myX = 0;
			const Unit theirX = gridSize;
			for (int y = 0; y < gridSize; ++y)
			{
				bool value = grid.board->GetCurrentCell(Coord{ myX, y });
				grid.west->board->SetCell(Coord{ theirX, y }, value);
			}
		}

		//grab northwest corner and put in southeast
		if (grid.west != nullptr && grid.north != nullptr)
		{
			ConnectedGrid* northWest = grid.north->west == nullptr ? grid.west->north : grid.north->west;
			
			if (northWest != nullptr)
			{
				//While we're here, do a debug sanity check to make sure we did indeed hook everything up properly.
				assert(grid.north->west == nullptr || grid.west->north == nullptr || 
					(grid.north->west == northWest && grid.west->north == northWest));


				bool value = grid.board->GetCurrentCell(Coord{ 0, 0 });
				northWest->board->SetCell(Coord{ gridSize, gridSize }, value);
			}
		}

		//grab northeast corner and put in southwest
		if (grid.east != nullptr && grid.north != nullptr)
		{
			ConnectedGrid* northEast = grid.north->east == nullptr ? grid.east->north : grid.north->east;

			if (northEast != nullptr)
			{
				//While we're here, do a debug sanity check to make sure we did indeed hook everything up properly.
				assert(grid.north->east == nullptr || grid.east->north == nullptr ||
					(grid.north->east == northEast && grid.east->north == northEast));

				bool value = grid.board->GetCurrentCell(Coord{ gridSize - 1, 0 });
				northEast->board->SetCell(Coord{ -1, gridSize }, value);
			}
		}

		//grab southeast corner and put in northwest
		if (grid.east != nullptr && grid.south != nullptr)
		{
			ConnectedGrid* southEast = grid.south->east == nullptr ? grid.east->south : grid.south->east;

			if (southEast != nullptr)
			{
				//While we're here, do a debug sanity check to make sure we did indeed hook everything up properly.
				assert(grid.south->east == nullptr || grid.east->south == nullptr ||
					(grid.south->east == southEast && grid.east->south == southEast));

				bool value = grid.board->GetCurrentCell(Coord{ gridSize - 1, gridSize - 1 });
				southEast->board->SetCell(Coord{ -1, -1 }, value);
			}
		}

		//grab southwest corner and put in northeast
		if (grid.west != nullptr && grid.south != nullptr)
		{
			ConnectedGrid* southWest = grid.south->west == nullptr ? grid.west->south : grid.south->west;

			if (southWest != nullptr)
			{
				//While we're here, do a debug sanity check to make sure we did indeed hook everything up properly.
				assert(grid.south->west == nullptr || grid.west->south == nullptr ||
					(grid.south->west == southWest && grid.west->south == southWest));

				bool value = grid.board->GetCurrentCell(Coord{ 0, gridSize - 1 });
				southWest->board->SetCell(Coord{ gridSize, -1 }, value);
			}
		}
	}

	std::pair<Coord, Coord> GetMacroAndLocalCoordFromParentCoord(const Coord& position, Unit gridSize)
	{
		GameBoard::Coord macroCoord{ position.x / gridSize, position.y / gridSize };
		GameBoard::Coord localCoord{ position.x % gridSize, position.y % gridSize };
		if (localCoord.x < 0)
		{
			localCoord.x += gridSize;
			macroCoord.x -= 1;
		}
		if (localCoord.y < 0)
		{
			localCoord.y += gridSize;
			macroCoord.y -= 1;
		}

		return std::make_pair(macroCoord, localCoord);
	}

	class MultiGridBoard : public IGameBoard
	{
	public:
		MultiGridBoard(GameBoardCreationFn subBoardCreationFn):
			m_subBoardCreationFn(subBoardCreationFn),
			m_gridSize(subBoardCreationFn !=nullptr ? subBoardCreationFn()->MaximumBoardLength() : 0) // To get the grid size, just make one of the sub boards and ask it
		{
			if (m_subBoardCreationFn == nullptr)
			{
				//Really bad case but nobody should do this unless they're being malicious. In either case I'll just make it so you can't make cells
				//if you do this as there's no boards to put them on.
				std::cout << "MultiGridBoard was not passed a sub board creation function! This board is invalid";
			}

			Clear();
		}

		/// <summary>
		/// remove all the child grids
		/// </summary>
		void Clear()
		{
			m_connectedGrids.clear();
		}

		/// <summary>
		/// If the multi grid is truly empty then it will have removed all its child grids
		/// </summary>
		/// <returns></returns>
		bool Empty()
		{
			return m_connectedGrids.empty();
		}

		/// <summary>
		/// Find the sub-board and call it's get cell. If we can't find the board we can safely assume it is a dead cell.
		/// This is expensive since we need to traverse the map every time.
		/// </summary>
		bool GetCell(const Coord& position) const
		{
			auto [macroCoord, localCoord] = GetMacroAndLocalCoordFromParentCoord(position, m_gridSize);

			auto foundGrid = m_connectedGrids.find(macroCoord);
			if (foundGrid != m_connectedGrids.end())
			{
				foundGrid->second.board->GetCell(localCoord);
			}

			return false;
		}

		/// <summary>
		/// Find the sub-board and call it's get cell. If we can't find the board we can safely assume it is a dead cell.
		/// This is expensive since we need to traverse the map every time.
		/// </summary>
		bool GetCurrentCell(const Coord& position) const
		{
			auto [macroCoord, localCoord] = GetMacroAndLocalCoordFromParentCoord(position, m_gridSize);

			auto foundGrid = m_connectedGrids.find(macroCoord);
			if (foundGrid != m_connectedGrids.end())
			{
				foundGrid->second.board->GetCurrentCell(localCoord);
			}

			return false;
		}

		/// <summary>
		/// Find the sub-board and call it's set cell. If you're setting the cell to dead and we can't find a board, it's a no-op
		/// If you need to set a cell to alive and we don't have a board for it, create it. This is expensive since we need to traverse
		/// the map every time.
		/// </summary>
		void SetCell(const Coord& position, bool value)
		{
			auto [macroCoord, localCoord] = GetMacroAndLocalCoordFromParentCoord(position, m_gridSize);

			auto foundGrid = m_connectedGrids.find(macroCoord);
			if (foundGrid != m_connectedGrids.end())
			{
				foundGrid->second.board->SetCell(localCoord, value);
			}
			else if (value == true && m_subBoardCreationFn != nullptr)
			{
				//we need to make a new board in this case, but only if we are actually creating a cell.
				CreateAndHookUpBoard(macroCoord).board->SetCell(localCoord, value);
			}
		}

		/// <summary>
		/// We should support any grid location in the 64 bit space
		/// </summary>
		/// <returns>maximum allowable length</returns>
		Unit MaximumBoardLength()
		{
			return std::numeric_limits<Unit>::max();
		}

		/// <summary>
		/// Performs the intrusive operations to clean up boards that are far away from live cells and copy cell locations from adjacent boards
		/// to padding
		/// </summary>
		void FinishCurrentGeneration()
		{
			auto gridIt = m_connectedGrids.begin();
			while (gridIt != m_connectedGrids.end())
			{
				bool isEmpty = gridIt->second.board->Empty();

				//Delete boards which have no cells on them after the generation ends, and have no neighboring grids with cells in them either
				if (isEmpty &&
					(gridIt->second.north == nullptr || gridIt->second.north->board->Empty()) &&
					(gridIt->second.south == nullptr || gridIt->second.south->board->Empty()) &&
					(gridIt->second.east == nullptr || gridIt->second.east->board->Empty()) &&
					(gridIt->second.west == nullptr || gridIt->second.west->board->Empty()))
				{
					gridIt = m_connectedGrids.erase(gridIt);
				}
				else
				{
					//If this board isn't empty, then make sure it has neighbors
					if (!isEmpty)
					{
						HookUpBoard(gridIt->first, gridIt->second);
					}
					CopyBoardEdgesToNeighbors(gridIt->second, m_gridSize);
					++gridIt;
				}
			}

			gridIt = m_connectedGrids.begin();
			while (gridIt != m_connectedGrids.end())
			{
				//Stores if the board was empty before finalizing the current generation, so we can check
				//if this grid was empty and now is no longer empty
				//bool wasEmpty = gridIt->second.board->Empty();

				gridIt->second.board->FinishCurrentGeneration();
				++gridIt;
			}
		}

		/// <summary>
		/// This one just walks the grid and looks at itself and the adjacent cells
		/// </summary>
		void IterateCurrentGenerationBoard(GameSimFn gameSim)
		{
			for (auto& grid : m_connectedGrids)
			{
				if (!grid.second.board->Empty())
				{
					grid.second.board->IterateCurrentGenerationBoard(gameSim);
				}
			}
		}

		/// <summary>
		/// Walk all the connected grids and print them out
		/// </summary>
		void IterateCurrentGenerationAliveCells(const Coord& parentCoord, BoardIteratorFn fn) const
		{
			for (auto& grid : m_connectedGrids)
			{
				if (!grid.second.board->Empty())
				{
					Coord aliveCellCoord = {	grid.first.x * m_gridSize + parentCoord.x,
												grid.first.y * m_gridSize + parentCoord.y };
					grid.second.board->IterateCurrentGenerationAliveCells(aliveCellCoord, fn);
				}
			}
		}

	private:
		/// <summary>
		/// If we need a new board, make a new one in the sparse grid and hook it up to any adjacent existing boards
		/// </summary>
		/// <param name="macroCoord">The coord that represents the sparse coordinate system the boards live in</param>
		ConnectedGrid& CreateAndHookUpBoard(const Coord& macroCoord)
		{
			ConnectedGrid& newBoard = m_connectedGrids.emplace(macroCoord, m_subBoardCreationFn()).first->second;

			HookUpBoard(macroCoord, newBoard);

			return newBoard;
		}

		/// <summary>
		/// I'm going to proactively create neighbors when we specifically make 
		/// </summary>
		/// <param name="macroCoord"></param>
		/// <param name="board"></param>
		void HookUpBoard(const Coord& macroCoord, ConnectedGrid& board)
		{
			if (board.north == nullptr)
			{
				auto north = m_connectedGrids.emplace(Coord{ macroCoord.x, macroCoord.y - 1 }, m_subBoardCreationFn());
				board.north = &north.first->second;
				north.first->second.south = &board;
			}

			if (board.south == nullptr)
			{
				auto south = m_connectedGrids.emplace(Coord{ macroCoord.x, macroCoord.y + 1 }, m_subBoardCreationFn());
				board.south = &south.first->second;
				south.first->second.north = &board;
			}

			if (board.east == nullptr)
			{
				auto east = m_connectedGrids.emplace(Coord{ macroCoord.x + 1, macroCoord.y }, m_subBoardCreationFn());
				board.east = &east.first->second;
				east.first->second.west = &board;
			}

			if (board.west == nullptr)
			{
				auto west = m_connectedGrids.emplace(Coord{ macroCoord.x - 1, macroCoord.y }, m_subBoardCreationFn());
				board.west = &west.first->second;
				west.first->second.east = &board;
			}
		}

		const Unit m_gridSize;
		GameBoardCreationFn m_subBoardCreationFn;
		std::map<Coord, ConnectedGrid, LessCoord> m_connectedGrids;
	};
}

IGameBoardPtr GameBoard::CreateMultiGridBoard(GameBoardCreationFn subBoardCreationFn)
{
	return std::make_unique<MultiGridBoard>(subBoardCreationFn);
}