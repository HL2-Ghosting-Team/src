#include "cbase.h"
#include "TickCounter.h"
#pragma once

class TickCounterEngine {

public:

	CUtlVector<TickCounter*> counters;


	void createCounter(const char* name) {
		TickCounter* newT = new TickCounter(name, gpGlobals->tickcount);
		counters.AddToTail(newT);
	}

	TickCounter* getCounter(const char* name) {
		size_t size = counters.Count();
		for (unsigned int i = 0; i < size; i++) {
			if (Q_strcmp(counters[i]->getName(), name) == 0) {
				return counters[i];
			}
		}
		return NULL;
	}


};