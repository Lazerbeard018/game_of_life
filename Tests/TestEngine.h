#pragma once
#include "../GameBoard/GameBoardInterface.h"
#include <unordered_map>
#include <optional>

namespace Tests
{
	using TestFn = bool (*)(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoard& gameBoard, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max);

	class Test
	{
	public:
		Test(std::string name, TestFn fn): m_name(name), m_fn(fn){}

		const std::string& GetName() const { return m_name; }

		bool operator()(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoard& gameBoard, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max) const { return m_fn(output, suiteName, testName, gameBoard, min, max); }

	private:
		std::string m_name;
		TestFn m_fn;
	};

	class TestEngine
	{
	public:
		TestEngine();

		void RunAllTests(std::ostream& output) const;

		void RunAliveCellListTests(std::ostream& output) const;

		void RunStaticGridBoardTests(std::ostream& output) const;

		void RunMultiGridBoardTests(std::ostream& output) const;

		void RunStressBoardTests(std::ostream& output) const;

		void RunTestSuite(std::ostream& output, GameBoard::IGameBoard& gameBoard, std::string suiteName, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max) const;

	private:
		std::unordered_map<std::string, std::vector<Test>> m_testSuites;
	};
}