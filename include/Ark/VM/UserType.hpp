#ifndef ark_vm_usertype
#define ark_vm_usertype

namespace Ark
{
    class UserType
    {
    public:
        // TODO enhance by using template to cast to void* automatically?
        UserType(unsigned type_id, void* data=nullptr);

        inline unsigned type_id()
        {
            return m_type_id;
        }

        inline void* data()
        {
            return m_data;
        }
    
    private:
        unsigned m_type_id;
        void* m_data;
    };
}

#endif