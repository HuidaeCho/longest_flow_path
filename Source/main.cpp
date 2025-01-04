#include "FlowDirectionLoader.h"
#include "RecursiveSeqLfp.h"
#include "RecursiveTaskLfp.h"
#include "TopDownMaxSeqLfp.h"
#include "TopDownSingleSeqLfp.h"
#include "TopDownSingleOmpLfp.h"
#include "DoubleDropSeqLfp.h"
#include "DoubleDropOmpLfp.h"
#include <chrono>
#include <iostream>
#include <fstream>


const int cellPrintLimit = 8;


std::string algorithmLabel(int algorithmIndex)
{
  switch (algorithmIndex)
  {
    case 1: return "recursive (sequential)";
    case 2: return "recursive (task-based parallel)";
    case 3: return "top-down: maximum length (sequential)";
    case 4: return "top-down: single update (sequential)";
    case 5: return "top-down: single update (parallel)";
    case 6: return "double drop (sequential)";
    case 7: return "double drop (parallel)";
    default: return "";
  }
}


ILongestFlowPathMultipleAlgorithm* createMultipleAlgorithm(int algorithmIndex, int algorithmParameter)
{
  switch (algorithmIndex)
  {
    case 3: return new TopDownMaxSeqLfp();
    case 4: return new TopDownSingleSeqLfp();
    case 5: return new TopDownSingleOmpLfp();
    default: return nullptr;
  }
}


ILongestFlowPathAlgorithm* createAlgorithm(int algorithmIndex, int algorithmParameter)
{
  switch (algorithmIndex)
  {
    case 1: return new RecursiveSeqLfp();
    case 2: return new RecursiveTaskLfp(algorithmParameter);
    case 3: return new TopDownMaxSeqLfp();
    case 4: return new TopDownSingleSeqLfp();
    case 5: return new TopDownSingleOmpLfp();
    case 6: return new DoubleDropSeqLfp();
    case 7: return new DoubleDropOmpLfp();
    default: return nullptr;
  }
}


void printUsage()
{
  std::cout << "required arguments:" << std::endl
            << " 1.  flow direction filename" << std::endl
            << " 2.  outlet location filename (containing row and column coordinates, one-based indexing)" << std::endl
            << " 3.  algorithm index" << std::endl
            << " 4.  output filename" << std::endl
            << "(5.) algorithm parameter (task-based recursive: task creation limit, top-down: 1 for all outlets (default: only first outlet))" << std::endl
            << std::endl
            << "available algorithms:" << std::endl;

  for (int i = 1; i <= 7; ++i)
  {
    std::cout << ' ' << i << ".  " << algorithmLabel(i) << std::endl;
  }
}


std::vector<CellLocation> loadOutletLocations(std::string filename)
{
  std::vector<CellLocation> outlets;

  std::fstream file(filename, std::ios::in);
  int row, col, label;

  while (file >> row >> col >> label)
  {
    outlets.push_back({row, col});
  }
  file.close();

  return outlets;
}


void printCells(std::string label, std::vector<CellLocation>& cells)
{
  const int cellsTotal = cells.size();
  const int cellsToPrint = std::min(cellsTotal, cellPrintLimit);

  std::cout << "number of " << label << " locations: " << cellsTotal << std::endl;

  for (int i = 0; i < cellsToPrint; ++i)
  {
    std::cout << "- row " << cells[i].row << ", column " << cells[i].col << std::endl;
  }

  if (cellsToPrint < cellsTotal)
  {
    std::cout << "- ..." << std::endl;
  }
}


void executeMeasurement(std::string directionFilename, std::string outletFilename, int algorithmIndex, std::string outputFilename, int algorithmParameter)
{
  std::cout << "loading flow direction file (" << directionFilename << ")..." << std::endl;
  FlowDirectionMatrix directionMatrix = FlowDirectionLoader::loadGdal(directionFilename);
  std::cout << "flow direction data: " << directionMatrix.height << " rows, " << directionMatrix.width << " columns" << std::endl;

  std::cout << "loading outlet file (" << outletFilename << ")..." << std::endl;
  std::vector<CellLocation> outletLocations = loadOutletLocations(outletFilename);

  std::cout << "executing " << algorithmLabel(algorithmIndex) << " algorithm..." << std::endl;

  if (algorithmParameter && algorithmIndex >= 3 && algorithmIndex <= 5) {
    printCells("outlet", outletLocations);

    ILongestFlowPathMultipleAlgorithm* algorithm = createMultipleAlgorithm(algorithmIndex, algorithmParameter);

    auto stamp_begin = std::chrono::high_resolution_clock::now();
    std::vector<CellLocation> sourceLocations = algorithm->executeMultiple(directionMatrix, outletLocations);
    auto stamp_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> stamp_diff = stamp_end - stamp_begin;
    printCells("source", sourceLocations);
    std::cout << "execution time (ms): " << lround(stamp_diff.count()) << std::endl;

    std::ofstream outfile;
    outfile.open(outputFilename);
    outfile << "row,column" << std::endl;
    for (int i = 0; i < sourceLocations.size(); ++i) {
	    outfile << sourceLocations[i].row << "," << sourceLocations[i].col << std::endl;
    }
    outfile.close();
  } else {
    std::cout << "outlet location: row " << outletLocations[0].row << ", column " << outletLocations[0].col << std::endl;

    ILongestFlowPathAlgorithm* algorithm = createAlgorithm(algorithmIndex, algorithmParameter);

    auto stamp_begin = std::chrono::high_resolution_clock::now();
    CellLocation sourceLocation = algorithm->execute(directionMatrix, outletLocations[0]);
    auto stamp_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> stamp_diff = stamp_end - stamp_begin;

    std::cout << "source location: row " << sourceLocation.row << ", column " << sourceLocation.col << std::endl;
    std::cout << "execution time (ms): " << lround(stamp_diff.count()) << std::endl;

    std::ofstream outfile;
    outfile.open(outputFilename);
    outfile << "row,column" << std::endl;
    outfile << sourceLocation.row << "," << sourceLocation.col << std::endl;
    outfile.close();
  }
}


int main(int argc, char** argv)
{
  if (argc < 4)
  {
    printUsage();
  }

  else
  {
    const std::string directionFilename = argv[1];
    const std::string outletFilename = argv[2];
    const int algorithmIndex = atoi(argv[3]);
    const std::string outputFilename = argv[4];
    const int algorithmParameter = (argc > 5) ? atoi(argv[5]) : 0;

    executeMeasurement(directionFilename, outletFilename, algorithmIndex, outputFilename, algorithmParameter);
  }
}
