#include "Mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <set>
#include <map>
#include <algorithm>

Object::Mesh::Mesh(Shader* shader, const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals,
                   const std::vector<Eigen::Vector3i>& indices) : Base(shader) {
    // Check =================================================
    if(vertices.size() != normals.size()) {
        std::cout << "Vertices and Normals doesn't match \n";
    }

    // Setup HalfEdge ========================================
    m_vertices.resize(vertices.size());
    m_triangles.resize(indices.size());
    m_halfEdges.resize(indices.size() * 3);

    for(int i = 0; i < vertices.size(); i++) {
        m_vertices[i].pos = vertices[i];
        m_vertices[i].normal = normals[i];
    }

    auto edge_key = [](int a, int b) {
        return std::make_pair(std::min(a, b), std::max(a, b));
    };

    std::map<std::pair<int, int>, HalfEdge*> edgeMap;

    for (int t = 0; t < indices.size(); ++t) {
        const auto& tri = indices[t];
        int i0 = tri[0], i1 = tri[1], i2 = tri[2];

        Triangle& triangle = m_triangles[t];

        HalfEdge* he0 = &m_halfEdges[t * 3 + 0];
        HalfEdge* he1 = &m_halfEdges[t * 3 + 1];
        HalfEdge* he2 = &m_halfEdges[t * 3 + 2];

        // Assign next/prev
        he0->next = he1; he1->next = he2; he2->next = he0;
        he0->prev = he2; he1->prev = he0; he2->prev = he1;

        // Assign vertices
        he0->vertex = &m_vertices[i1]; // from i0 to i1 → store i1 at the end of edge
        he1->vertex = &m_vertices[i2];
        he2->vertex = &m_vertices[i0];

        // Assign face
        he0->face = &triangle;
        he1->face = &triangle;
        he2->face = &triangle;

        // Assign triangle pointer
        triangle.he = he0;

        // Save halfedges to edgeMap to set twin
        auto k0 = edge_key(i0, i1);
        auto k1 = edge_key(i1, i2);
        auto k2 = edge_key(i2, i0);

        if (edgeMap.count(k0)) {
            HalfEdge* twin = edgeMap[k0];
            he0->twin = twin;
            twin->twin = he0;
        } else edgeMap[k0] = he0;

        if (edgeMap.count(k1)) {
            HalfEdge* twin = edgeMap[k1];
            he1->twin = twin;
            twin->twin = he1;
        } else edgeMap[k1] = he1;

        if (edgeMap.count(k2)) {
            HalfEdge* twin = edgeMap[k2];
            he2->twin = twin;
            twin->twin = he2;
        } else edgeMap[k2] = he2;
    }

    for (auto& he : m_halfEdges) {
        if (he.vertex && he.vertex->he == nullptr) {
            he.vertex->he = &he;
        }
    }

    // Setup Buffer Object ===================================
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

Object::Wireframe* Object::Mesh::createWireframe(Shader* wireframeShader) {
    std::vector<Eigen::Vector3f> pos;
    std::vector<Eigen::Vector2i> indices;

    for(const Vertex& vertex: m_vertices) {
        pos.push_back(vertex.pos);
    }

    std::set<std::pair<unsigned int, unsigned int>> edgeSet;
    auto edge_key = [](unsigned int a, unsigned int b) {
        return std::make_pair(std::min(a, b), std::max(a, b));
    };

    for(int i = 1; i < m_indices.size(); i++) {
        auto k = edge_key(m_indices[i], m_indices[i-1]);
        if(!edgeSet.count(k)) {
            edgeSet.insert(k);
            indices.push_back(Eigen::Vector2i(m_indices[i], m_indices[i-1]));
        }
    }

    Object::Wireframe* wireframe = new Wireframe(wireframeShader, pos, indices);
    return wireframe;
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
