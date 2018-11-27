#pragma once

#include "../common/Singleton.h"
#include "../Maths/Vector.h"
#include <memory>
#include <set>
#include <deque>

class Timer
{
protected:
	static std::deque<double> ElapsedTimeVec;
	static double StoredTotalElapsedTime;
	const static uint32_t ELAPSED_TIME_STORE_COUNT = 60;

public:
	static void SetElapsedTime(double inElapsedTime);
	static double GetElapsedTime() { return ElapsedTime; }
	static double GetEverageElapsedTime() { return EverageElapsedTime; }
	static double GetTotalTime() { return TotalTime; }

protected:
	static double ElapsedTime;
	static double EverageElapsedTime;
	static double TotalTime;
};