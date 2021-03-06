#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <omp.h>
#include <ctime>
#include <cstdlib>


#ifdef LIKWID_PERFMON
	#include <likwid.h>
#else
	#define LIKWID_MARKER_INIT
	#define LIKWID_MARKER_THREADINIT
	#define LIKWID_MARKER_SWITCH
	#define LIKWID_MARKER_REGISTER(regionTag)
	#define LIKWID_MARKER_START(regionTag)
	#define LIKWID_MARKER_STOP(regionTag)
	#define LIKWID_MARKER_CLOSE
	#define LIKWID_MARKER_GET(regionTag, nevents, events, time, count)
#endif

namespace G {

using namespace std;

#define LIVE 1
#define DEAD 0

#define LIVE_CHAR 'O'
#define DEAD_CHAR '.'

class Game
{
private:
	long size_;
	long sizeMem_;
	vector<char> grid_;
	vector<char> temp_;

public:

	Game(long size)
	{
LIKWID_MARKER_START("SEQUENTIAL");

		size_ = size;
		sizeMem_ = size_ + 2;
		
		grid_.reserve(sizeMem_*sizeMem_);
		temp_.reserve(sizeMem_*sizeMem_);
		
		for(long i = 0; i < sizeMem_; ++i){
			for(long j = 0; j < sizeMem_; ++j)
			{
				grid_.push_back(DEAD);
				temp_.push_back(DEAD);
			}
		}
		
LIKWID_MARKER_STOP("SEQUENTIAL");
	}

	inline long size() const {
		return size_;
	}

	inline char& cell(long i, long j){
		return grid_[(i+1)*sizeMem_ + (j+1)];
	}
	inline const char& cell(long i, long j) const {
		return grid_[(i+1)*sizeMem_ + (j+1)];
	}

	inline char& cellTmp(long i, long j){
		return temp_[(i+1)*sizeMem_ + (j+1)];
	}

	void print() const
	{
		for(long i = 0; i < size_; ++i){
			for(long j = 0; j < size_; ++j)
			{
				cout << cell(i,j) << " ";
			}
			cout << "\n";
		}
		cout << flush;
	}


	inline void nextGen()
	{

#pragma omp parallel
{
LIKWID_MARKER_START("Parallel");
		
		#pragma omp for schedule(static)
		for(long i = 0; i < size_; ++i)
		{
			for(long j = 0; j < size_; ++j)
			{
				int neib = neighbours(i,j);
				
				if(cell(i,j) == LIVE)
				{
					if(neib < 2 || neib > 3)
					{
						cellTmp(i,j) = DEAD;
					} else {
						cellTmp(i,j) = LIVE;
					}
				}
				else // if cell(i,j) == DEAD
				{
					if(neib == 3) {
						cellTmp(i,j) = LIVE;
					} else {
						cellTmp(i,j) = DEAD;
					}
				}
			}
		}
		
LIKWID_MARKER_STOP("Parallel");
}
		
LIKWID_MARKER_START("SEQUENTIAL");
		
		grid_.swap(temp_);
		
LIKWID_MARKER_STOP("SEQUENTIAL");
	}


	// With Blocking
	inline void nextGenB()
	{
		long blockSZ = 24;
		
#pragma omp parallel
{
LIKWID_MARKER_START("Parallel");
		
		#pragma omp for schedule(static)
		for( long ib = 0; ib < size_; ib += blockSZ ){
		long max_i = min(ib + blockSZ, size_);
		for( long jb = 0; jb < size_; jb += blockSZ ){
		long max_j = min(jb + blockSZ, size_);
		
			for( long i = ib; i < max_i; ++i )
			for( long j = jb; j < max_j; ++j )
			{
				int neib = neighbours(i,j);
				
				if(cell(i,j) == LIVE)
				{
					if(neib < 2 || neib > 3)
					{
						cellTmp(i,j) = DEAD;
					} else {
						cellTmp(i,j) = LIVE;
					}
				}
				else // if cell(i,j) == DEAD
				{
					if(neib == 3) {
						cellTmp(i,j) = LIVE;
					} else {
						cellTmp(i,j) = DEAD;
					}
				}
			}
			
		}
		}
LIKWID_MARKER_STOP("Parallel");
}
		
LIKWID_MARKER_START("SEQUENTIAL");
		
		grid_.swap(temp_);
		
LIKWID_MARKER_STOP("SEQUENTIAL");
	}

