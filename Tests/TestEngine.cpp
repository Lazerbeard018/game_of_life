#include "TestEngine.h"
#include "../Game/Game.h"
#include "../Input/Input.h"
#include "../Output/Output.h"
#include <fstream>

namespace
{
	void RunTestSuiteInternal(std::ostream& output, GameBoard::IGameBoard& gameBoard, std::string suiteName, const std::vector<Tests::Test>& testSuite)
	{
		output << "Running Test suite " << suiteName << ". " << testSuite.size() << " tests found." << std::endl;

		for (const Tests::Test& test : testSuite)
		{
			output << "    Running Test " << test.GetName() << std::endl;

			output << "        Clearing Gameboard" << std::endl;
			gameBoard.Clear();

			std::string testResult = test(output, suiteName, test.GetName(), gameBoard) ? "Succeeded" : "Failed";

			output << "    Test " << test.GetName() << " " << testResult << std::endl;
		}

		output << "-------------------------------------------------------------------------------------------------" << std::endl;
	}
}

void Tests::TestEngine::RunAllTests(std::ostream& output) const
{
	RunAliveCellListTests(output);
	RunStaticGridBoardTests(output);
}

void Tests::TestEngine::RunAliveCellListTests(std::ostream& output) const
{
	//This board is really just for doing the most basic IO tests
	GameBoard::IGameBoardPtr simpleGameBoard = GameBoard::CreateSimpleAliveCellListBoard();
	RunTestSuite(output, *simpleGameBoard, "Basic_IO");
}

void Tests::TestEngine::RunStaticGridBoardTests(std::ostream& output) const
{ 
	//Use this board to make sure basic life rules work
	GameBoard::IGameBoardPtr staticGridBoard = GameBoard::CreateStaticGridBoard64bit();
	RunTestSuite(output, *staticGridBoard, "8x8_Board");
}

void Tests::TestEngine::RunTestSuite(std::ostream& output, GameBoard::IGameBoard& gameBoard, std::string suiteName) const
{
	auto testSuite = m_testSuites.find(suiteName);

	if (testSuite == m_testSuites.end())
	{
		output << "There is no test suite with name " << suiteName << " skipping." << std::endl;
		return;
	}

	RunTestSuiteInternal(output, gameBoard, suiteName, testSuite->second);
}

namespace
{
	bool LoadTestDataFromName(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoard& gameBoard)
	{
		output << "        Loading test data" << std::endl;

		std::filesystem::path localPathToTestData = "testdata";
		localPathToTestData.append(suiteName);
		localPathToTestData.append(testName);
		localPathToTestData.append("input.life");

		return Input::CreateGameFromFile(localPathToTestData, gameBoard);
	}

	bool DiffFromDisk(std::ostream& output, const std::string& suiteName, const std::string& testName, const GameBoard::IGameBoard& gameBoard)
	{
		output << "        Diffing expected result" << std::endl;
		std::filesystem::path localPathToOutputData = "testdata";
		localPathToOutputData.append(suiteName);
		localPathToOutputData.append(testName);
		localPathToOutputData.append("output.life");

		Output::PrintGameBoardToFile(localPathToOutputData, gameBoard);

		std::filesystem::path localPathToDiffData = "testdata";
		localPathToDiffData.append(suiteName);
		localPathToDiffData.append(testName);
		localPathToDiffData.append("diff.life");

		std::fstream diffStream;
		diffStream.open(localPathToDiffData, std::fstream::in);

		std::fstream outputStream;
		outputStream.open(localPathToOutputData, std::fstream::in);

		std::string diffLine;
		std::string outputLine;

		int lineNumber = 0;

		do
		{
			std::getline(diffStream, diffLine);
			std::getline(outputStream, outputLine);

			if (diffLine != outputLine)
			{
				output << "        Found difference between diff and output. Line " << lineNumber << std::endl;

				output << "        Output from current: " << outputLine << std::endl;
				output << "        Diff from original : " << diffLine << std::endl;

				return false;
			}

			++lineNumber;

		} while (!diffLine.empty() && !outputLine.empty());

		diffStream.close();
		outputStream.close();

		return true;
	}
}

//This is a simple style of test that assumes there is a file at a location specified by the test suite and name
bool LoadAndDiffFromDiskTest(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoard& gameBoard)
{
	if (!LoadTestDataFromName(output, suiteName, testName, gameBoard))
	{
		output << "        Test data at " << suiteName << "\\" << testName << " failed to load." << std::endl;
		return false;
	}

	return DiffFromDisk(output, suiteName, testName, gameBoard);
}

//Actually runs one generation of the game of life
bool LoadAndRun1GenerationAndDiffFromDiskTest(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoard& gameBoard)
{
	if (!LoadTestDataFromName(output, suiteName, testName, gameBoard))
	{
		output << "        Test data at " << suiteName << "\\" << testName << " failed to load." << std::endl;
		return false;
	}

	output << "        Running generation 1" << std::endl;
	Game::RunGameOfLifeGeneration(gameBoard);

	return DiffFromDisk(output, suiteName, testName, gameBoard);
}

Tests::TestEngine::TestEngine()
{
	m_testSuites["Basic_IO"] =
	{
		Test("Identity", *LoadAndDiffFromDiskTest)
	};
	m_testSuites["8x8_Board"] =
	{
		Test("Identity", *LoadAndDiffFromDiskTest),
		Test("4Points", *LoadAndDiffFromDiskTest),
		Test("VerticalLive", *LoadAndRun1GenerationAndDiffFromDiskTest),
		Test("OneGeneration", *LoadAndRun1GenerationAndDiffFromDiskTest)
	};
}
