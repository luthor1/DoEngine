#include "Scene.h"

namespace DoEngine {

    Scene::Scene() {
        // Initial setup for the scene if needed
    }

    Scene::~Scene() {
        // Cleanup all entities on destruction
        m_Registry.clear();
    }

    entt::entity Scene::CreateEntity() {
        return m_Registry.create();
    }

    void Scene::DestroyEntity(entt::entity entity) {
        m_Registry.destroy(entity);
    }

    void Scene::OnUpdate(float deltaTime) {
        // Typical update for a system:
        // Here you would run all the engine's built-in systems (Physics, Scripting, etc.)
        // This is a placeholder for where the engine loop would call the systems.
    }

}
