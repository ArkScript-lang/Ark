template <typename... Args>
Value Value::resolve(Args&&... args) const
{
    if (m_vmf)
        return m_vmf->resolve(this, std::forward<Args>(args)...);
    else if (m_vmt)
        return m_vmt->resolve(this, std::forward<Args>(args)...);
    else
        throw std::runtime_error("Value::resolve couldn't resolve a without a VM");
}