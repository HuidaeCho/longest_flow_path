CXXFLAGS=-std=c++11 -O3 -fopenmp
LDFLAGS=-std=c++11 -O3 -fopenmp

all: blfp

clean:
	$(RM) *.o

blfp: CellLocation.o DoubleDropOmpLfp.o DoubleDropSeqLfp.o FlowDirectionLoader.o FlowDirectionMatrix.o FlowPathLength.o ILongestFlowPathAlgorithm.o ILongestFlowPathAllAlgorithm.o ILongestFlowPathMultipleAlgorithm.o InletNumberCalculator.o InletNumberMatrix.o RecursiveSeqLfp.o RecursiveTaskLfp.o TopDownMaxSeqLfp.o TopDownSingleOmpLfp.o TopDownSingleSeqLfp.o main.o
	$(CXX) $(LDFLAGS) -o $@ $^ `gdal-config --libs`
