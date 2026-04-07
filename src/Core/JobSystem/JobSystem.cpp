#include "JobSystem.h"
#include <iostream>

namespace DoEngine {

    enki::TaskScheduler JobSystem::g_Scheduler;

    class TaskDelegate : public enki::ITaskSet {
    public:
        TaskDelegate(std::function<void()> task) : m_Task(std::move(task)) {}

        void ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum) override {
            m_Task();
        }

    private:
        std::function<void()> m_Task;
    };

    class RangeTaskDelegate : public enki::ITaskSet {
    public:
        RangeTaskDelegate(std::function<void(uint32_t, uint32_t)> task, uint32_t size) 
            : enki::ITaskSet(size), m_Task(std::move(task)) {}

        void ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum) override {
            m_Task(range.start, range.end);
        }

    private:
        std::function<void(uint32_t, uint32_t)> m_Task;
    };

    void JobSystem::Initialize() {
        g_Scheduler.Initialize();
        std::cout << "[INFO] Job System initialized with " << g_Scheduler.GetNumTaskThreads() << " threads." << std::endl;
    }

    void JobSystem::Shutdown() {
        g_Scheduler.WaitforAll();
    }

    void JobSystem::Execute(std::function<void()> task) {
        // Here we just use a simplified version for this example.
        // In a real engine we'd pool these TaskSet objects to avoid allocations.
        static std::vector<std::unique_ptr<TaskDelegate>> tasks;
        auto& t = tasks.emplace_back(std::make_unique<TaskDelegate>(std::move(task)));
        g_Scheduler.AddTaskSetToPipe(t.get());
    }

    void JobSystem::ExecuteRange(uint32 start, uint32 end, std::function<void(uint32, uint32)> task) {
        static std::vector<std::unique_ptr<RangeTaskDelegate>> tasks;
        auto& t = tasks.emplace_back(std::make_unique<RangeTaskDelegate>(std::move(task), end - start));
        g_Scheduler.AddTaskSetToPipe(t.get());
    }

    void JobSystem::Wait() {
        g_Scheduler.WaitforAll();
    }

}
