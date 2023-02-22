#include "TestEngine.h"
#include "../Game/Game.h"
#include "../Input/Input.h"
#include "../Output/Output.h"
#include <fstream>
#include <chrono>
#include <optional>

namespace
{
	void RunTestSuiteInternal(std::ostream& output, GameBoard::IGameBoard& gameBoard, std::string suiteName, const std::vector<Tests::Test>& testSuite, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max)
	{
#if defined(_DEBUG)
		int memoryDbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		memoryDbgFlags |= _CRTDBG_ALLOC_MEM_DF;
		memoryDbgFlags |= _CRTDBG_CHECK_ALWAYS_DF;
		memoryDbgFlags |= _CRTDBG_LEAK_CHECK_DF;

		// Set flag to the new value.
		_CrtSetDbgFlag(memoryDbgFlags);
#endif //_DEBUG

		output << "Running Test suite " << suiteName << ". " << testSuite.size() << " tests found." << std::endl;

		for (const Tests::Test& test : testSuite)
		{
			output << "    Running Test " << test.GetName() << std::endl;

			output << "        Clearing Gameboard" << std::endl;
			gameBoard.Clear();

#if defined(_DEBUG)
			_CrtMemState memoryBeforeTest;
			_CrtMemCheckpoint(&memoryBeforeTest);
#endif //_DEBUG

			const auto timeBeforeTest = std::chrono::high_resolution_clock::now();

			std::string testResult = test(output, suiteName, test.GetName(), gameBoard, min, max) ? "Succeeded" : "Failed";

			output << "    Test " << test.GetName() << " " << testResult << std::endl;

			const auto timeAfterTest = std::chrono::high_resolution_clock::now();

			std::chrono::duration<float, std::chrono::milliseconds::period> elapsedTime = timeAfterTest - timeBeforeTest;
			output << "    Elapsed time: " << elapsedTime << std::endl;

#if defined(_DEBUG)
			_CrtMemState memoryAfterTest;
			_CrtMemCheckpoint(&memoryAfterTest);

			_CrtMemState difference;
			_CrtMemDifference(&difference, &memoryBeforeTest, &memoryAfterTest);
			output << "    Total memory allocated: " << difference.lTotalCount << std::endl;
			output << "    Maximum memory used at a time (since tests started): " << memoryAfterTest.lHighWaterCount << std::endl;
			output << "--------------------------------------------" << std::endl;
#endif //_DEBUG
		}

		output << "-------------------------------------------------------------------------------------------------" << std::endl;
	}
}

void Tests::TestEngine::RunAllTests(std::ostream& output) const
{
	RunAliveCellListTests(output);
	RunStaticGridBoardTests(output);
	RunMultiGridBoardTests(output);
	RunStressBoardTests(output);
}

void Tests::TestEngine::RunAliveCellListTests(std::ostream& output) const
{
	//This board is really just for doing the most basic IO tests
	GameBoard::IGameBoardPtr simpleGameBoard = GameBoard::CreateSimpleAliveCellListBoard();
	RunTestSuite(output, *simpleGameBoard, "Basic_IO", std::nullopt, std::nullopt);
}

void Tests::TestEngine::RunStaticGridBoardTests(std::ostream& output) const
{ 
	//Use this board to make sure basic life rules work
	GameBoard::IGameBoardPtr staticGridBoard = GameBoard::CreateStaticGridBoard6();
	RunTestSuite(output, *staticGridBoard, "8x8_Board", GameBoard::Coord{ 0,0 }, GameBoard::Coord{ 8,8 });
}

void Tests::TestEngine::RunMultiGridBoardTests(std::ostream& output) const
{
	//Make a multi grid board but use a very small static grid so the numbers are small when we have to deal with traversing boards
	GameBoard::IGameBoardPtr multiGridBoard = GameBoard::CreateMultiGridBoard(&GameBoard::CreateStaticGridBoard6);
	RunTestSuite(output, *multiGridBoard, "8x8_Board", GameBoard::Coord{ 0,0 }, GameBoard::Coord{ 8,8 });
	RunTestSuite(output, *multiGridBoard, "Big_Board", std::nullopt, std::nullopt);
}

void Tests::TestEngine::RunStressBoardTests(std::ostream& output) const
{
	//Make a multi grid board but use a very small static grid so the numbers are small when we have to deal with traversing boards
	GameBoard::IGameBoardPtr multiGridBoard = GameBoard::CreateMultiGridBoard(&GameBoard::CreateStaticGridBoard);
	RunTestSuite(output, *multiGridBoard, "Stress_Test", GameBoard::Coord{ 0,0 }, GameBoard::Coord{ 50,50 });
}

