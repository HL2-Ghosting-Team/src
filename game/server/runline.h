#pragma once
struct RunLine {
		char map[32];
		char name[32];
		float tim;
		float x;
		float y;
		float z;
		float yaw;	// Saves only yaw because ghost isn't player and it will be leans

		RunLine& operator=(const RunLine& other) {
			Q_strcpy(map, other.map);
			Q_strcpy(name, other.name);
			tim = other.tim;
			x = other.x;
			y = other.y;
			z = other.z;
			yaw = other.yaw;
			return *this;
		};
};

struct OnlineRunLine {
	char map[32];
	char name[32];
	float velX;
	float velY;
	float velZ;
	float locX;
	float locY;
	float locZ;

	OnlineRunLine& operator=(const OnlineRunLine& other) {
			Q_strcpy(map, other.map);
			Q_strcpy(name, other.name);
			velX = other.velX;
			velY = other.velY;
			velZ = other.velZ;
			locX = other.locX;
			locY = other.locY;
			locZ = other.locZ;
			return *this;
		};
	
};