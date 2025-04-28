#include "Mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Object::Mesh::Mesh(Shader* shader, const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals,
                   const std::vector<Eigen::Vector3i>& indices) : Base(shader) {
    // Check
    if(vertices.size() != normals.size()) {
        std::cout << "Vertices and Normals doesn't match \n";
    }

    // Setup Buffer Object
    for(int i = 0; i < vertices.size(); i++) {
        m_buffer.push_back(vertices[i].x());
        m_buffer.push_back(vertices[i].y());
        m_buffer.push_back(vertices[i].z());

        m_buffer.push_back(normals[i].x());
        m_buffer.push_back(normals[i].y());
        m_buffer.push_back(normals[i].z());
    }
    for(int i = 0; i < indices.size(); i++) {
        m_indices.push_back(indices[i].x());
        m_indices.push_back(indices[i].y());
        m_indices.push_back(indices[i].z());
    }
    init();
}

Object::Mesh::~Mesh() {

}

void Object::Mesh::init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_buffer.size() * sizeof(float), m_buffer.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Object::Mesh::draw(const CameraParam& cameraParam) {
    shader->use();
    shader->setMat4("projection", cameraParam.projection);
    shader->setMat4("view", cameraParam.view);
    shader->setVec3("camPos", cameraParam.position);
    shader->setMat4("model", modelMatrix);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
}

std::vector<Object::Mesh*> Object::Mesh::loadMeshes(Shader* shader, const std::string& filePath){
    std::vector<Object::Mesh*> meshes;

    Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile( filePath,
		aiProcess_CalcTangentSpace       |
		aiProcess_Triangulate            |
		aiProcess_JoinIdenticalVertices  |
		aiProcess_SortByPType);

	if(!scene) {
	    std::cout << "Couldn't load model " << filePath << '\n';
		std::terminate();
	}

	std::cout << "Load 3D models " << filePath << '\n';

	for(int i = 0; i < scene->mNumMeshes; i++) {
	    std::vector<Eigen::Vector3f> vertices;
        std::vector<Eigen::Vector3f> normals;
        std::vector<Eigen::Vector3i> indices;

		aiMesh* mesh = scene->mMeshes[i];

		// Vertices & Normals
		for(int j = 0; j < mesh->mNumVertices; j++) {
		    aiVector3D position = mesh->mVertices[j];
			vertices.push_back(Eigen::Vector3f(position.x, position.y, position.z));

			aiVector3D normal = mesh->mNormals[j];
			normals.push_back(Eigen::Vector3f(normal.x, normal.y, normal.z));
		}

		// Indices
		for(int j = 0; j < mesh->mNumFaces; j++) {
			aiFace face = mesh->mFaces[j];
			indices.push_back(Eigen::Vector3i(face.mIndices[0], face.mIndices[1], face.mIndices[2]));
		}

		// Push to list of meshes
		meshes.push_back(new Object::Mesh(shader, vertices, normals, indices));
	}

    return meshes;
}
