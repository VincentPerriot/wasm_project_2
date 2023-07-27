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

		this->colors = a_colors;

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
	}

	Planet() = default;

	void Draw(GLuint programID)
	{
		for (auto face : terrainFaces)
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

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 300 es");

	}

	void RenderUI(GLFWwindow* window)
	{
		glfwSwapInterval(1); // Enable vsync

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		// Our state
		bool show_demo_window = true;

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Sphere Explorer!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("Change settings to observe real time changes");     // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
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

private:
	ImGuiIO io;
};

