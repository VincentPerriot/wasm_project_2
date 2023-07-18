#pragma once

#include <string>
#include <vector>
#include <GLES3/gl3.h>

#include "vec3.h"
#include "vec2.h"

struct Vertex {
	vec3 Pos;
	vec3 Normal;
	vec3 Color;

	vec2 TexUV;
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
	GLuint VAO;

	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures) 
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		SetMesh();
	}

	~Mesh() {}
	
	void Draw(GLuint programID)
	{

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

		// Set normal ptr
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		// Set color ptr
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));

		// Set TextUV ptr
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexUV));

		// Unbind VAO
		glBindVertexArray(0);

	}
};


