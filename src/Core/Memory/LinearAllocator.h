#pragma once

#include "Allocator.h"
#include <cassert>

namespace DoEngine {

    class LinearAllocator : public Allocator {
    public:
        LinearAllocator(size_t size, void* start) : Allocator(size, start), m_CurrentPos(start) {}

        ~LinearAllocator() override {
            m_CurrentPos = nullptr;
        }

        void* Allocate(size_t size, size_t alignment = 0) override {
            assert(size > 0);

            uint8_t adjustment = PointerUtils::GetAlignmentAdjustment(m_CurrentPos, alignment);

            if (m_UsedMemory + adjustment + size > m_Size)
                return nullptr;

            void* alignedAddr = PointerUtils::Add(m_CurrentPos, adjustment);
            m_CurrentPos = PointerUtils::Add(alignedAddr, size);
            m_UsedMemory += (adjustment + size);
            m_NumAllocations++;

            return alignedAddr;
        }

        void Deallocate(void* p) override {
            // Linear allocator does not support single deallocation
            assert(false && "Linear Allocator does not support single deallocation. Use Clear().");
        }

        void Clear() {
            m_UsedMemory = 0;
            m_NumAllocations = 0;
            m_CurrentPos = m_Start;
        }

    private:
        void* m_CurrentPos;
    };

}
