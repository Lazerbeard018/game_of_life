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

	bool Get1DIndexFromCoord(const Coord& coord, Unit columnSize, Unit padding, Unit& output);
	Coord GetCoordFrom1DIndex(Unit index, Unit columnSize, Unit padding);

	/// <summary>
	/// Less than operator to help comparison and iterators. 
	/// A coord is less than if is on a row lower than the comparer. If they are on the same row, 
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

	class HashCoord
	{
	public:
		size_t operator()(const Coord& coord) const
		{
			std::hash<Unit> unitHash;
			size_t x = unitHash(coord.x);
			size_t y = unitHash(coord.y);
			return x ^ y + 0x9e3779b9 + (x << 6) + (x >> 2);
		}
	};

	class EqualCoord
	{
	public:
		size_t operator()(const Coord& lhs, const Coord& rhs) const
		{
			return lhs.x == rhs.x && rhs.y == lhs.y;
		}
	};
}
