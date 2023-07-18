#pragma once

class vec2 {
public:
	// 2 ways to build
	vec4() : e{ 0,0 } {}
	vec4(double e0, double e1) : e{ e0, e1 } {}

	double x() const { return e[0]; }
	double y() const { return e[1]; }

	vec4& operator += (const vec4& v)
	{
		e[0] += v.e[0];
		e[1] += v.e[1];
		return *this;
	}

	double operator[](int i) const { return e[i]; }
	double& operator[](int i) { return e[i]; }

	double e[4];
};

inline vec4 operator+(const vec4& u, const vec4& v)
{
	return vec4(u.e[0] + v.e[0], u.e[1] + v.e[1]);
}

