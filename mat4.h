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

	std::vector<float> toFloatVector() const
	{
		std::vector<float> floatFormatMat;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				floatFormatMat.push_back(static_cast<float>(M[j][i]));
			}
		}

		return floatFormatMat;
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

inline mat4 translation_mat(const vec3& vec)
{
	mat4 translation_mat = mat4{ {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {vec.x(),vec.y(),vec.z(),1}};
	return translation_mat;
}

inline mat4 scale_mat(double s)
{
	mat4 scale_mat = mat4{ {s,0,0,0}, {0,s,0,0}, {0,0,s,0}, {0,0,0,1} };
	return scale_mat;
}

inline mat4 scale_mat3(const vec3& vec)
{
	mat4 scale_mat = mat4{ {vec.x(),0,0,0}, {0,vec.y(),0,0}, {0,0,vec.z(),0}, {0,0,0,1}};
	return scale_mat;
}

inline mat4 pitch_mat(double angle)
{
	double c = cos(degrees_to_radians(angle));
	double s = sin(degrees_to_radians(angle));

	mat4 pitch_mat = mat4{ {1, 0, 0, 0}, {0, c, s, 0}, {0, -s, c, 0}, {0, 0, 0, 1} };

	return pitch_mat;
}

inline mat4 yaw_mat(double angle)
{
	double c = cos(degrees_to_radians(angle));
	double s = sin(degrees_to_radians(angle));

	mat4 pitch_mat = mat4{ {c, 0, -s, 0}, {0, 1, 0, 0}, {s, 0, c, 0}, {0, 0, 0, 1} };

	return pitch_mat;
}

inline mat4 roll_mat(double angle)
{
	double c = cos(degrees_to_radians(angle));
	double s = sin(degrees_to_radians(angle));

	mat4 pitch_mat = mat4{ {c, s, 0, 0}, {-s, c, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1} };

	return pitch_mat;
}

inline mat4 rotate_mat(double angle, const vec3& axis)
{
	double c = cos(degrees_to_radians(angle));
	double s = sin(degrees_to_radians(angle));

	vec3 normalised_axis = unit_vector(axis);
	double ux = normalised_axis.x();
	double uy = normalised_axis.y();
	double uz = normalised_axis.z();

	mat4 rotate_axis_mat = { 
		{c + ux * ux * (1 - c), uy * ux * (1 - c) + uz * s, uz * ux * (1 - c) - uy * s, 0},
		{ux * uy * (1 - c) - uz * s, c + uy * uy * (1 - c), uz * uy * (1 - c) + ux * s, 0},
		{ux * uz * (1 - c) + uy * s, uy * uz * (1 - c) - ux * s, c + uz * uz * (1 - c), 0},
		{0, 0, 0, 1}
	};

	return rotate_axis_mat;
}

inline mat4 translate(mat4 M, const vec3& pos)
{
	mat4 translate = translation_mat(pos);
	return M * translate;
}

inline mat4 scale(mat4 M, double s)
{
	mat4 scale = scale_mat(s);
	return M * scale;
}

inline mat4 scale3(mat4 M, const vec3& vec)
{
	mat4 scale3 = scale_mat3(vec);
	return M * scale3;
}

inline mat4 pitch(mat4 M, double angle)
{
	mat4 pitch = pitch_mat(angle);
	return M * pitch;
}
inline mat4 yaw(mat4 M, double angle)
{
	mat4 yaw = yaw_mat(angle);
	return M * yaw;
}
inline mat4 roll(mat4 M, double angle)
{
	mat4 roll = roll_mat(angle);
	return M * roll;
}

inline mat4 rotate(mat4 M, double angle, vec3 axis)
{
	mat4 rotate = rotate_mat(angle, axis);
	return M * rotate;
}


