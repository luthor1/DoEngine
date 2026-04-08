#pragma once

#include "../Base.h"
#include "TaskScheduler.h"
#include <functional>
#include <memory>
#include <vector>

namespace DoEngine {

    class JobSystem {
    public:
        static void Initialize();
        static void Shutdown();

        // Execution of a task
        static void Execute(std::function<void()> task);

        // Execute a range of tasks (parallel for)
        static void ExecuteRange(uint32 start, uint32 end, std::function<void(uint32, uint32)> task);

        // Wait for all tasks to complete
        static void Wait();

    private:
        static enki::TaskScheduler g_Scheduler;
    };

}
