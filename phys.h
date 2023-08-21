#pragma once

#include "mesh.h"
#include "vec3.h"

#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "maths.h"

class Helix
{
public:

	Helix() = default;

	Helix(Mesh a_mesh, int a_resolution, vec3 a_localUp, std::vector<float> a_colors)
	{

		colors = a_colors;

		mesh = a_mesh;
		resolution = a_resolution;
		localUp = a_localUp;

		axisA[0] = localUp.y();
		axisA[1] = localUp.z();
		axisA[2] = localUp.x();
		axisB = cross(localUp, axisA);

		constructMesh();
	}

	void update()
	{
		constructMesh();
	}

	void constructMesh()
	{
		std::vector<Vertex> vertices(resolution * resolution);
		std::vector<GLuint> triangles((resolution - 1) * (resolution - 1) * 6);
		int triIndex = 0;

		for (int y = 0; y < resolution; y++)
		{
			for (int x = 0; x < resolution; x++)
			{
				int i = x + y * resolution;
				vec2 percent = vec2((double)x / (resolution - 1), (double)y / (resolution - 1));
				vec3 pointOnUnitCylinder = localUp + (percent.x() - 0.5) * 2.0 * axisA + (percent.y() - 0.5) * 2.0 * axisB;

				vertices[i].Pos[0] = (float)pointOnUnitCylinder.x();
				vertices[i].Pos[1] = (float)pointOnUnitCylinder.y();
				vertices[i].Pos[2] = (float)pointOnUnitCylinder.z();

				vertices[i].Colors[0] = colors[0];
				vertices[i].Colors[1] = colors[1];
				vertices[i].Colors[2] = colors[2];

				vertices[i].Normal[0] = (float)pointOnUnitCylinder.x();
				vertices[i].Normal[1] = (float)pointOnUnitCylinder.y();
				vertices[i].Normal[2] = (float)pointOnUnitCylinder.z();
				
				// Draw counter Clockwise
				if (x != (resolution - 1) && y != (resolution - 1))
				{
					triangles[triIndex] = i;
					triangles[triIndex + 1] = i + resolution;
					triangles[triIndex + 2] = i + resolution + 1;

					triangles[triIndex + 3] = i;
					triangles[triIndex + 4] = i + resolution + 1;
					triangles[triIndex + 5] = i + 1;
					triIndex += 6;
				}
			}
		}


		mesh = Mesh(vertices, triangles);
	}

	~Helix() {}

public:
	Mesh mesh;
	int resolution;
	vec3 localUp;
	vec3 axisA;
	vec3 axisB;
	std::vector<float> colors;
};
