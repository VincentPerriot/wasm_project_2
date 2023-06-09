#pragma once

class vec4 {
public:
	// 2 ways to build
	vec4() : e{ 0,0,0,0 } {}
	vec4(double e0, double e1, double e2, double e3) : e{ e0, e1, e2, e3 } {}

	double x() const { return e[0]; }
	double y() const { return e[1]; }
	double z() const { return e[2]; }
	double w() const { return e[3]; }

	double r() const { return e[0]; }
	double g() const { return e[1]; }
	double b() const { return e[2]; }
	double a() const { return e[3]; }

	vec4& operator += (const vec4& v)
	{
		e[0] += v.e[0];
		e[1] += v.e[1];
		e[2] += v.e[2];
		e[3] += v.e[3];
		return *this;
	}

	double operator[](int i) const { return e[i]; }
	double& operator[](int i) { return e[i]; }

	double e[4];
};

inline vec4 operator+(const vec4& u, const vec4& v)
{
	return vec4(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2], u.e[3] + v.e[3]);
}

