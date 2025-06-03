#include "ModelLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include <windows.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp> // potrzebne do dekompozycji

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
bool LoadModel(const std::string& path) {
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
            if (mesh->mTextureCoords[0]) // Sprawdź, czy są współrzędne tekstur
                vertex.texCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            else
                vertex.texCoord = glm::vec2(0.0f, 0.0f);
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

		// Ustawienie atrybutów wierzchołków (pozycja, normalna, współrzędne tekstury)
        myMesh.textureID = 0;
        if (scene->HasMaterials() && mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString texPath;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
                    std::string texName = texPath.C_Str();
                    if (!texName.empty() && texName[0] == '*') {
                        // Tekstura osadzona w pliku GLTF
                        int texIndex = std::atoi(texName.c_str() + 1);
                        const aiTexture* texture = scene->mTextures[texIndex];
                        if (texture) {
                            if (texture->mHeight == 0) {
                                // Compressed (np. PNG/JPG)
                                myMesh.textureID = LoadTextureFromMemory(
                                    reinterpret_cast<unsigned char*>(texture->pcData),
                                    texture->mWidth
                                );
                            }
                            else {
                                // Uncompressed (rzadko spotykane, np. RAW RGBA)
                                std::cerr << "Uncompressed embedded textures are not supported." << std::endl;
                            }
                        }
                    }
                    else {
                        // Zwykła tekstura z pliku
                        std::string dir = path.substr(0, path.find_last_of("/\\"));
                        std::string fullPath = dir + "/" + texPath.C_Str();
                        myMesh.textureID = LoadTexture(fullPath);
                    }
                }
            }
        }

        // Pozycja: location 0
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        // Normal: location 1
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        // TexCoord: location 2
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        meshes.push_back(myMesh);
    }

    // Przetwarzanie hierarchii węzłów sceny
    rootNode = processNode(scene->mRootNode);
    return true;
}

GLuint LoadTextureFromMemory(unsigned char* data, int size) {
    int width, height, nrChannels;
    unsigned char* imgData = stbi_load_from_memory(data, size, &width, &height, &nrChannels, 0);
    if (!imgData) {
        std::cerr << "Failed to load embedded texture from memory" << std::endl;
        return 0;
    }
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imgData);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(imgData);
    return texture;
}

// Ładowanie tekstury z pliku przy użyciu stb_image
GLuint LoadTexture(const std::string& path) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return texture;
}


void drawNodeWithRotation(const Node& node, const glm::mat4& parentTransform, GLuint shaderProgram, int& nodeCounter, float& tiltAngleX, float& tiltAngleY)
{
    // Ograniczenie kąta przechyłu do 10 stopni (w radianach)
    constexpr float maxTilt = glm::radians(10.0f);
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

        if (mesh.textureID) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.textureID);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        }

        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }

    nodeCounter++;
    for (const Node& child : node.children) {
        drawNodeWithRotation(child, globalTransform, shaderProgram, nodeCounter, tiltAngleX, tiltAngleY);
    }
}

