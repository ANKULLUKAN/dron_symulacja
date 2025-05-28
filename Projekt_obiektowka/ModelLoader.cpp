#include "ModelLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include <windows.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp> // potrzebne do dekompozycji

// Globalne kontenery na siatki i korzeń drzewa sceny
std::vector<Mesh> meshes;
Node rootNode;

// Konwersja macierzy z formatu Assimp (aiMatrix4x4) do GLM (glm::mat4)
glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& mat) {
    return glm::transpose(glm::make_mat4(&mat.a1));
}

// Rekurencyjne przetwarzanie węzła drzewa sceny Assimp na własną strukturę Node
Node processNode(aiNode* ainode) {
    Node node;
    node.transform = aiMatrix4x4ToGlm(ainode->mTransformation);
    // Dodaj indeksy siatek przypisanych do tego węzła
    for (unsigned int i = 0; i < ainode->mNumMeshes; i++)
        node.meshIndices.push_back(ainode->mMeshes[i]);
    // Przetwarzaj dzieci rekurencyjnie
    for (unsigned int i = 0; i < ainode->mNumChildren; i++)
        node.children.push_back(processNode(ainode->mChildren[i]));
    return node;
}

// Ładowanie modelu z pliku (np. .gltf, .obj) przy użyciu Assimp
bool loadModel(const std::string& path) {
    Assimp::Importer importer;
    // Wczytaj scenę i przetwórz: triangulacja, generowanie normalnych, łączenie wierzchołków
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
    if (!scene || !scene->HasMeshes()) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    // Przetwarzanie wszystkich siatek w modelu
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* mesh = scene->mMeshes[m];
        Mesh myMesh;
        // Wczytaj wierzchołki
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex;
            vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            vertex.normal = { mesh->mNormals[i].x,  mesh->mNormals[i].y,  mesh->mNormals[i].z };
            myMesh.vertices.push_back(vertex);
        }
        // Wczytaj indeksy (każda twarz/face to trójkąt)
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j)
                myMesh.indices.push_back(face.mIndices[j]);
        }
        // Tworzenie VAO, VBO, EBO dla siatki w OpenGL
        glGenVertexArrays(1, &myMesh.VAO);
        glGenBuffers(1, &myMesh.VBO);
        glGenBuffers(1, &myMesh.EBO);

        glBindVertexArray(myMesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, myMesh.VBO);
        glBufferData(GL_ARRAY_BUFFER,
            myMesh.vertices.size() * sizeof(Vertex),
            myMesh.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myMesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            myMesh.indices.size() * sizeof(unsigned int),
            myMesh.indices.data(), GL_STATIC_DRAW);

        // Ustawienie atrybutów wierzchołków (pozycja)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        meshes.push_back(myMesh);
    }

    // Przetwarzanie hierarchii węzłów sceny
    rootNode = processNode(scene->mRootNode);
    return true;
}

// Indeksy węzłów, które mają być obracane (np. śmigła)
std::vector<int> rotatingNodeIndices = { 125 };

// Rysowanie węzła (i jego dzieci) z opcjonalną rotacją wybranych węzłów
void drawNodeWithRotation(const Node& node, const glm::mat4& parentTransform, GLuint shaderProgram, int& nodeCounter, float& rotationAngle) {
    glm::mat4 localTransform = node.transform;

    // Jeśli ten węzeł jest na liście obracanych, zastosuj dodatkową rotację
    if (std::find(rotatingNodeIndices.begin(), rotatingNodeIndices.end(), nodeCounter) != rotatingNodeIndices.end()) {
        glm::vec3 scale, translation, skew;
        glm::vec4 perspective;
        glm::quat rotation;
        glm::decompose(localTransform, scale, rotation, translation, skew, perspective);

        glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0, 0, 1)); // np. obrót wokół Z
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

        localTransform = T * R * S;
    }

    // Oblicz globalną transformację dla tego węzła
    glm::mat4 globalTransform = parentTransform * localTransform;

    // Rysuj wszystkie siatki przypisane do tego węzła
    for (unsigned int i : node.meshIndices) {
        const Mesh& mesh = meshes[i];
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(globalTransform));
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }

    nodeCounter++; // Zwiększ licznik węzłów (ważne dla identyfikacji obracanych)
    // Rekurencyjnie rysuj dzieci
    for (const Node& child : node.children) {
        drawNodeWithRotation(child, globalTransform, shaderProgram, nodeCounter, rotationAngle);
    }
}
