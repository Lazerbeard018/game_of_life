#pragma once
#include <memory>
#include <filesystem>
#include <iostream>
#include <utility>
#include <cassert>

namespace GameBoard
{
	//Our chosen world units. To store 64 bit signed integers we need a uint64
	using Unit = long long;
	using UnsignedUnit = unsigned long long;
}