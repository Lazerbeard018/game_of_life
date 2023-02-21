#include "GameBoardCoord.h"

namespace GameBoard
{
	Unit Get1DIndexFromCoord(const Coord& coord, Unit columnSize, Unit padding)
	{
#if defined(_DEBUG)
		//Do some bounds checking. By the time we convert to a 1D array index no dimension bounds can exceed the size of a 32 bit long
		//or we will overflow a 64 bit int.
		if (coord.y + padding >= std::numeric_limits<long>::max() ||
			coord.x + padding >= std::numeric_limits<long>::max())
		{
			std::cout << "coord is out of bounds at (" << coord.x << ", " << coord.y << ")" << std::endl;
			return 0;
		}
#endif

		return (coord.x + padding) + columnSize * (coord.y + padding);
	}

	Coord GetCoordFrom1DIndex(Unit index, Unit columnSize, Unit padding)
	{
		return Coord{ index % columnSize - padding, index / columnSize - padding };
	}
}