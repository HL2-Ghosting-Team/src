#include "cbase.h"
#pragma once


class TickCounter {
public:

	char name[64];
	int startTick;
	int currentTick;
	int totalTicks;

	TickCounter(const char* program, int startTic) {
		Q_strcpy(name, program);
		startTick = startTic;
	}

	void init(int startTic) {
		startTick = startTic;
	}

	void increment(int currentTick) {
		totalTicks += currentTick - startTick;
		startTick = currentTick;
	}

	float getTicks() {
		return (float)totalTicks;
	}

	const char* getName() {
		return name;
	}


};