void Tests::TestEngine::RunTestSuite(std::ostream& output, GameBoard::IGameBoard& gameBoard, std::string suiteName, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max) const
{
	auto testSuite = m_testSuites.find(suiteName);

	if (testSuite == m_testSuites.end())
	{
		output << "There is no test suite with name " << suiteName << " skipping." << std::endl;
		return;
	}

	RunTestSuiteInternal(output, gameBoard, suiteName, testSuite->second, min, max);
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

	bool DiffFromDisk(std::ostream& output, const std::string& suiteName, const std::string& testName, const GameBoard::IGameBoard& gameBoard, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max)
	{
		output << "        Diffing expected result" << std::endl;
		std::filesystem::path localPathToOutputData = "testdata";
		localPathToOutputData.append(suiteName);
		localPathToOutputData.append(testName);
		localPathToOutputData.append("output.life");

		if (min.has_value() && max.has_value())
		{
			Output::PrintGameRectToFile(localPathToOutputData, *min, *max, gameBoard);
		}
		else
		{
			Output::PrintGameBoardToFile(localPathToOutputData, gameBoard);
		}

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
bool LoadAndDiffFromDiskTest(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoard& gameBoard, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max)
{
	if (!LoadTestDataFromName(output, suiteName, testName, gameBoard))
	{
		output << "        Test data at " << suiteName << "\\" << testName << " failed to load." << std::endl;
		return false;
	}

	return DiffFromDisk(output, suiteName, testName, gameBoard, min, max);
}

//Actually runs one generation of the game of life
bool LoadAndRun1GenerationAndDiffFromDiskTest(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoard& gameBoard, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max)
{
	if (!LoadTestDataFromName(output, suiteName, testName, gameBoard))
	{
		output << "        Test data at " << suiteName << "\\" << testName << " failed to load." << std::endl;
		return false;
	}

	output << "        Running generation 1" << std::endl;
	Game::RunGameOfLifeGeneration(gameBoard);

	return DiffFromDisk(output, suiteName, testName, gameBoard, min, max);
}

bool LoadAndRun15GenerationAndDiffFromDiskTest(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoard& gameBoard, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max)
{
	if (!LoadTestDataFromName(output, suiteName, testName, gameBoard))
	{
		output << "        Test data at " << suiteName << "\\" << testName << " failed to load." << std::endl;
		return false;
	}

	for (int i = 0; i < 15; ++i)
	{
		output << "        Running generation " << i << std::endl;
		Game::RunGameOfLifeGeneration(gameBoard);
	}

	return DiffFromDisk(output, suiteName, testName, gameBoard, min, max);
}

bool LoadAndRun100GenerationAndDiffFromDiskTest(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoard& gameBoard, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max)
{
	if (!LoadTestDataFromName(output, suiteName, testName, gameBoard))
	{
		output << "        Test data at " << suiteName << "\\" << testName << " failed to load." << std::endl;
		return false;
	}

	for (int i = 0; i < 100; ++i)
	{
		Game::RunGameOfLifeGeneration(gameBoard);
	}

	return DiffFromDisk(output, suiteName, testName, gameBoard, min, max);
}

bool MakeTheLineTest(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoard& gameBoard, std::optional<GameBoard::Coord> min, std::optional<GameBoard::Coord> max)
{
	for (long long i = 0; i < 1000000; ++i)
	{
		gameBoard.SetCell({i,i}, true);
	}
	gameBoard.FinishCurrentGeneration();

	for (int i = 0; i < 100; ++i)
	{
		Game::RunGameOfLifeGeneration(gameBoard);
	}

	return DiffFromDisk(output, suiteName, testName, gameBoard, min, max);
}

Tests::TestEngine::TestEngine()
{
	m_testSuites["Basic_IO"] =
	{
		Test("Identity", *LoadAndDiffFromDiskTest),
	};
	m_testSuites["8x8_Board"] =
	{
		Test("Identity", *LoadAndDiffFromDiskTest),
		Test("4Points", *LoadAndDiffFromDiskTest),
		Test("VerticalLive", *LoadAndRun1GenerationAndDiffFromDiskTest),
		Test("OneGeneration", *LoadAndRun1GenerationAndDiffFromDiskTest)
	};
	m_testSuites["Big_Board"] =
	{
		Test("Negative", *LoadAndDiffFromDiskTest),
		Test("BorderTraversal", *LoadAndRun1GenerationAndDiffFromDiskTest),
		Test("VerticalLive", *LoadAndRun1GenerationAndDiffFromDiskTest),
		Test("OneGeneration", *LoadAndRun1GenerationAndDiffFromDiskTest),
		Test("TenGeneration", *LoadAndRun15GenerationAndDiffFromDiskTest),
		Test("OneHundredGeneration", *LoadAndRun100GenerationAndDiffFromDiskTest),
	};
	m_testSuites["Stress_Test"] =
	{
		Test("TheLine", *MakeTheLineTest),
	};
}
