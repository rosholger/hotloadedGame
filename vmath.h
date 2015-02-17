#ifndef VMATH_H
#define VMATH_H 1
#include <tgmath.h>
struct v2 {
	float x, y;
};


inline v2 operator+(v2 a, v2 b) {

	v2 ret = {a.x + b.x, a.y + b.y};
	return ret;
};

inline v2 operator-(v2 a, v2 b) {
	v2 ret = {a.x - b.x, a.y - b.y};
	return ret;
};

inline v2 operator*(v2 a, float b) {
	v2 ret = {a.x * b, a.y * b};
	return ret;
};

inline v2 operator*(float b, v2 a) {
	return a * b;
};

inline v2 operator/(v2 a, float b) {
	v2 ret = {a.x / b, a.y / b};
	return ret;
};

inline void operator+=(v2 &a, v2 b) {
	a = a + b;
};

inline void operator-=(v2 &a, v2 b) {
	a = a - b;
};

inline void operator*=(v2 &a, float b) {
	a = a * b;
};

inline void operator/=(v2 &a, float b) {
	a = a / b;
};

inline bool operator==(v2 a, v2 b) {
	return a.x == b.x && a.y == b.y;
};

inline bool operator!=(v2 a, v2 b) {
	return !(a == b);
};

inline float v2dot(v2 a, v2 b) {
	float ret = a.x * b.x + a.y * b.y;
	return ret;
};

inline float v2cross(v2 a, v2 b) {
	return a.x*b.y - a.y*b.x;
};	

inline float v2sqrLen(v2 a) {
	return v2dot(a, a);
};

inline float v2len(v2 a) {
	return sqrt(v2sqrLen(a));
};

inline v2 v2normalize(v2 a) {
	return a == v2{0, 0} ? a : a / v2len(a);
};

#endif
