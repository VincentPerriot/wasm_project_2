#pragma once

class vec2 {
public:
	// 2 ways to build
	vec2() : e{ 0,0 } {}
	vec2(double e0, double e1) : e{ e0, e1 } {}

	double x() const { return e[0]; }
	double y() const { return e[1]; }

	vec2& operator += (const vec2& v)
	{
		e[0] += v.e[0];
		e[1] += v.e[1];
		return *this;
	}

	double operator[](int i) const { return e[i]; }
	double& operator[](int i) { return e[i]; }

	double e[4];
};

inline vec2 operator+(const vec2& u, const vec2& v)
{
	return vec2(u.e[0] + v.e[0], u.e[1] + v.e[1]);
}

