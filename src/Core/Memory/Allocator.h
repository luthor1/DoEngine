#pragma once

#include "Core/Base.h"
#include <cstddef>

namespace DoEngine {

    class Allocator {
    public:
        Allocator(size_t size, void* start)
            : m_Size(size), m_Start(start), m_UsedMemory(0), m_NumAllocations(0) {}

        virtual ~Allocator() {
            m_Start = nullptr;
            m_Size = 0;
        }

        virtual void* Allocate(size_t size, size_t alignment = 0) = 0;
        virtual void Deallocate(void* p) = 0;

        void* GetStart() const { return m_Start; }
        size_t GetSize() const { return m_Size; }
        size_t GetUsedMemory() const { return m_UsedMemory; }
        size_t GetNumAllocations() const { return m_NumAllocations; }

    protected:
        void* m_Start;
        size_t m_Size;
        size_t m_UsedMemory;
        size_t m_NumAllocations;
    };

    namespace PointerUtils {
        inline void* Add(void* p, size_t x) {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + x);
        }

        inline const void* Add(const void* p, size_t x) {
            return reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(p) + x);
        }

        inline uint8_t GetAlignmentAdjustment(const void* address, size_t alignment) {
            size_t adjustment = alignment - (reinterpret_cast<uintptr_t>(address) & (alignment - 1));

            if (adjustment == alignment)
                return 0; // already aligned

            return (uint8_t)adjustment;
        }
    }

}
