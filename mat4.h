#include <cmath>

#include "vec4.h"
#include "vec3.h"
#include "maths.h"
#include <iostream>
#include <cmath>

class mat4 {
public:
	// Allow build with Columns only
	mat4() : M{ c0, c1, c2, c3 } {}
	mat4(vec4 c0, vec4 c1, vec4 c2, vec4 c3) : M{ c0, c1, c2, c3 } {}

	mat4& operator += (const mat4& B)
	{
		c0 += B.c0;
		c1 += B.c1;
		c2 += B.c2;
		c3 += B.c3;
		return *this;
	}

	vec4 operator[](int i) const { return M[i]; }
	vec4& operator[](int i) { return M[i]; }

	void print()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				std::cout << M[j][i] << " ";
			}
			std::cout << std::endl;
		}
	}

	// Initialise at Identity by default
	vec4 c0 = { 1, 0, 0, 0 };
	vec4 c1 = { 0, 1, 0, 0 };
	vec4 c2 = { 0, 0, 1, 0 };
	vec4 c3 = { 0, 0, 0, 1 };
	vec4 M[4];
};

inline mat4 operator*(const mat4 A, const mat4 B)
{
	mat4 C = {
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
	};

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				C[i][k] += B[i][j] * A[j][k];
			}
		}
	}
	return C;
}

inline mat4 translation_mat(vec3 vec)
{
	mat4 translation_mat = mat4{ {1,0,0,vec.x()}, {0,1,0,vec.y()}, {0,0,1,vec.z()}, {0,0,0,1}};
	return translation_mat;
}

inline mat4 scale_mat(double s)
{
	mat4 scale_mat = mat4{ {s,0,0,0}, {0,s,0,0}, {0,0,s,0}, {0,0,0,1} };
	return scale_mat;
}

inline mat4 scale_mat3(vec3 vec)
{
	mat4 scale_mat = mat4{ {vec.x(),0,0,0}, {0,vec.y(),0,0}, {0,0,vec.z(),0}, {0,0,0,1}};
	return scale_mat;
}

inline mat4 translation_mat(double x, double y, double z)
{
	mat4 translation_mat = mat4{ {1,0,0,x}, {0,1,0,y}, {0,0,1,z}, {0,0,0,1} };
	return translation_mat;
}

inline mat4 pitch_mat(double angle)
{
	double cos = cos(degrees_to_radians(angle));
}

