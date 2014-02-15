#pragma once
struct RunLine {
		char map[32];
		char name[32];
		float tim;
		float x;
		float y;
		float z;

		RunLine& operator=(const RunLine& other) {
			Q_strcpy(map, other.map);
			Q_strcpy(name, other.name);
			tim = other.tim;
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		};
};