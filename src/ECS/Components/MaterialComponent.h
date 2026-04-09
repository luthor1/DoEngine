#pragma once

#include "../../Graphics/RHI.h"
#include <string>

namespace DoEngine {

    struct MaterialComponent {
        std::string Name = "DefaultMaterial";
        TextureHandle AlbedoMap;
        BindingSetHandle BindingSet; // GPU-side bindings for this material
        
        // Optional: Sampler, NormalMap, etc.
    };

}
