#include "../GameBoardInterface.h"
#include <bitset>

namespace GameBoard
{
	template<unsigned int gridSize>
	class StaticGridBoard : public IGameBoard
	{
	private:
		static constexpr unsigned int paddingSize = 1;
		static constexpr unsigned int gridSizeWithPadding = gridSize + paddingSize * 2;
		static constexpr unsigned int gridSize1D = gridSize * gridSize;
		static constexpr unsigned int gridSizeWithPadding1D= gridSizeWithPadding * gridSizeWithPadding;
		using GridBits = std::bitset<gridSizeWithPadding1D>;

	public:
		StaticGridBoard()
		{
			Clear();
		}

		void Clear()
		{
			swapChain = false;
			m_gridBits[0].reset();
			m_gridBits[1].reset();
		}

		/// <summary>
		/// Create a cell at the position laid out. Since the grid is statically allocated to a specific size, cells at coordinates larger than
		/// the grid size will be discarded.
		/// </summary>
		/// <param name="position">The position of the alive cell we wish to create. Between [0, gridSize]</param>
		void CreateCell(const Coord& position)
		{
			if (position.x >= 0 && position.y >= 0  &&
				position.x < gridSize && position.y < gridSize)
			{
				m_gridBits[!swapChain].set(Get1DIndexFromCoord(position, gridSizeWithPadding, paddingSize));
			}
			else
			{
				int outputboardsize = gridSize;
				std::cout << "Cell at position " << position.x << ", " << position.y << " cannot be created because it is outside the bounds of this board. board size = " << outputboardsize << std::endl;
			}
		}

		/// <summary>
		/// Need to know when to swap to the second board
		/// </summary>
		void FinishCurrentGeneration()
		{
			swapChain = !swapChain;
		}

		/// <summary>
		/// This one just walks the grid and looks at itself and the adjacent cells
		/// </summary>
		/// <param name="gameSim">function that runs the game of life</param>
		void IterateCurrentGenerationBoard(GameSimFunction gameSim)
		{
			Coord placement = { 0,0 };
			for (unsigned int x = 0; x < gridSize; ++x)
			{
				for (unsigned int y = 0; y < gridSize; ++y)
				{
					Unit index = Get1DIndexFromCoord(Coord{ x, y }, gridSizeWithPadding, paddingSize);
					unsigned int numNeighborsSet =
						m_gridBits[swapChain].test(index - gridSizeWithPadding - 1) +
						m_gridBits[swapChain].test(index - gridSizeWithPadding - 0) +
						m_gridBits[swapChain].test(index - gridSizeWithPadding + 1) +
						m_gridBits[swapChain].test(index - 1) +
						m_gridBits[swapChain].test(index + 1) +
						m_gridBits[swapChain].test(index + gridSizeWithPadding - 1) +
						m_gridBits[swapChain].test(index + gridSizeWithPadding - 0) +
						m_gridBits[swapChain].test(index + gridSizeWithPadding + 1);

					bool alive = false;
					gameSim(m_gridBits[swapChain][index], numNeighborsSet, alive);

					m_gridBits[!swapChain][index] = alive;
				}
			}
		}

		/// <summary>
		/// Walk the grid, report locations of bits that are set.
		/// </summary>
		/// <param name="fn">The function to run on all the alive cells.</param>
		void IterateCurrentGenerationAliveCells(std::function<void(const Coord&)> fn) const
		{
			for (size_t i = 0; i < m_gridBits[swapChain].size(); ++i)
			{
				if (m_gridBits[swapChain].test(i) == true)
				{
					fn(GetCoordFrom1DIndex(i, gridSizeWithPadding, paddingSize));
				}
			}
		}

	private:
		bool swapChain;
		GridBits m_gridBits[2];
	};

	IGameBoardPtr CreateStaticGridBoard64bit()
	{
		return std::make_unique<StaticGridBoard<6>>();
	}
}