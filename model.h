#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "stb_image.h"


unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

class Model
{
public:
	Model() = default;
	Model(char *path) 
	{
		loadModel(path);
	}

	void Draw(GLuint& programId)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i].Draw(programId);
		}
	}

	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<Texture> textures_loaded;
	bool gammaCorrection;

	~Model() {}

private:

	void loadModel(std::string path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR ASSIMP: " << importer.GetErrorString() << std::endl;
		}
		
		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, scene);

	}

	void processNode(aiNode* node, const aiScene* scene)
	{
		// Process each node then each children
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		for(unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		// walk mesh vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			float values[3];
			// Note, will use same array to populate different data in vertex
			// positions
			values[0] = mesh->mVertices[i].x;
			values[1] = mesh->mVertices[i].y;
			values[2] = mesh->mVertices[i].z;
			vertex.Pos[0] = values[0];
			vertex.Pos[1] = values[1];
			vertex.Pos[2] = values[2];

			// normals
			if (mesh->HasNormals())
			{
				values[0] = mesh->mNormals[i].x;
				values[1] = mesh->mNormals[i].y;
				values[2] = mesh->mNormals[i].z;
				vertex.Normal[0] = values[0];
				vertex.Normal[1] = values[1];
				vertex.Normal[2] = values[2];
			}

			// Texture
			if (mesh->mTextureCoords[0])
			{
				float val[2];

				val[0] = mesh->mTextureCoords[0][i].x;
				val[1] = mesh->mTextureCoords[0][i].y;
				
				vertex.TexUV[0] = val[0];
				vertex.TexUV[1] = val[1];
				// TODO add tan and bitan here
			}
			else
			{
				vertex.TexUV[0] = 0.0;
				vertex.TexUV[1] = 0.0;
			}

			vertices.push_back(vertex);
		}
		// Walk mesh faces
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// Directly from learnOpenGL at the moment
		// TODO: IMPROVE TEXTURE LOADING
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
			// 1. diffuse maps
			std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// 2. specular maps
			std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			/* FOR LATER USE 
			// 3. normal maps
			std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			// 4. height maps
			std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
			textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
			*/
		}
        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
	}

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat,
		aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);

			}
		}
		return textures;
	}

};

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureId;
	glGenTextures(1, &textureId);

	int width, height, numComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &numComponents, 0);

	if (data)
	{
		GLenum format;
		if (numComponents == 1)
			format = GL_RED;
		else if (numComponents == 3)
			format = GL_RGB;
		else if (numComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureId;

}

