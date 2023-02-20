#pragma once
#include "../GameBoard/GameBoardInterface.h"
#include <unordered_map>

namespace Tests
{
	using TestFn = bool (*)(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoardPtr& gameBoard);

	class Test
	{
	public:
		Test(std::string name, TestFn fn): m_name(name), m_fn(fn){}

		const std::string& GetName() const { return m_name; }

		bool operator()(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoardPtr& gameBoard) const { return m_fn(output, suiteName, testName, gameBoard); }

	private:
		std::string m_name;
		TestFn m_fn;
	};

	class TestEngine
	{
	public:
		TestEngine();

		void RunAllTests(std::ostream& output, GameBoard::IGameBoardPtr& gameBoard) const;

		void RunTestSuite(std::ostream& output, GameBoard::IGameBoardPtr& gameBoard, std::string suiteName) const;

	private:
		std::unordered_map<std::string, std::vector<Test>> m_testSuites;
	};
}