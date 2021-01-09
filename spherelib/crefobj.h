    #pragma once

#define PTR_CAST(a,b) dynamic_cast<a*>(b)
#define STATIC_CAST(a,b) static_cast<a*>(b)
#define REF_CAST(a,b) CRefPtr<a>((a*)b.GetRefObj())

#define CNewPtr CRefPtr
template <class T>
class CRefPtr
{
private:
    T* m_pointer;
public:
    CRefPtr() : m_pointer(0) {}
    CRefPtr(T* p) : m_pointer(p) {}
    ~CRefPtr()
    {
    }

    void SetRefObj(T* pObj) { m_pointer = pObj; }
    T* GetRefObj() const { return m_pointer; }
    void ReleaseRefObj() {} // STUB
    virtual void UnLink() {} // STUB
    bool IsValidNewObj() const { return true; } // STUB
    bool IsValidRefObj() const { throw "not implemented"; }
    void Free() { throw "not implemented"; }

    T* DetachObj() { throw "not implemented"; }

    CRefPtr& operator=(const CRefPtr& rhs)
    {                             // Assignment operator. 
        if (this == &rhs) return *this;
        m_pointer = rhs.m_pointer;
        return *this;
    }

    template <class U>
    CRefPtr& operator=(U* rhs) { return CRefPtr<T>(static_cast<T*>(rhs)); }

    T& operator*() { return *m_pointer; }
    T* operator->() const { return m_pointer; }
    operator T* () const { return m_pointer; }
    operator bool() const { return m_pointer != 0; }
};
