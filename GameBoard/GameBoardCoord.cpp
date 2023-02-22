#include "GameBoardCoord.h"

namespace GameBoard
{
	bool Get1DIndexFromCoord(const Coord& coord, Unit columnSize, Unit padding, Unit& output)
	{
#if defined(_DEBUG)
		//Do some bounds checking. By the time we convert to a 1D array index no dimension bounds can exceed the size of a 32 bit long
		//or we will overflow a 64 bit int. 1D indexes should also be 0 based so report on weirdness going on there.
		if ((coord.y + padding) >= columnSize* columnSize ||
			(coord.x + padding) >= columnSize* columnSize ||
			(coord.y + padding) < 0 ||
			(coord.x + padding) < 0 )
		{
			std::cout << "coord is out of bounds at (" << coord.x << ", " << coord.y << ")" << std::endl;
			output = 0;
			return false;
		}
#endif

		output = (coord.x + padding) + columnSize * (coord.y + padding);

		return true;
	}

	Coord GetCoordFrom1DIndex(Unit index, Unit columnSize, Unit padding)
	{
		return Coord{ index % columnSize - padding, index / columnSize - padding };
	}
}