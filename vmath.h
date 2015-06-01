#ifndef VMATH_H
#define VMATH_H 1
#ifdef _WIN32
#include <cmath>
#include <limits>
#define FLOAT_INF std::numeric_limits<float>::infinity() 
#else
#include <tgmath.h>
#define FLOAT_INF 1.0/0.0
#endif

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

inline float lerp(float x, float y, float t) {
    return x * (1 - t) + y * t;
}

inline v2 lerp(v2 a, v2 b, float t) {
    return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)};
}

inline float sign(v2 a, v2 b, v2 c) {
    return (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);
}

inline bool pointInTriangle (v2 p, v2 a, v2 b, v2 c) {
    bool r1, r2, r3;

    r1 = sign(p, a, b) < 0.0f;
    r2 = sign(p, b, c) < 0.0f;
    r3 = sign(p, c, a) < 0.0f;

    return ((r1 == r2) && (r2 == r3));
}

#endif
