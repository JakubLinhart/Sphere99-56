    #pragma once

#define PTR_CAST(a,b) static_cast<a*>(b)
#define REF_CAST(a,b) static_cast<CRefPtr<a>>(b)

#define CNewPtr CRefPtr
template <class T>
class CRefPtr
{
private:
    T* m_pointer;
public:
    CRefPtr() : m_pointer(0) {}
    CRefPtr(T* p) :
        m_pointer(p) {}
    ~CRefPtr()
    {
    }

    T* GetRefObj() const { return m_pointer; }
    void ReleaseRefObj() {} // STUB
    virtual void UnLink() {} // STUB
    bool IsValidNewObj() const { return true; } // STUB

    CRefPtr& operator=(const CRefPtr& rhs)
    {                             // Assignment operator. 
        if (this == &rhs) return *this;
        m_pointer = rhs.m_pointer;
        return *this;
    }
    T& operator*() { return *m_pointer; }
    T* operator->() { return m_pointer; }
    operator T* () const { return m_pointer; }
    operator bool() const { return m_pointer != 0; }
};
