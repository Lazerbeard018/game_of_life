#include "TestEngine.h"
#include "../Input/Input.h"
#include "../Output/Output.h"
#include <fstream>

namespace
{
	void RunTestSuiteInternal(std::ostream& output, GameBoard::IGameBoardPtr& gameBoard, std::string suiteName, const std::vector<Tests::Test>& testSuite)
	{
		output << "Running Test suite " << suiteName << ". " << testSuite.size() << " tests found." << std::endl;

		for (const Tests::Test& test : testSuite)
		{
			output << "    Running Test " << test.GetName() << std::endl;

			output << "        Clearing Gameboard" << std::endl;
			gameBoard->Clear();

			std::string testResult = test(output, suiteName, test.GetName(), gameBoard) ? "Succeeded" : "Failed";

			output << "    Test " << test.GetName() << " " << testResult << std::endl;
		}

		output << "-------------------------------------------------------------------------------------------------" << std::endl;
	}
}

void Tests::TestEngine::RunAllTests(std::ostream& output, GameBoard::IGameBoardPtr& gameBoard) const
{
	for (auto& testSuite : m_testSuites)
	{
		RunTestSuiteInternal(output, gameBoard, testSuite.first, testSuite.second);
	}
}

void Tests::TestEngine::RunTestSuite(std::ostream& output, GameBoard::IGameBoardPtr& gameBoard, std::string suiteName) const
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
	bool LoadTestDataFromName(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoardPtr& gameBoard)
	{
		output << "        Loading test data" << std::endl;

		std::filesystem::path localPathToTestData = "testdata";
		localPathToTestData.append(suiteName);
		localPathToTestData.append(testName);
		localPathToTestData.append("input.life");

		return Input::CreateGameFromFile(localPathToTestData, gameBoard);
	}

	bool DiffFromDisk(std::ostream& output, const std::string& suiteName, const std::string& testName, const GameBoard::IGameBoard* gameBoard)
	{
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
bool LoadAndDiffFromDiskTest(std::ostream& output, const std::string& suiteName, const std::string& testName, GameBoard::IGameBoardPtr& gameBoard)
{
	if (!LoadTestDataFromName(output, suiteName, testName, gameBoard))
	{
		output << "        Test data at " << suiteName << "\\" << testName << " failed to load." << std::endl;
		return false;
	}

	return DiffFromDisk(output, suiteName, testName, gameBoard.get());
}

Tests::TestEngine::TestEngine()
{
	m_testSuites["Basic_IO"] =
	{
		Test("Identity", *LoadAndDiffFromDiskTest)
	};
}