	inline int neighbours(long x, long y) const
	{
		int count = 0;
		
		count += cell(x-1,y-1);
		count += cell(x-1,y);
		count += cell(x-1,y+1);
		count += cell(x,y-1);
		count += cell(x,y+1);
		count += cell(x+1,y-1);
		count += cell(x+1,y);
		count += cell(x+1,y+1);
		
		return(count);
	}
	
	
	void init(double chanceAlive){
	
#pragma omp parallel
{
LIKWID_MARKER_START("Parallel");
		
		#pragma omp for schedule(static)
		for(long i = 0; i < size_; ++i)
		{
			for(long j = 0; j < size_; ++j)
			{
				double val = (double)rand() / RAND_MAX;
				if (val < chanceAlive)
					cell(i,j) = LIVE;
				else
					cell(i,j) = DEAD;
			}
		}

LIKWID_MARKER_STOP("Parallel");
}
	}
	
	void init(string s)
	{
LIKWID_MARKER_START("SEQUENTIAL");
		
		if(s == "Glider"){
			cell(0,1) = LIVE;
			cell(1,2) = LIVE;
			cell(2,0) = LIVE;
			cell(2,1) = LIVE;
			cell(2,2) = LIVE;
		}

		if(s == "Pulsar"){
			cell(2,4) = LIVE;
			cell(2,5) = LIVE;
			cell(2,6) = LIVE;
			cell(2,10) = LIVE;
			cell(2,11) = LIVE;
			cell(2,12) = LIVE;
			cell(4,2) = LIVE;
			cell(4,7) = LIVE;
			cell(4,9) = LIVE;
			cell(4,14) = LIVE;
			cell(5,2) = LIVE;
			cell(5,7) = LIVE;
			cell(5,9) = LIVE;
			cell(5,14) = LIVE;
			cell(6,2) = LIVE;
			cell(6,7) = LIVE;
			cell(6,9) = LIVE;
			cell(6,14) = LIVE;
			cell(7,4) = LIVE;
			cell(7,5) = LIVE;
			cell(7,6) = LIVE;
			cell(7,10) = LIVE;
			cell(7,11) = LIVE;
			cell(7,12) = LIVE;
			cell(9,4) = LIVE;
			cell(9,5) = LIVE;
			cell(9,6) = LIVE;
			cell(9,10) = LIVE;
			cell(9,11) = LIVE;
			cell(9,12) = LIVE;
			cell(10,2) = LIVE;
			cell(10,7) = LIVE;
			cell(10,9) = LIVE;
			cell(10,14) = LIVE;
			cell(11,2) = LIVE;
			cell(11,7) = LIVE;
			cell(11,9) = LIVE;
			cell(11,14) = LIVE;
			cell(12,2) = LIVE;
			cell(12,7) = LIVE;
			cell(12,9) = LIVE;
			cell(12,14) = LIVE;
			cell(14,4) = LIVE;
			cell(14,5) = LIVE;
			cell(14,6) = LIVE;
			cell(14,10) = LIVE;
			cell(14,11) = LIVE;
			cell(14,12) = LIVE;
		}

		if(s == "Glider gun"){
			cell(5,1) = LIVE;
			cell(6,1) = LIVE;
			cell(5,2) = LIVE;
			cell(6,2) = LIVE;

			cell(5,11) = LIVE;
			cell(6,11) = LIVE;
			cell(7,11) = LIVE;
			cell(4,12) = LIVE;
			cell(8,12) = LIVE;
			cell(3,13) = LIVE;
			cell(9,13) = LIVE;
			cell(3,14) = LIVE;
			cell(9,14) = LIVE;
			cell(6,15) = LIVE;
			cell(4,16) = LIVE;
			cell(8,16) = LIVE;
			cell(5,17) = LIVE;
			cell(6,17) = LIVE;
			cell(7,17) = LIVE;
			cell(6,18) = LIVE;

			cell(3,21) = LIVE;
			cell(4,21) = LIVE;
			cell(5,21) = LIVE;
			cell(3,22) = LIVE;
			cell(4,22) = LIVE;
			cell(5,22) = LIVE;
			cell(2,23) = LIVE;
			cell(6,23) = LIVE;
			cell(1,25) = LIVE;
			cell(2,25) = LIVE;
			cell(6,25) = LIVE;
			cell(7,25) = LIVE;

			cell(3,35) = LIVE;
			cell(4,35) = LIVE;
			cell(3,36) = LIVE;
			cell(4,36) = LIVE;
		}
		
LIKWID_MARKER_STOP("SEQUENTIAL");
	}
};

}
#endif // GAME_H
