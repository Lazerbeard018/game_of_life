#include "../GameBoardInterface.h"
#include <set>

namespace GameBoard
{
	class SimpleAliveCellListBoard : public IGameBoard
	{
	private:
		/// <summary>
		/// Less than operator to help comparison and iterators. 
		/// A coord is less than if is on a row lower than the comparer.If they are on the same row, 
		/// compare the column.
		/// </summary>
		class LessCoord
		{
		public:
			bool operator()(const Coord& lhs, const Coord& rhs) const
			{
				if (lhs.y == rhs.y)
				{
					return lhs.x < rhs.x;
				}
				return lhs.y < rhs.y;
			}
		};

		using CoordSet = std::set<Coord, LessCoord>;

	public:
		void Clear()
		{
			m_aliveCells.clear();
		}

		/// <summary>
		/// Since this only stores the alive cells, create cell is trivial.
		/// </summary>
		/// <param name="position">The position of the alive cell we wish to create.</param>
		void CreateCell(const Coord& position)
		{
			m_aliveCells.insert(position);
		}

		/// <summary>
		/// Since this only stores the cells that are alive in the first place, running a function only on the alive cells is trivial.
		/// </summary>
		/// <param name="fn">The function to run on all the alive cells.</param>
		void IterateCurrentGenerationAliveCells(std::function<void (const Coord&)> fn) const
		{
			for (auto aliveCellIt : m_aliveCells)
			{
				fn(aliveCellIt);
			}
		}

	private:
		CoordSet m_aliveCells;
	};

	IGameBoardPtr GameBoard::CreateSimpleAliveCellListBoard()
	{
		return std::make_unique<SimpleAliveCellListBoard>();
	}
}