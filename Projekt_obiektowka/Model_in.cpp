#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Assimp::Importer importer;

// Za�aduj plik .gltf
const aiScene* scene = importer.ReadFile("model.gltf", aiProcess_Triangulate | aiProcess_FlipWindingOrder);

// Sprawd�, czy plik zosta� poprawnie za�adowany
if (!scene) {
    std::cerr << "Failed to load model: " << importer.GetErrorString() << std::endl;
    return;
}

// Przechodzimy przez wszystkie modele w scenie (najcz�ciej b�dziesz mie� tylko jeden)
for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[i];
    // Tutaj b�dziesz musia� przetworzy� wierzcho�ki i inne dane z siatki
}
