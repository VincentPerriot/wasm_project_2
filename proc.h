#pragma once

#include "mesh.h"
#include "vec3.h"

#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "maths.h"

class TerrainFace
{
public:

	TerrainFace() = default;

	TerrainFace(Mesh a_mesh, int a_resolution, vec3 a_localUp, std::vector<float> a_colors)
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

				vec3 pointOnUnitCube = localUp + (percent.x() - 0.5) * 2.0 * axisA + (percent.y() - 0.5) * 2.0 * axisB;

				vec3 pointOnUnitSphere = unit_vector(pointOnUnitCube);
				if (!elevations.empty())
				{
					std::cout << "applying noise" << std::endl;
					vertices[i].Pos[0] = (float)pointOnUnitSphere.x() * (1 + elevations[i].x());
					vertices[i].Pos[1] = (float)pointOnUnitSphere.y() * (1 + elevations[i].y());
					vertices[i].Pos[2] = (float)pointOnUnitSphere.z() * (1 + elevations[i].z());
				}
				else
				{
					std::cout << "generating without noise" << std::endl;
					vertices[i].Pos[0] = (float)pointOnUnitSphere.x();
					vertices[i].Pos[1] = (float)pointOnUnitSphere.y();
					vertices[i].Pos[2] = (float)pointOnUnitSphere.z();
				}

				vertices[i].Colors[0] = colors[0];
				vertices[i].Colors[1] = colors[1];
				vertices[i].Colors[2] = colors[2];

				vertices[i].Normal[0] = (float)pointOnUnitSphere.x();
				vertices[i].Normal[1] = (float)pointOnUnitSphere.y();
				vertices[i].Normal[2] = (float)pointOnUnitSphere.z();
				
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

	~TerrainFace() {}

public:
	Mesh mesh;
	int resolution;
	vec3 localUp;
	vec3 axisA;
	vec3 axisB;
	std::vector<float> colors;
	std::vector<vec3> elevations;
};


class PerlinNoise {
public:
	PerlinNoise()
	{
		// Initialise a vector of 256 random numbers from -1 to 1
		randvec.resize(256);
		for (int i = 0; i < 256; i++)
		{
			randvec[i] = unit_vector(vec3::random(-1, 1));
		}
		
		perm_x = generate_perm();
		perm_y = generate_perm();
		perm_z = generate_perm();
	}

	double noise(const vec3& pos)
	{
		// Save the remainder of each coordinates
		double u = pos.x() - floor(pos.x());
		double v = pos.y() - floor(pos.y());
		double w = pos.z() - floor(pos.z());
		
		// Save the integer part of each coordinates
		int i = static_cast<int>(floor(pos.x()));
		int j = static_cast<int>(floor(pos.y()));
		int k = static_cast<int>(floor(pos.z()));
		vec3 c[2][2][2];
		//Access perm array with index calculated by adding i and di, then adding bitwise wrap around 255
		// Essentially realising suffled permutations on a 3d cube
		for (int di = 0; di < 2; di++)
		{
			for (int dj = 0; dj < 2; dj++)
			{
				for (int dk = 0; dk < 2; dk++)
				{
					c[di][dj][dk] = randvec[
						perm_x[(i + di) & 255] ^
						perm_y[(j + dj) & 255] ^
						perm_z[(k + dk) & 255]];
				}
			}
		}
		return trilinear_interp(c, u, v, w);
	}

	~PerlinNoise() {
	}

private:
	std::vector<vec3> randvec;
	std::vector<int> perm_x;
	std::vector<int> perm_y;
	std::vector<int> perm_z;

	// For n integers in array p, permute with a random position from 0 to index
	void permute(std::vector<int> p, int n)
	{
		for (int i = n - 1; i > 0; i--)
		{
			int target = random_int(0, i);
			int tmp = p[i];
			p[i] = p[target];
			p[target] = tmp;
		}
	}
	
	// Fill array with n integers then permute to random positions in the array
	std::vector<int> generate_perm()
	{
		std::vector<int> p(256);

		for (int i = 0; i < 256; i++)
		{
			p[i] = i;
		}
		permute(p, 256);

		return p;
	}

