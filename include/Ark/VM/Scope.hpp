#ifndef ark_vm_scope
#define ark_vm_scope

#include <vector>

#include <Ark/VM/Value.hpp>

namespace Ark::internal
{
    class Scope
    {
    public:
        Scope();
        Scope(std::size_t count, bool owned_by_vm=true);
        Scope(const Scope&) = default;

        inline unsigned id() const
        {
            return m_id;
        }

        inline Value& operator[](std::size_t index)
        {
            return m_locals[index];
        }

        inline bool isOwnedByVM() const
        {
            return m_owned_by_vm;
        }

        inline unsigned refCount() const
        {
            return m_ref_count;
        }

        inline void incRefCount()
        {
            m_ref_count++;
        }

        inline void decRefCount()
        {
            m_ref_count--;
        }

    private:
        static unsigned id_generator;
        unsigned m_id;

        std::vector<Value> m_locals;
        bool m_owned_by_vm;
        unsigned m_ref_count;
    };
}

#endif