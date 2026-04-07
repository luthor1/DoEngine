#pragma once

#include "../Core/Base.h"
#include "entt/entt.hpp"
#include <utility>

namespace DoEngine {

class Scene {
public:
  Scene();
  ~Scene();

  // Entity management
  entt::entity CreateEntity();
  void DestroyEntity(entt::entity entity);

  // Component management
  template <typename T, typename... Args>
  T &AddComponent(entt::entity entity, Args &&...args) {
    return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
  }

  template <typename T> void RemoveComponent(entt::entity entity) {
    m_Registry.remove<T>(entity);
  }

  template <typename T> T &GetComponent(entt::entity entity) {
    return m_Registry.get<T>(entity);
  }

  template <typename T> bool HasComponent(entt::entity entity) {
    return m_Registry.all_of<T>(entity);
  }

  // View for systems
  template <typename... Component> auto View() {
    return m_Registry.view<Component...>();
  }

  void OnUpdate(float deltaTime);

private:
  entt::registry m_Registry;
};

} // namespace DoEngine
