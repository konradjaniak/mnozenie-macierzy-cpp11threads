/**
* Compile:  g++ zadanie.cpp -o zadanie -O3 -lpthread
* Use:      ./zadanie [rozmiar_macierzy: 1..X] [transpozycja: -T0 (nie), -T1 (tak)] [ilosc watkow: -P1..6]
* Example:  PRZYKLAD: ./zadanie 1000 -T1 -P2
**/

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <vector>

const int MIN_THREADS = 1;
const int MAX_THREADS = 6;

bool isTransposed(const char* arg);
int getMatrixSize(const char* arg);
int getNumOfThreads(const char* arg);
int** allocateMemoryFor2DArray(const int tabSize);
void freeMemoryOf2DArray(int** tab, const int tabSize);
void fill2DArrayWithRandomValues(int** tabToFill, const int tabSize);
void multiplyTwo2DArrays(int** tabA, int** tabB, int** tabResult, const int tabSize, const int partOfMatrix, const int numOfElements, const int remainder, bool isTransposed);

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::cerr << "\n\tERROR: Zbyt mala ilosc argumentow\n";
        std::cerr << "\tPOPRAWNIE: ./zadanie [rozmiar_macierzy: 1..X] [transpozycja: -T0 (nie), -T1 (tak)] [ilosc watkow: -P1..6]\n";
        std::cerr << "\tPRZYKLAD: ./zadanie 1000 -T1 -P2\n\n";
        return 1;
    }

    srand(time(0));

    int matrixSize = getMatrixSize(argv[1]);
    bool transposed = isTransposed(argv[2]);
    int numOfThreads = getNumOfThreads(argv[3]);

    int** A = allocateMemoryFor2DArray(matrixSize);
    int** B = allocateMemoryFor2DArray(matrixSize);
    int** C = allocateMemoryFor2DArray(matrixSize);

    fill2DArrayWithRandomValues(A, matrixSize);
    fill2DArrayWithRandomValues(B, matrixSize);

    int columnsPerThread = (numOfThreads > 1) ? (matrixSize/numOfThreads) : matrixSize;
    int remainingColumns = matrixSize % numOfThreads;
    int packetCounter = 0;

    std::vector<std::thread> threads;

    auto timeStart = std::chrono::steady_clock::now();
    
    if (numOfThreads > 1)
    {
        for (int i = 0; i < numOfThreads - 1; i++)
        {
            threads.push_back(std::thread(multiplyTwo2DArrays, A, B, C, matrixSize, packetCounter, columnsPerThread, 0, transposed));
            packetCounter++;
        }
    }

    multiplyTwo2DArrays(A, B, C, matrixSize, packetCounter, columnsPerThread, remainingColumns, transposed);
    
    if (numOfThreads > 1)
    {
        for (int i = 0; i < numOfThreads - 1; i++)
        {
            threads[i].join();
        }
    }

    auto timeEnd = std::chrono::steady_clock::now();
    std::chrono::duration<double> timeElapsed = timeEnd - timeStart;

    freeMemoryOf2DArray(A, matrixSize);
    freeMemoryOf2DArray(B, matrixSize);
    freeMemoryOf2DArray(C, matrixSize);

    std::cout << "===========================================================\n";
    std::cout << "Rozmiar macierzy: " << matrixSize << "x" << matrixSize << "\n";
    std::cout << "Transponowana:    " << (transposed ? "TAK" : "NIE") << "\n";
    std::cout << "Ilosc watkow:     " << numOfThreads << "\n";
    std::cout << "Czas wykonywania: " << timeElapsed.count() << " s\n";
    std::cout << "===========================================================\n\n";

    return 0;
}


bool isTransposed(const char* arg)
{
    std::string str = arg;
    if (str.compare("-T1") == 0) return true;
    else return false;
}

int getMatrixSize(const char* arg)
{
    int size = std::atoi(arg);
    return size > 0 ? size : 1;
}

int getNumOfThreads(const char* arg)
{
    std::string str = arg;

    if (str.find("-P") != std::string::npos)
    {
        std::string subs(arg, 2, str.size() - 2);
        int t = std::atoi(subs.c_str());
        if (t > 1 && t <= MAX_THREADS) return t;
        else if (t > MAX_THREADS) return MAX_THREADS;
    }

    return 1;
}

int** allocateMemoryFor2DArray(const int tabSize)
{
    int** tab = new int*[tabSize];

    for (int i = 0; i < tabSize; i++)
    {
        tab[i] = new int[tabSize];
    }

    return tab;
}

void freeMemoryOf2DArray(int** tab, const int tabSize)
{
    for (int i = 0; i < tabSize; i++)
    {
        delete [] tab[i];
    }
}

void fill2DArrayWithRandomValues(int** tabToFill, int tabSize)
{
    for (int i = 0; i < tabSize; i++)
    {
        for (int j = 0; j < tabSize; j++)
        {
            tabToFill[i][j] = rand() % 10;
        }
    }
}

void multiplyTwo2DArrays(int** tabA, int** tabB, int** tabResult, int tabSize, int partOfMatrix, int numOfElements, int remainder, bool isTransposed)
{
    int indexFirstElementOfPacket = partOfMatrix * numOfElements;
    int indexLastElementOfPacket = indexFirstElementOfPacket + numOfElements - 1 + remainder;

    for (int i = indexFirstElementOfPacket; i < indexLastElementOfPacket; i++)
    {
        for (int j = 0; j < tabSize; j++)
        {
            for (int k = 0; k < tabSize; k++)
            {
                if (!isTransposed)
                    tabResult[i][j] = tabA[i][k] * tabB[k][j];
                else
                    tabResult[i][j] = tabA[i][k] * tabB[j][k];
            }
        }
    }
}