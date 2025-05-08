#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Assimp::Importer importer;

// Za³aduj plik .gltf
const aiScene* scene = importer.ReadFile("model.gltf", aiProcess_Triangulate | aiProcess_FlipWindingOrder);

// SprawdŸ, czy plik zosta³ poprawnie za³adowany
if (!scene) {
    std::cerr << "Failed to load model: " << importer.GetErrorString() << std::endl;
    return;
}

// Przechodzimy przez wszystkie modele w scenie (najczêœciej bêdziesz mieæ tylko jeden)
for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[i];
    // Tutaj bêdziesz musia³ przetworzyæ wierzcho³ki i inne dane z siatki
}
