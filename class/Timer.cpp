#include "Timer.h"

std::deque<double> Timer::ElapsedTimeVec;
double Timer::StoredTotalElapsedTime = 0.0f;

double Timer::ElapsedTime = 0.0;
double Timer::EverageElapsedTime = 0.0;
double Timer::TotalTime = 0.0;

void Timer::SetElapsedTime(double inElapsedTime)
{
	ElapsedTime = inElapsedTime;

	double kicked = 0;
	if (ElapsedTimeVec.size() == ELAPSED_TIME_STORE_COUNT)
	{
		kicked = ElapsedTimeVec.front();
		ElapsedTimeVec.pop_front();
	}

	ElapsedTimeVec.push_back(ElapsedTime);
	StoredTotalElapsedTime = StoredTotalElapsedTime + ElapsedTime - kicked;
	EverageElapsedTime = StoredTotalElapsedTime / (double)ElapsedTimeVec.size();

	TotalTime += ElapsedTime;
}