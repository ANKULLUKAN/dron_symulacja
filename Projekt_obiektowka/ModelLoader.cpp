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
    std::cout << "Node name: " << ainode->mName.C_Str() << std::endl;
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



// na razie odpuszcam obrot skrzydel
void drawNodeWithRotation(const Node& node, const glm::mat4& parentTransform, GLuint shaderProgram, int& nodeCounter, float& tiltAngleX, float& tiltAngleY)
{
    // Ograniczenie kąta przechyłu do 10 stopni (w radianach)
    const float maxTilt = glm::radians(10.0f);
    tiltAngleX = glm::clamp(tiltAngleX, -maxTilt, maxTilt);
    tiltAngleY = glm::clamp(tiltAngleY, -maxTilt, maxTilt);

    glm::mat4 localTransform = node.transform;

    
    static float wingsAngle = 0.0f;
    if (nodeCounter == 127) { // lewe skrzydlo tylne
        wingsAngle += 0.001f;
        if (wingsAngle > glm::two_pi<float>()) wingsAngle -= glm::two_pi<float>();

        glm::vec3 pivot(-8.4f, 4.2f, 8.4f); 
        glm::mat4 toPivot = glm::translate(glm::mat4(1.0f), pivot);
        glm::mat4 fromPivot = glm::translate(glm::mat4(1.0f), -pivot);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), wingsAngle, glm::vec3(0, 1, 0));
        localTransform = toPivot * rotation * fromPivot * localTransform;
    }
    
    if (nodeCounter == 130) { // lewe skrzydlo z przodu
        wingsAngle += 0.05f;
        if (wingsAngle > glm::two_pi<float>()) wingsAngle -= glm::two_pi<float>();

        glm::vec3 pivot(-8.5f, 4.2f, -8.0f); 
        glm::mat4 toPivot = glm::translate(glm::mat4(1.0f), pivot);
        glm::mat4 fromPivot = glm::translate(glm::mat4(1.0f), -pivot);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), wingsAngle, glm::vec3(0, 1, 0));
        localTransform = toPivot * rotation * fromPivot * localTransform;
    }
    
    if (nodeCounter == 133) { // prawe skrzydlo z tylu 
        wingsAngle += 0.05f;
        if (wingsAngle > glm::two_pi<float>()) wingsAngle -= glm::two_pi<float>();

        glm::vec3 pivot(+8.5f, 4.2f, -8.0f); 
        glm::mat4 toPivot = glm::translate(glm::mat4(1.0f), pivot);
        glm::mat4 fromPivot = glm::translate(glm::mat4(1.0f), -pivot);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), wingsAngle, glm::vec3(0, 1, 0));
        localTransform = toPivot * rotation * fromPivot * localTransform;
    }

	if (nodeCounter == 136) { // prawe skrzydlo z przodu
        wingsAngle += 0.05f;
        if (wingsAngle > glm::two_pi<float>()) wingsAngle -= glm::two_pi<float>();

        glm::vec3 pivot(+8.5f, 4.2f, 8.0f); 
        glm::mat4 toPivot = glm::translate(glm::mat4(1.0f), pivot);
        glm::mat4 fromPivot = glm::translate(glm::mat4(1.0f), -pivot);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), wingsAngle, glm::vec3(0, 1, 0));
        localTransform = toPivot * rotation * fromPivot * localTransform;
    }

    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), tiltAngleX, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), tiltAngleY, glm::vec3(0.0f, 0.0f, -1.0f));
    glm::mat4 globalTransform = parentTransform * rotationY * rotationX * localTransform;

    for (unsigned int i : node.meshIndices) {
        const Mesh& mesh = meshes[i];
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(globalTransform));
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }

    nodeCounter++;
    for (const Node& child : node.children) {
        drawNodeWithRotation(child, globalTransform, shaderProgram, nodeCounter, tiltAngleX, tiltAngleY);
    }
}

