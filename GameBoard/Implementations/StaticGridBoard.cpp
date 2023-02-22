#include "../GameBoardInterface.h"
#include <bitset>

using namespace GameBoard;

namespace 
{
	/// <summary>
	/// This is another simple board that I don't expect to use as my final set, however I do think it will be a building block.
	/// This game board is statically allocated and has a grid of a fixed size. As such it is rather inflexible by itself.
	/// It won't support big integers and within this grid we are not being space efficient (we are storing data to account
	/// for empty spaces) but it's very easy to run the rules of the game of life on such a board, since we can iterate all the locations
	/// and compare adjacent spaces.
	/// </summary>
	/// <typeparam name="gridSize">The size of the grid. The memory footprint of this board will be gridSize^2 * 2, so a 1000x1000 
	/// grid would take .25MB and a 4000x4000 grid would take 4MB</typeparam>
	template<int gridSize>
	class StaticGridBoard : public IGameBoard
	{
	private:
		static constexpr int paddingSize = 1;
		static constexpr int gridSizeWithPadding = gridSize + paddingSize * 2;
		static constexpr int gridSizeWithPadding1D= gridSizeWithPadding * gridSizeWithPadding;
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
		/// The idea is to check if any bits are set. We don't know when this is being called so I don't want to call a grid board
		/// empty unless both sets of bits have ended up being cleared. This check also includes the padding bits, which I'm
		/// fine with because if any pad bits are set that means there are cells very near to this board which could spill over
		/// </summary>
		/// <returns>If the grid has zero bits set, even on its padding</returns>
		bool Empty()
		{
			return m_gridBits[swapChain].none() && m_gridBits[!swapChain].none();
		}

		/// <summary>
		/// Get a cell's alive status at the position. Since the grid is statically allocated to a specific size, cells at coordinates larger than
		/// the grid size will be discarded.
		/// </summary>
		/// <param name="position">The position of the cell we wish to check. Between [0, gridSize]</param>
		bool GetCell(const Coord& position) const
		{
			Unit coord1D = 0;
			if (Get1DIndexFromCoord(position, gridSizeWithPadding, paddingSize, coord1D) && coord1D < gridSizeWithPadding1D)
			{
				return m_gridBits[swapChain].test(coord1D);
			}

			return false;
		}

		/// <summary>
		/// Get a cell's alive status at the position. Since the grid is statically allocated to a specific size, cells at coordinates larger than
		/// the grid size will be discarded.
		/// </summary>
		/// <param name="position">The position of the cell we wish to check. Between [0, gridSize]</param>
		bool GetCurrentCell(const Coord& position) const
		{
			Unit coord1D = 0;
			if (Get1DIndexFromCoord(position, gridSizeWithPadding, paddingSize, coord1D) && coord1D < gridSizeWithPadding1D)
			{
				return m_gridBits[!swapChain].test(coord1D);
			}

			return false;
		}

		/// <summary>
		/// Set a cell's alive status at the position laid out. Since the grid is statically allocated to a specific size, cells at coordinates larger than
		/// the grid size will be discarded.
		/// </summary>
		/// <param name="position">The position of the alive cell we wish to set. Between [0, gridSize]</param>
		void SetCell(const Coord& position, bool value)
		{
			Unit coord1D = 0;
			if (Get1DIndexFromCoord(position, gridSizeWithPadding, paddingSize, coord1D) && coord1D < gridSizeWithPadding1D)
			{
				m_gridBits[!swapChain].set(coord1D, value);
			}
		}

		/// <summary>
		/// These are constrained to the size of the grid we allocated for them
		/// </summary>
		/// <returns>maximum allowable length</returns>
		Unit MaximumBoardLength()
		{
			return gridSize;
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
		void IterateCurrentGenerationBoard(GameSimFn gameSim)
		{
			//Make my own array of bits which has a pattern for the areas we need to check for live cells.
			GridBits testPattern = 0b111 | 0b101 << gridSizeWithPadding | 0b111 << gridSizeWithPadding * 2;
			GridBits testedBits = 0;
			Unit index = 0;
			size_t numNeighborsSet = 0;
			bool alive = false;

			Coord placement = { 0,0 };
			for (placement.y = 0; placement.y < gridSize; ++placement.y)
			{
				for (placement.x = 0; placement.x < gridSize; ++placement.x)
				{
					testedBits = m_gridBits[swapChain] & testPattern;
					numNeighborsSet = testedBits.count();

					Get1DIndexFromCoord(placement, gridSizeWithPadding, paddingSize, index);

					gameSim(m_gridBits[swapChain].test(index), numNeighborsSet, alive);

					m_gridBits[!swapChain].set(index, alive);

					testPattern = testPattern << 1;
				}
				testPattern = testPattern << paddingSize*2;
			}

			//Original implementation, the above was found to be faster.
			//Coord placement = { 0,0 };
			//for (unsigned int x = 0; x < gridSize; ++x)
			//{
			//	for (unsigned int y = 0; y < gridSize; ++y)
			//	{
			//		Unit index = 0;
			//		Get1DIndexFromCoord(Coord{ x, y }, gridSizeWithPadding, paddingSize, index);

			//		unsigned int numNeighborsSet =
			//			m_gridBits[swapChain].test(index - gridSizeWithPadding - 1) +
			//			m_gridBits[swapChain].test(index - gridSizeWithPadding - 0) +
			//			m_gridBits[swapChain].test(index - gridSizeWithPadding + 1) +
			//			m_gridBits[swapChain].test(index - 1) +
			//			m_gridBits[swapChain].test(index + 1) +
			//			m_gridBits[swapChain].test(index + gridSizeWithPadding - 1) +
			//			m_gridBits[swapChain].test(index + gridSizeWithPadding - 0) +
			//			m_gridBits[swapChain].test(index + gridSizeWithPadding + 1);

			//		bool alive = false;
			//		gameSim(m_gridBits[swapChain].test(index), numNeighborsSet, alive);

			//		m_gridBits[!swapChain].set(index, alive);
			//	}
			//}
		}

		/// <summary>
		/// Walk the grid, report locations of bits that are set.
		/// </summary>
		/// <param name="fn">The function to run on all the alive cells.</param>
		void IterateCurrentGenerationAliveCells(const Coord& parentCoord, BoardIteratorFn fn) const
		{
			for (unsigned int y = 0; y < gridSize; ++y)
			{
				for (unsigned int x = 0; x < gridSize; ++x)
				{
					Coord coord{ x, y };
					Unit index = 0;
					Get1DIndexFromCoord(coord, gridSizeWithPadding, paddingSize, index);
					if (m_gridBits[swapChain].test(index) == true)
					{
						fn(Coord{ coord.x + parentCoord.x, coord.y + parentCoord.y });
					}
				}
			}
		}

	private:
		bool swapChain;
		GridBits m_gridBits[2];
	};
}

//From testing, having the bit board being size 6 seems to be the sweet spot. This makes sense as with padding
//it aligns perfectly with 64 bit boundaries.
IGameBoardPtr GameBoard::CreateStaticGridBoard6()
{
	return std::make_unique<StaticGridBoard<6>>();
}

//made this to play around with the size of the grid just to see if there was anything better than 6 bits and there really isn't.
//That makes me happy since it's very space efficient and it lines up with what you'd expect given the hardware.
IGameBoardPtr GameBoard::CreateStaticGridBoard()
{
	return std::make_unique<StaticGridBoard<6>>();
}