#include <cmath>
#include <vector>

#include "vec4.h"

class mat4 {
	public:
		// 2 ways to build
		mat4() : M{ c0{0,0,0,0}, c1{0,0,0,0}, c2{0,0,0,0}, c3{0,0,0,0} } {}
		mat4(vec4 c0, vec4 c1, vec4 c2, vec4 c3) : M{c0, c1, c2, c3} {}
};

inline mat4 operator*(const mat4& , con)
{
	for 
}
