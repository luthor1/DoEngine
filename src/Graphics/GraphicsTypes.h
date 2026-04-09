#pragma once
#include <glm/glm.hpp>

namespace DoEngine {

    struct Vertex {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec3 Normal;
        glm::vec2 TexCoord;
    };

    struct UniformData {
        glm::mat4 MVP;
    };

} // namespace DoEngine
