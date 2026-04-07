#pragma once

#include "Allocator.h"
#include <cassert>

namespace DoEngine {

    class StackAllocator : public Allocator {
    public:
        struct AllocationHeader {
            uint8_t Adjustment;
        };

        StackAllocator(size_t size, void* start) : Allocator(size, start), m_CurrentPos(start) {}

        ~StackAllocator() override {
            m_CurrentPos = nullptr;
        }

        void* Allocate(size_t size, size_t alignment = 0) override {
            assert(size > 0);

            size_t adjustment = PointerUtils::GetAlignmentAdjustment(m_CurrentPos, alignment);

            if (m_UsedMemory + adjustment + size > m_Size)
                return nullptr;

            void* alignedAddr = PointerUtils::Add(m_CurrentPos, adjustment);
            
            // Add header
            AllocationHeader* header = (AllocationHeader*)(PointerUtils::Add(alignedAddr, -sizeof(AllocationHeader)));
            header->Adjustment = (uint8_t)adjustment;

            m_CurrentPos = PointerUtils::Add(alignedAddr, size);
            m_UsedMemory += (adjustment + size);
            m_NumAllocations++;

            return alignedAddr;
        }

        void Deallocate(void* p) override {
            // Stack allocator supports deallocation in reverse order
            AllocationHeader* header = (AllocationHeader*)(PointerUtils::Add(p, -sizeof(AllocationHeader)));
            
            m_UsedMemory -= (reinterpret_cast<uintptr_t>(m_CurrentPos) - reinterpret_cast<uintptr_t>(p) + header->Adjustment);
            m_CurrentPos = PointerUtils::Add(p, -header->Adjustment);
            m_NumAllocations--;
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
