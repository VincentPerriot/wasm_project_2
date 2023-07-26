#pragma once

#include "mesh.h"
#include "vec3.h"

class TerrainFace
{
public:

	TerrainFace() = default;

	TerrainFace(Mesh a_mesh, int a_resolution, vec3 a_localUp)
	{
		mesh = a_mesh;
		resolution = a_resolution;
		localUp = a_localUp;

		axisA[0] = localUp.y();
		axisA[1] = localUp.z();
		axisA[2] = localUp.x();
		axisB = cross(localUp, axisA);

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

				vec3 pointOnUnitCube = localUp + (percent.x() - 0.5) * 2.0 * axisA + (percent.y() - 0.5) * 2.0 * axisB;

				vec3 pointOnUnitSphere = unit_vector(pointOnUnitCube);
				vertices[i].Pos[0] = (float)pointOnUnitSphere.x();
				vertices[i].Pos[1] = (float)pointOnUnitSphere.y();
				vertices[i].Pos[2] = (float)pointOnUnitSphere.z();
				
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
		// TODO -> calculate normals and update the mesh
	}

	~TerrainFace() {}

public:
	Mesh mesh;
	int resolution;
	vec3 localUp;
	vec3 axisA;
	vec3 axisB;

};

class Planet {
public:
	Planet(std::vector<TerrainFace> a_terrainfaces) {
		terrainFaces = a_terrainfaces;
	};

	Planet() = default;

	void Draw(GLuint programID)
	{
		for (auto face : terrainFaces)
		{
			face.mesh.Draw(programID);
		}
	}

	~Planet() {};

public:
	std::vector<TerrainFace> terrainFaces;
};


