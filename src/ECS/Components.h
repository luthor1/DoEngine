#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Graphics/RHI.h"

#include "Components/MaterialComponent.h"

namespace DoEngine {

    struct TagComponent {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag) : Tag(tag) {}
    };

    struct TransformComponent {
        glm::mat4 Transform = glm::mat4(1.0f);

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::mat4& transform) : Transform(transform) {}
    };

    struct MeshComponent {
        BufferHandle VertexBuffer;
        PipelineHandle Pipeline;
        uint32_t VertexCount = 0;

        MeshComponent() = default;
        MeshComponent(const MeshComponent&) = default;
    };

}
