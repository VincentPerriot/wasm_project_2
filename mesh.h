#pragma once

#include <string>
#include <vector>
#include <GLES3/gl3.h>

#include "vec3.h"
#include "vec2.h"

struct Vertex {
	float Pos[3];
	float Colors[3] = { 1.0, 1.0, 1.0 };
	float TexUV[2];
	float Normal[3];
};

struct Texture {
	unsigned int id;
	std::string path;
	std::string type;
};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	
	std::vector<Texture> textures;
	std::vector<Texture> defaultTextures;
	GLuint VAO;

	Mesh() = default;

	Mesh(std::vector<Vertex> a_vertices, std::vector<GLuint> a_indices)
	{
		vertices = a_vertices;
		indices = a_indices;
		textures = defaultTextures;

		SetMesh();
	}

	Mesh(std::vector<Vertex> a_vertices, std::vector<GLuint> a_indices, std::vector<Texture> a_textures) 
	{
		vertices = a_vertices;
		indices = a_indices;
		textures = a_textures;

		SetMesh();
	}

	~Mesh() {}
	
	void Draw(GLuint programID)
	{
		unsigned int diffuseNum = 1;
		unsigned int specularNum = 1;
		unsigned int normalNum = 1;
		unsigned int heightNum = 1;
	
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			// Activate proper texture unit
			glActiveTexture(GL_TEXTURE0 + i);
			std::string number;
			std::string name = textures[i].type;

			if (name == "texture_diffuse")
				number = std::to_string(diffuseNum++);
			else if (name == "texture_specular")
				number = std::to_string(specularNum++);
			else if (name == "texture_normal")
				number = std::to_string(normalNum++);
			else if (name == "texture_height")
				number = std::to_string(heightNum++);

			// Set sampler to tex unit
			glUniform1i(glGetUniformLocation(programID, (name + number).c_str()), i);
			// Bind tex
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

		// Set everything back to default
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);

	}

private:
	GLuint VBO, EBO;


	void SetMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);


		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

		// Set Vertex attrib ptr
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		// Set Color attrib
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Colors));

		// Set TexUV ptr
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexUV));

		// Set normal ptr
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		// Unbind VAO
		glBindVertexArray(0);

	}
};


