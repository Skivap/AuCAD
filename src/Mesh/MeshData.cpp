#include "MeshData.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>

MeshData::MeshData(Shader* shader, Shader* wireframe_shader, Shader* pointcloud_shader, const std::string& filePath)
: m_meshColor(0.8f, 0.2f, 0.2f), m_wireframeColor(1.0f, 1.0f, 1.0f), m_pointsColor(0.1f, 0.1f, 0.9f), lastSelectedVertex(-1) {
    Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile( filePath,
    	aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_SortByPType |
        aiProcess_ValidateDataStructure |
        aiProcess_ImproveCacheLocality);

	if(!scene) {
	    std::cout << "Couldn't load model " << filePath << '\n';
		std::terminate();
	}

	std::cout << "Loading 3D model " << filePath << '\n';

	for(int i = 0; i < /*scene->mNumMeshes*/ 1; i++) {
	    std::vector<Eigen::Vector3f> vertices;
        std::vector<Eigen::Vector3f> normals;
        std::vector<Eigen::Vector3i> indices;

		aiMesh* mesh = scene->mMeshes[i];

		if(mesh->mNormals == nullptr) {
		    std::cout << "NullPtr Normal\n";
		}

		// Vertices & Normals
		for(int j = 0; j < mesh->mNumVertices; j++) {
		    aiVector3D position = mesh->mVertices[j];
			vertices.push_back(Eigen::Vector3f(position.x, position.y, position.z));

			if(mesh->mNormals != nullptr) {
			    aiVector3D normal = mesh->mNormals[j];
				normals.push_back(Eigen::Vector3f(normal.x, normal.y, normal.z));
			}
			else{
			    normals.push_back(Eigen::Vector3f(1.0f, 0.0f, 0.0f));
			}
		}

		// Indices
		for(int j = 0; j < mesh->mNumFaces; j++) {
			aiFace face = mesh->mFaces[j];
			indices.push_back(Eigen::Vector3i(face.mIndices[0], face.mIndices[1], face.mIndices[2]));
		}

		// Push to list of meshes
		init(vertices, normals, indices);
		initVisualizer(shader, wireframe_shader, pointcloud_shader, vertices, normals, indices);
	}

    m_VBOmesh = m_mesh->getVBO();
    m_VBOwireframe = m_wireframe->getVBO();
    resetSelection();

    precomputeARAP();
    saveTimeFrame(0);
}

void MeshData::init(const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals,
                    const std::vector<Eigen::Vector3i>& indices) {
    m_vertices.resize(vertices.size());
    m_triangles.resize(indices.size());
    m_halfEdges.resize(indices.size() * 3);

    // Setup Vertices ========================================
    for(int i = 0; i < vertices.size(); i++) {
        m_vertices[i].pos = vertices[i].cast<double>();
        m_vertices[i].normal = normals[i].cast<double>();
        m_vertices[i].index = i;

        m_vertices[i].originalPos = m_vertices[i].pos;
        // m_vertices[i].deformedRot = Eigen::Matrix4d::Identity();
    }

    // Setup HalfEdge ========================================
    std::map<std::pair<int, int>, HalfEdge*> edgeMap;

    for (int t = 0; t < indices.size(); ++t) {
        const auto& tri = indices[t];
        int i0 = tri[0], i1 = tri[1], i2 = tri[2];

        Triangle& triangle = m_triangles[t];
        triangle.index = t;

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
        std::pair<int, int> k0 = std::make_pair(i0, i1);
        std::pair<int, int> k1 = std::make_pair(i1, i2);
        std::pair<int, int> k2 = std::make_pair(i2, i0);

        auto k0_twin = std::make_pair(i1, i0);
        auto k1_twin = std::make_pair(i2, i1);
        auto k2_twin = std::make_pair(i0, i2);

        if (edgeMap.count(k0_twin)) {
            HalfEdge* twin = edgeMap[k0_twin];
            he0->twin = twin;
            twin->twin = he0;
        } else edgeMap[k0] = he0;

        if (edgeMap.count(k1_twin)) {
            HalfEdge* twin = edgeMap[k1_twin];
            he1->twin = twin;
            twin->twin = he1;
        } else edgeMap[k1] = he1;

        if (edgeMap.count(k2_twin)) {
            HalfEdge* twin = edgeMap[k2_twin];
            he2->twin = twin;
            twin->twin = he2;
        } else edgeMap[k2] = he2;
    }

    for (auto& he : m_halfEdges) {
        if (he.vertex && he.vertex->he == nullptr) {
            he.vertex->he = &he;
        }
    }

    // Setup Edges ========================================
    m_edges.reserve(edgeMap.size());
    for (auto it = edgeMap.begin(); it != edgeMap.end(); ++it) {
        const std::pair<int, int>& key = it->first;
        HalfEdge* he = it->second;
        Edge edge;
        edge.he = he;
        edge.index = m_edges.size();
        m_edges.push_back(edge);

        he->edge = &m_edges[m_edges.size() - 1];
        he->twin->edge = he->edge;
    }

    // Setup Selection =====================================
    m_selectedEdges.resize(m_edges.size(), false);
    m_selectedVertices.resize(m_vertices.size(), false);
    m_selectedTriangles.resize(m_triangles.size(), false);
}

void MeshData::initVisualizer(Shader* shader, Shader* wireframe_shader, Shader* pointcloud_shader,
                              const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals, const std::vector<Eigen::Vector3i>& indices) {
    // Mesh Visualizer
    m_mesh = new Object::Mesh(shader, vertices, normals, indices);

    // Wireframe Visualizer
    {
        std::vector<Eigen::Vector3f> wireframePos;
        std::vector<Eigen::Vector2i> wireframeIndices;

        for (const Vertex& vertex : m_vertices) {
            wireframePos.push_back(vertex.pos.cast<float>());
        }

        for (Edge e: m_edges) {
            HalfEdge* he = e.he;
            wireframeIndices.push_back(Eigen::Vector2i(
                he->vertex->index,
                he->twin->vertex->index
            ));
        }

        m_wireframe = new Object::Wireframe(wireframe_shader, wireframePos, wireframeIndices);
    }

    // Point Cloud Visualizer
    m_pointCloud = new Object::PointCloud(pointcloud_shader, vertices);
}

void MeshData::draw(const CameraParam& cameraParam) {
    m_mesh->draw(cameraParam);
    m_wireframe->draw(cameraParam);
    m_pointCloud->draw(cameraParam, m_selectedVertices);
}
