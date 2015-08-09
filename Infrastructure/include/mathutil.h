
#pragma once

constexpr double PId = 3.14159265358979323846; 
constexpr float PIf = 3.14159265358979323846f;

inline float deg2rad(float degrees) {
	return degrees / (180.0f / PIf);
}
