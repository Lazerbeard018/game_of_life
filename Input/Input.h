#pragma once
#include "..\GameBoard\GameBoardInterface.h"

/// <summary>
/// I decided to allow any gameboard I come up with to be filled out by two methods.
/// standard output is what the original spec asked for, but I'd like to get input from a file as well.
/// Mainly to support automated testing of the feature that I can get from making my own datasets or
/// well-known patterns I can find online.
/// </summary>
namespace Input
{
	/// <summary>
	/// Fills out game of life gameboard with integer numbers entered from any standard input stream.
	/// </summary>
	/// <param name="stream">Standard input stream to read live cell positions.</param>
	/// <param name="gameBoard">The gameboard we intend to fill out.</param>
	void CreateGameFromStream(std::istream& stream, GameBoard::IGameBoardPtr& gameBoard);

	/// <summary>
	/// Fills out game of life gameboard with integer numbers entered from standard output.
	/// </summary>
	/// <param name="gameBoard">The gameboard we intend to fill out.</param>
	void CreateGameFromStdInput(GameBoard::IGameBoardPtr& gameBoard);

	/// <summary>
	/// Fills out game of life gameboard with integer numbers found in a file.
	/// </summary>
	/// <param name="filename">A path to the file we intend to load, either relative to the working directory or a full path.</param>
	/// <param name="gameBoard">The gameboard we intend to fill out.</param>
	bool CreateGameFromFile(std::filesystem::path filename, GameBoard::IGameBoardPtr& gameBoard);
}