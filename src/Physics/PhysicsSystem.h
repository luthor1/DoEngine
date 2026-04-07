#pragma once

#include "../Core/Base.h"

namespace JPH {
    class PhysicsSystem; // Forward declaration
}

namespace DoEngine {

    class PhysicsSystem {
    public:
        static void Initialize();
        static void Shutdown();

        static void OnUpdate(float deltaTime);

    private:
        static JPH::PhysicsSystem* s_PhysicsSystem;
        // Jolt physics needs a lot of boilerplate (JobSystem, TempAllocator, etc.)
        // We will wrap those in the Initialize call.
    };

}
