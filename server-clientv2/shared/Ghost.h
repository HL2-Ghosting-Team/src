#include <string>
class Ghost
{
public:
	std::string ip;
	std::string name;
	std::string model;
	std::string map;
	float x;
	float y;
	float z;
	float vx;
	float vy;
	float vz;
	void SetPositionAndVelocity(float _x, float _y, float _z, float _vx, float _vy, float _vz)
	{
		x = _x;
		y = _y;
		z = _z;
		vx = _vx;
		vy = _vy;
		vz = _vz;
	}
};