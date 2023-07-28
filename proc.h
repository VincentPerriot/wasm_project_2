#pragma once

#include "mesh.h"
#include "vec3.h"

#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
				vertices[i].Pos[0] = (float)pointOnUnitSphere.x();
				vertices[i].Pos[1] = (float)pointOnUnitSphere.y();
				vertices[i].Pos[2] = (float)pointOnUnitSphere.z();

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
		for (auto& face : terrainFaces)
		{
			face.resolution = res;

			face.colors[0] = colors.x;
			face.colors[1] = colors.y;
			face.colors[2] = colors.z;

			face.update();
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

		ImGui::SliderInt("Resolution", &res, 2, 128);            // Edit int using a slider
		ImGui::ColorEdit3("clear color", (float*)&colors); // Edit 3 floats representing a color

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

private:
	ImGuiIO io;
};

