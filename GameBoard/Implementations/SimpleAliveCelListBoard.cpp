#include "../GameBoardInterface.h"
#include <set>

using namespace GameBoard;

namespace
{
	class SimpleAliveCellListBoard : public IGameBoard
	{
	public:
		void Clear()
		{
			m_aliveCells.clear();
		}

		bool Empty()
		{
			return m_aliveCells.empty();
		}

		/// <summary>
		/// Since this only stores the alive cells, get cell is trivial.
		/// </summary>
		/// <param name="position">The position of the alive cell we wish to create.</param>
		bool GetCell(const Coord& position) const
		{
			return m_aliveCells.contains(position);
		}

		/// <summary>
		/// Since this only stores the alive cells, get cell is trivial.
		/// </summary>
		/// <param name="position">The position of the alive cell we wish to create.</param>
		bool GetCurrentCell(const Coord& position) const
		{
			return m_aliveCells.contains(position);
		}

		/// <summary>
		/// Since this only stores the alive cells, create cell is trivial.
		/// </summary>
		/// <param name="position">The position of the alive cell we wish to create.</param>
		void SetCell(const Coord& position, bool value)
		{
			if (value == true)
			{
				m_aliveCells.insert(position);
			}
			else
			{
				m_aliveCells.erase(position);
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
		/// Does nothing
		/// </summary>
		void FinishCurrentGeneration() {}

		/// <summary>
		/// Does nothing
		/// </summary>
		/// <param name="gameSim"></param>
		void IterateCurrentGenerationBoard(GameSimFn gameSim) {}

		/// <summary>
		/// Since this only stores the cells that are alive in the first place, running a function only on the alive cells is trivial.
		/// </summary>
		/// <param name="fn">The function to run on all the alive cells.</param>
		void IterateCurrentGenerationAliveCells(const Coord& parentCoord, BoardIteratorFn fn) const
		{
			for (auto aliveCellIt : m_aliveCells)
			{
				fn(aliveCellIt);
			}
		}

	private:
		using CoordSet = std::set<Coord, LessCoord>;
		CoordSet m_aliveCells;
	};
}

IGameBoardPtr GameBoard::CreateSimpleAliveCellListBoard()
{
	return std::make_unique<SimpleAliveCellListBoard>();
}