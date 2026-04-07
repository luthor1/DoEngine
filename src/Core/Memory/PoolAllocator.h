#pragma once

#include "Allocator.h"
#include <cassert>

namespace DoEngine {

    class PoolAllocator : public Allocator {
    public:
        PoolAllocator(size_t objectSize, size_t objectAlignment, size_t size, void* start)
            : Allocator(size, start), m_ObjectSize(objectSize), m_ObjectAlignment(objectAlignment) {
            
            assert(objectSize >= sizeof(void*));

            // Create free list
            uint8_t adjustment = PointerUtils::GetAlignmentAdjustment(start, objectAlignment);
            m_FreeList = reinterpret_cast<void**>(PointerUtils::Add(start, adjustment));

            size_t numObjects = (size - adjustment) / objectSize;

            void** current = m_FreeList;

            for (size_t i = 0; i < numObjects - 1; ++i) {
                *current = PointerUtils::Add(current, objectSize);
                current = reinterpret_cast<void**>(*current);
            }

            *current = nullptr;
        }

        ~PoolAllocator() override {
            m_FreeList = nullptr;
        }

        void* Allocate(size_t size, size_t alignment = 0) override {
            assert(size == m_ObjectSize && alignment == m_ObjectAlignment);

            if (m_FreeList == nullptr)
                return nullptr;

            void* p = m_FreeList;
            m_FreeList = reinterpret_cast<void**>(*m_FreeList);

            m_UsedMemory += m_ObjectSize;
            m_NumAllocations++;

            return p;
        }

        void Deallocate(void* p) override {
            *(reinterpret_cast<void**>(p)) = m_FreeList;
            m_FreeList = reinterpret_cast<void**>(p);

            m_UsedMemory -= m_ObjectSize;
            m_NumAllocations--;
        }

    private:
        size_t m_ObjectSize;
        size_t m_ObjectAlignment;
        void** m_FreeList;
    };

}