	// Trilinear interpolation, directly from RayTracing in a week book
	double trilinear_interp(vec3 c[2][2][2], double u, double v, double w)
		{
		// Hermitian Smoothing
		auto uu = u * u * (3 - 2 * u);
		auto vv = v * v * (3 - 2 * v);
		auto ww = w * w * (3 - 2 * w);
		auto accum = 0.0;

		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				for (int k = 0; k < 2; k++)
				{
					vec3 weight_v(u - i, v - j, w - k);
					accum += (i * uu + (1 - i) * (1 - uu))
						* (j * vv + (1 - j) * (1 - vv))
						* (k * ww + (1 - k) * (1 - ww))
						* dot(c[i][j][k], weight_v);
				}

		return accum;
	}
};


class NoiseLayer {
public:
	NoiseLayer() {}

	NoiseLayer(double a_scale) {
		noise = PerlinNoise();
		scale = a_scale;
	};

	vec3 values(const vec3& pos)
	{
		return vec3(1, 1, 1) * 0.5 * (1.0 + noise.noise(scale * pos));
	}

	~NoiseLayer() {}

private:
	PerlinNoise noise;
	double scale;
};


class Planet {
public:
	Planet(std::vector<TerrainFace> a_terrainfaces) 
	{
		terrainFaces = a_terrainfaces;
		// All faces have same color and res, just take first one
		colors = ImVec4(terrainFaces[0].colors[0], terrainFaces[0].colors[1], terrainFaces[0].colors[2], 1.0f);
		res = terrainFaces[0].resolution;
	}

	Planet() = default;

	void Draw(GLuint programID)
	{
		for (auto& face : terrainFaces)
		{
			face.mesh.Draw(programID);
		}
	}

	void setBaseGUI(GLFWwindow* window)
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.2f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 300 es");

	}

	void update()
	{
		static int prevResolution = res;
		static ImVec4 prevColors = colors;
		static bool prevAddedNoise = addedNoise;

		if (prevResolution != res ||
			prevColors.x != colors.x ||
			prevColors.y != colors.y ||
			prevColors.z != colors.z ||
			prevAddedNoise != addedNoise)
		{
			for (auto& face : terrainFaces)
			{
				face.resolution = res;

				// To get coherent noise on the whole sphere, we need to cut the used elevations
				// Otherwise the next 5 faces will repeat same noise pattern and junction will not work
				if (addedNoise)
				{
					getElevations();
					face.elevations = elevations;
					elevations.erase(elevations.begin(), elevations.begin() + (face.resolution * face.resolution));
				}

				face.colors[0] = colors.x;
				face.colors[1] = colors.y;
				face.colors[2] = colors.z;

				face.update();
			}
		}

        prevResolution = res;
        prevColors = colors;
		prevAddedNoise = addedNoise;
		elevations = {};
	}

	void getElevations()
	{
		double scale = (double)noiseScale;
		noiseLayer = NoiseLayer(scale);

		for (auto& face : terrainFaces)
		{
			for (auto& vertex : face.mesh.vertices)
			{
				// Built Noise with vec3 of double
				double x = (double)vertex.Pos[0];
				double y = (double)vertex.Pos[1];
				double z = (double)vertex.Pos[2];
				vec3 newPos = noiseLayer.values(vec3(x, y, z));

				elevations.push_back(newPos);
			}
		}
	}

	void waitCheck()
	{
		if (addNoise && !addedNoise)
		{
			//getElevations();
			addedNoise = true;
		}
	}

	void RenderUI(GLFWwindow* window)
	{
		glfwSwapInterval(1); // Enable vsync

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Sphere Explorer!");                          // Create a window

		ImGui::Text("Change settings to observe real time changes");

		ImGui::SliderInt("Resolution", &res, 2, 32);            // Edit int using a slider
		ImGui::ColorEdit3("Sphere color", (float*)&colors); // Edit 3 floats representing a color

		ImGui::Checkbox("Add Noise", &addNoise);
		ImGui::SliderFloat("Noise Scale", &noiseScale, 0.1, 10.0);

		ImGui::End();

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}


	~Planet() {};

public:
	std::vector<TerrainFace> terrainFaces;
	ImVec4 colors;
	int res;
	bool addNoise = false;
	float noiseScale = 2.0;
	NoiseLayer noiseLayer;
	std::vector<vec3> elevations;

private:
	ImGuiIO io;
	bool addedNoise = false;
};


