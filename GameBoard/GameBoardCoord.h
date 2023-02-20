#pragma once
#include "GameBoardDefines.h"

namespace GameBoard
{
	/// <summary>
	/// A simple POD struct to hold the coordinates for incoming and outgoing gameboard locations.
	/// </summary>
	struct Coord
	{
		Unit x;
		Unit y;
	};
}
