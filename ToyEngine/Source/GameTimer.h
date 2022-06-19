//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#pragma once

class GameTimer
{
public:
	GameTimer();

	float TotalTime()const;		// total game time
	float DeltaTime()const;		// time between the frame

	void Reset();               // before the message loop
	void Start();               // call it when cancelling the pause
	void Stop();                // pause the game
	void Tick();                // call it every frame

private:
	double m_SecondsPerCount;
	double m_DeltaTime;

	__int64 m_BaseTime;
	__int64 m_PausedTime;
	__int64 m_StopTime;
	__int64 m_PrevTime;
	__int64 m_CurrTime;

	bool m_Stopped;
};
