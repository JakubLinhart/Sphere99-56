    #pragma once
template <class T>
class CReferenceCounted
{
private:
    unsigned m_ReferenceCount;
    T* m_Pointer;

    // Copy construction and assignment will not be allowed:

    template < class U>
    CReferenceCounted<T>& operator=(const CReferenceCounted<U>& rhs);
    template < class U>
    CReferenceCounted(const CReferenceCounted<U>& rhs);
public:
    CReferenceCounted(T* p) :
        m_ReferenceCount(1),
        m_Pointer(p) {}
    ~CReferenceCounted() { delete m_Pointer; }
    T& operator*() { return *m_Pointer; }
    T* get() { return m_Pointer; }
    void reference() { m_ReferenceCount++; }
    void dereference() { m_ReferenceCount--; }
    int  norefs() { return !m_ReferenceCount; }
    int  refcount() { return m_ReferenceCount; }
};

#define PTR_CAST(a,b) static_cast<a*>(b)
#define REF_CAST(a,b) static_cast<CRefPtr<a>>(b)

#define CNewPtr CRefPtr
template <class T>
class CRefPtr
{
private:
    CReferenceCounted<T>* m_Object;
public:
    CRefPtr() : m_Object(0) {}
    CRefPtr(T* p) :                // Constructor
        m_Object(new CReferenceCounted<T>(p)) {}
    ~CRefPtr() {           // Destructor
        if (m_Object) {
            m_Object->dereference();
            if (m_Object->norefs()) delete m_Object;

        }
    }

    template < class U>
    CRefPtr(const CRefPtr<U>& rhs) {  // Copy constructor.
        m_Object = rhs.m_Object;
        if (m_Object) {
            m_Object->reference();

        }
    }
    CRefPtr& operator=(const CRefPtr& rhs)
    {                             // Assignment operator. 
        if (this == &rhs) return *this;
        if (m_Object) {
            m_Object->dereference();
            if (m_Object->norefs()) delete m_Object;
            m_Object = 0;
        }
        if (rhs.m_Object) {
            m_Object = rhs.m_Object;
            m_Object->reference();
        }
        return *this;
    }
    int refcount() const {
        if (m_Object) {
            return m_Object->refcount();
        }
        else {
            return -1;
        }
    }
    // Operations:
    //
    template <class U>
    int operator==(const CRefPtr<U>& rhs) {
        return (m_Object == rhs.m_Object);
    }
    T& operator*() { return m_Object->operator*(); }
    T* operator->() { return m_Object->get(); }
};

class CRefObjDef
{
};

#define CResourceObjPtr CResourceObj*
class CResourceObj
{
private:
    HASH_INDEX m_dwHashIndex;

public:
    CResourceObj(HASH_INDEX dwHashIndex)
    {
        m_dwHashIndex = dwHashIndex;
    }

	int GetRefCount();
    HASH_INDEX GetUIDIndex() const { return m_dwHashIndex; }
};
