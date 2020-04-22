template <typename... Args>
Value Value::resolve(Args&&... args) const
{
    if (m_vm)
        return m_vm->resolve(this, std::forward<Args>(args)...);
    else
        throw std::runtime_error("Value::resolve couldn't resolve a without a VM");
}