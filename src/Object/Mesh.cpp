#include "Mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Object::Mesh::Mesh(Shader* shader, Shader* wireframe_shader,
                   const std::vector<Eigen::Vector3f>& vertices,
                   const std::vector<Eigen::Vector3f>& normals,
                   const std::vector<Eigen::Vector3i>& indices)
    : Base(shader), m_baseColor(1.0f, 0.5f, 0.5f), m_drawWireframe(false), m_drawPointCloud(false) {

    if (vertices.size() != normals.size()) {
        std::cout << "Vertices and Normals doesn't match\n";
    }

    std::vector<float> buffer;
    std::vector<float> bufferPositions, bufferNormals, bufferColors;

    for (const auto& tri : indices) {
        for (int j = 0; j < 3; ++j) {
            int idx = tri[j];
            const auto& v = vertices[idx];
            const auto& n = normals[idx];

            bufferPositions.push_back(v.x());
            bufferPositions.push_back(v.y());
            bufferPositions.push_back(v.z());

            bufferNormals.push_back(n.x());
            bufferNormals.push_back(n.y());
            bufferNormals.push_back(n.z());

            bufferColors.push_back(m_baseColor.x());
            bufferColors.push_back(m_baseColor.y());
            bufferColors.push_back(m_baseColor.z());
        }
    }

    buffer.clear();
    buffer.insert(buffer.end(), bufferPositions.begin(), bufferPositions.end());
    buffer.insert(buffer.end(), bufferNormals.begin(), bufferNormals.end());
    buffer.insert(buffer.end(), bufferColors.begin(), bufferColors.end());

    // INDICES ONLY FOR WIREFRAME SETUP!!!
    std::vector<unsigned int> bufferIndices;
    for(int i = 0; i < indices.size(); i++) {
        bufferIndices.push_back(indices[i].x());
        bufferIndices.push_back(indices[i].y());
        bufferIndices.push_back(indices[i].z());
    }

    bufferSize = buffer.size();
    indicesSize = 0;

    // Setup the data structure
    meshData = new MeshData(vertices, normals, indices, VBO);

    init(buffer, bufferIndices);
    initWireframe(wireframe_shader);
    initPointCloud(shader);
}

Object::Mesh::~Mesh() {}

void Object::Mesh::init(std::vector<float>& buffer, std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);

    size_t vertexCount = buffer.size() / 9;
    size_t verticesOffset = 0;
    size_t normalsOffset = vertexCount * 3 * sizeof(float);
    size_t colorsOffset  = vertexCount * 6 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)verticesOffset);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)normalsOffset);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)colorsOffset);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Object::Mesh::initWireframe(Shader* wireframeShader) {
    std::vector<Eigen::Vector3f> pos;
    std::vector<Eigen::Vector2i> indices;

    const std::vector<Vertex>& vertices = meshData->getVertices();
    const std::vector<Edge>& edges = meshData->getEdges();

    for (const Vertex& vertex : vertices) {
        pos.push_back(vertex.pos);
    }

    for (Edge e: edges) {
        HalfEdge* he = e.he;
        indices.push_back(Eigen::Vector2i(
            he->vertex->index,
            he->prev->vertex->index
        ));
    }

    m_wireframe = new Wireframe(wireframeShader, pos, indices);
}

void Object::Mesh::initPointCloud(Shader* shader) {
    const std::vector<Vertex>& vertices = meshData->getVertices();
    std::vector<Eigen::Vector3f> pos;
    for (const Vertex& vertex : vertices) {
        pos.push_back(vertex.pos);
    }
    m_pointCloud = new PointCloud(shader, pos);
}

std::vector<Object::Mesh*> Object::Mesh::loadMeshes(Shader* shader, Shader* wireframe_shader, const std::string& filePath){
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

	std::cout << "Loading 3D model " << filePath << '\n';

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
		meshes.push_back(new Object::Mesh(shader, wireframe_shader, vertices, normals, indices));
	}

    return meshes;
}

void Object::Mesh::draw(const CameraParam& cameraParam) {
    shader->use();
    shader->setMat4("projection", cameraParam.projection);
    shader->setMat4("view", cameraParam.view);
    shader->setVec3("camPos", cameraParam.position);
    shader->setMat4("model", modelMatrix);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, bufferSize / 9);

    m_wireframe->draw(cameraParam);
    m_pointCloud->draw(cameraParam);
}
