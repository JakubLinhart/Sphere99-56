#pragma once

//*************************************************

class CMemDynamic
{
	// This item will always be dynamically allocated with new/delete!
	// Never stack or data seg based.

#if defined(_DEBUG) || defined(DEBUG)

#define DECLARE_MEM_DYNAMIC virtual const void * GetTopPtr() const { return this; } // Get the top level ptr.
#define COBJBASE_SIGNATURE  0xDEADBEEF      // used just to make sure this is valid.
private:
	DWORD m_dwSignature;

protected:
	virtual const void* GetTopPtr() const = 0;     // Get the top level class ptr.
public:
	bool IsValidDynamic() const
	{
		if (m_dwSignature != COBJBASE_SIGNATURE)
		{
			return(false);
		}
#ifdef GRAY_SVR
		// return( DEBUG_ValidateAlloc( GetTopPtr()) ? true : false );
		return(true);
#else
		return(true);
#endif      // GRAY_SVR
	}
	CMemDynamic()
	{
		// NOTE: virtuals don't work in constructors or destructors !
		m_dwSignature = COBJBASE_SIGNATURE;
		// ASSERT( IsValidDynamic());
	}
	virtual ~CMemDynamic()
	{
		ASSERT(IsValidDynamic());
		m_dwSignature = 0;
	}

#else       // _DEBUG

#define DECLARE_MEM_DYNAMIC
public:
	bool IsValidDynamic() const
	{
		return(true);
	}
	virtual ~CMemDynamic()  // always virtual so we can always use dynamic_cast correctly.
	{
	}

#endif      // _DEBUG
};

//*************************************************
// CGObList

class CGObListRec : public CMemDynamic      // generic list record base class. 
{
	// This item belongs to JUST ONE LIST
	friend class CGObList;
private:
	CGObList* m_pParent;              // link me back to my parent object.
	CGObListRec* m_pNext;
	CGObListRec* m_pPrev;
public:
	CGObList* GetParent() const { return(m_pParent); }
	CGObListRec* GetNext() const { return(m_pNext); }
	CGObListRec* GetPrev() const { return(m_pPrev); }
public:
	CGObListRec()
	{
		m_pParent = NULL;       // not linked yet.
		m_pNext = NULL;
		m_pPrev = NULL;
	}
	void RemoveSelf();      // remove myself from my parent list.
	virtual ~CGObListRec()
	{
		RemoveSelf();
	}
};

class CGObList      // generic list of objects based on CGObListRec.
{
	friend class CGObListRec;
private:
	CGObListRec* m_pHead;
	CGObListRec* m_pTail;  // Do we really care about tail ? (as it applies to lists anyhow)
	int m_iCount;
private:
	void RemoveAtSpecial(CGObListRec* pObRec)
	{
		// only called by pObRec->RemoveSelf()
		OnRemoveOb(pObRec);   // call any approriate virtuals.
	}
protected:
	// Override this to get called when an item is removed from this list.
	// Never called directly. call pObRec->RemoveSelf()
	virtual void OnRemoveOb(CGObListRec* pObRec); // Override this = called when removed from list.
public:
	CGObListRec* GetAt(int index) const;
	// pPrev = NULL = first
	virtual void InsertAfter(CGObListRec* pNewRec, CGObListRec* pPrev = NULL);
	void InsertBefore(CGObListRec* pNewRec, CGObListRec* pNext)
	{
		// pPrev = NULL = last
		InsertAfter(pNewRec, (pNext) ? (pNext->GetPrev()) : GetTail());
	}
	void InsertHead(CGObListRec* pNewRec)
	{
		InsertAfter(pNewRec, NULL);
	}
	void InsertTail(CGObListRec* pNewRec)
	{
		InsertAfter(pNewRec, GetTail());
	}
	void DeleteAll();
	void Empty() { DeleteAll(); }
	CGObListRec* GetHead(void) const { return(m_pHead); }
	CGObListRec* GetTail(void) const { return(m_pTail); }
	int GetCount() const { return(m_iCount); }
	bool IsEmpty() const
	{
		return(!GetCount());
	}
	CGObList()
	{
		m_pHead = NULL;
		m_pTail = NULL;
		m_iCount = 0;
	}
	virtual ~CGObList()
	{
		DeleteAll();
	}
};

inline void CGObListRec::RemoveSelf()       // remove myself from my parent list.
{
	// Remove myself from my parent list (if i have one)
	if (GetParent() == NULL)
		return;
	m_pParent->RemoveAtSpecial(this);
	ASSERT(GetParent() == NULL);
}

template<class TYPE>
class CGObListType : public CGObList
{
};

///////////////////////////////////////////////////////////
// CGTypedArray<class TYPE, class ARG_TYPE>

/**
* @brief Typed Array.
*
* NOTE: This will not call true constructors or destructors !
* TODO: Really needed two types in template?
*/
template<class TYPE, class ARG_TYPE>
class CGTypedArray
{
private:
	TYPE* m_pData;	///< Pointer to allocated mem.
	size_t m_nCount;	///< count of elements stored.
	size_t m_nRealCount;	///< Size of allocated mem.

public:
	static const char* m_sClassName;
	/**
	* @brief Initializes array.
	*
	* Sets m_pData to NULL and counters to zero.
	*/
	CGTypedArray();
	virtual ~CGTypedArray();
	const CGTypedArray<TYPE, ARG_TYPE>& operator=(const CGTypedArray<TYPE, ARG_TYPE>& array);
private:
	/**
	* @brief No copy on construction allowed.
	*/
	CGTypedArray<TYPE, ARG_TYPE>(const CGTypedArray<TYPE, ARG_TYPE>& copy);
public:
	/**
	* @brief Get the internal data pointer.
	*
	* This is dangerous to use of course.
	* @return the internal data pointer.
	*/
	TYPE* GetBasePtr() const;
	/**
	* @brief Get the element count in array.
	* @return get the element count in array.
	*/
	size_t GetCount() const;
	/**
	* @brief Get the total element that fits in allocated mem.
	* @return get the total element that fits in allocated mem.
	*/
	int GetSize() const	// same thing just for compatibility
	{
		return m_nCount;
	}
	size_t GetRealCount() const;
	/**
	* @brief Check if index is valid for this array.
	* @param i index to check.
	* @return true if index is valid, false otherwise.
	*/
	bool IsValidIndex(size_t i) const;
	/**
	* @brief Realloc the internal data into a new size.
	* @param nNewCount new size of the mem.
	*/
	void SetCount(size_t nNewCount);
	/**
	* @brief Remove all elements from the array and free mem.
	*/
	void RemoveAll();
	/**
	* @brief Remove all elements from the array and free mem.
	*
	* TODO: Really needed?
	* @see RemoveAll()
	*/
	void Empty();
	/**
	* @brief Update element nth to a new value.
	* @param nIndex index of element to update.
	* @param newElement new value.
	*/
	void SetAt(size_t nIndex, ARG_TYPE newElement);
	/**
	* @brief Update element nth to a new value.
	*
	* If size of array is lesser to nIndex, increment array size.
	* @param nIndex index of element to update.
	* @param newElement new value.
	*/
	void SetAtGrow(size_t nIndex, ARG_TYPE newElement);
	/**
	* @brief Insert a element in nth position.
	* @param nIndex position to insert the element.
	* @param newElement element to insert.
	*/
	void InsertAt(size_t nIndex, ARG_TYPE newElement);
	/**
	* @brief Insert a new element to the end of the array.
	* @param newElement element to insert.
	* @return the element count of the array.
	*/
	size_t Add(ARG_TYPE newElement);
	/**
	* @brief Removes the nth element and move the next elements one position left.
	* @param nIndex position of the element to remove.
	*/
	void RemoveAt(size_t nIndex);
	/**
	* @brief get the nth element.
	*
	* Also checks if index is valid.
	* @param nIndex position of the element.
	* @return Element in nIndex position.
	*/
	TYPE GetAt(size_t nIndex) const;
	/**
	* @brief get the nth element.
	*
	* Also checks if index is valid.
	* @see GetAt()
	* @param nIndex position of the element.
	* @return Element in nIndex position.
	*/
	TYPE operator[](size_t nIndex) const;
	/**
	* @brief get a reference to the nth element.
	*
	* Also checks if index is valid.
	* @param nIndex position of the element.
	* @return Element in nIndex position.
	*/
	TYPE& ElementAt(size_t nIndex);
	TYPE& ConstElementAt(size_t nIndex) const;
	/**
	* @brief get a reference to the nth element.
	*
	* Also checks if index is valid.
	* @see ElementAt()
	* @param nIndex position of the element.
	* @return Element in nIndex position.
	*/
	TYPE& operator[](size_t nIndex);
	/**
	* @brief get a reference to the nth element.
	*
	* Also checks if index is valid.
	* @param nIndex position of the element.
	* @return Element in nIndex position.
	*/
	const TYPE& ElementAt(size_t nIndex) const;
	/**
	* @brief TODOC
	* @param pElements TODOC
	* @param nCount TODOC
	*/
	virtual void ConstructElements(TYPE* pElements, size_t nCount);
	/**
	* @brief TODOC
	* @param pElements TODOC
	* @param nCount TODOC
	*/
	virtual void DestructElements(TYPE* pElements, size_t nCount);
	/**
	* @brief Copy an CGTypedArray into this.
	* @param pArray array to copy.
	*/
	void Copy(const CGTypedArray<TYPE, ARG_TYPE>* pArray);
public:
	inline size_t BadIndex() const { return (std::numeric_limits<size_t>::max)(); }
};

/**
* @brief An Array of pointers.
*/
template<class TYPE>
class CGRefArray : public CGTypedArray<TYPE, TYPE>
{
protected:
	/**
	* @brief TODOC
	* @param pElements TODOC
	* @param nCount TODOC
	*/
	virtual void DestructElements(TYPE* pElements, size_t nCount);
public:
	static const char* m_sClassName;
	/**
	* @brief get the position of a data in the array.
	* @param pData data to look for.
	* @return position of the data if data is in the array, BadIndex otherwise.
	*/
	size_t FindPtr(TYPE pData) const;
	/**
	* @brief check if data is in this array.
	* @param pData data to find in the array.
	* @return true if pData is in the array, BadIndex() otherwise.
	*/
	bool ContainsPtr(TYPE pData) const;
	/**
	* @brief if data is in array, rmove it.
	* @param pData data to remove from the array.
	*/
	bool RemovePtr(TYPE pData);
	/**
	* @brief Check if an index is between 0 and element count.
	* @param i index to check.
	* @return true if index is valid, false otherwise.
	*/
	bool IsValidIndex(size_t i) const;
public:
	CGRefArray() { };
	virtual ~CGRefArray() { };
private:
	/**
	* @brief No copy on construction allowed.
	*/
	CGRefArray<TYPE>(const CGRefArray<TYPE>& copy);
	/**
	* @brief No copy allowed.
	*/
	CGRefArray<TYPE>& operator=(const CGRefArray<TYPE>& other);
};

//*************************************************
// CGObArray

template<class TYPE>
class CGObArray : public CGRefArray<TYPE>
{
	// The point of this type is that the array now OWNS the element.
	// It will get deleted when the array is deleted.
protected:
	virtual void DestructElements(TYPE* pElements, int nCount)
	{
		// delete the objects that we own.
		for (int i = 0; i < nCount; i++)
		{
			TYPE pDestruct = pElements;
			if (pDestruct == NULL)
				continue;
			pElements = NULL;       // in case the destructor looks for itself.
			delete pDestruct;
		}
		CGRefArray<TYPE>::DestructElements(pElements, nCount);
	}
public:
	bool DeleteOb(TYPE pData)
	{
		return(RemovePtr(pData));
	}
	void DeleteAt(int nIndex)
	{
		RemoveAt(nIndex);
	}
	TYPE UnLinkIndex(int index)
	{
		// Remove the object from the list so it will not get destroyed.
		TYPE data = GetAt(index);
		ElementAt(index) = NULL;
		RemoveAt(index);
		return(data);
	}
	~CGObArray()
	{
		// Make sure the virtuals get called.
		SetCount(0);
	}
};


//*************************************************
// CGSortedArray = A sorted array of objects.

template<class TYPE, class ARG_TYPE, class KEY_TYPE>
struct CGSortedArray : public CGTypedArray<TYPE, ARG_TYPE>
{
	int FindKeyNear(KEY_TYPE key, int& iCompareRes) const
	{
		// Do a binary search for the key.
		// RETURN: index
		//  iCompareRes =
		//              0 = match with index.
		//              -1 = key should be less than index.
		//              +1 = key should be greater than index
		//

		int iHigh = GetCount() - 1;
		if (iHigh < 0)
		{
			iCompareRes = -1;
			return(0);
		}

		int iLow = 0;
		int i;
		while (iLow <= iHigh)
		{
			i = (iHigh + iLow) / 2;
			iCompareRes = CompareKey(key, GetAt(i));
			if (iCompareRes == 0)
				break;
			if (iCompareRes > 0)
			{
				iLow = i + 1;
			}
			else
			{
				iHigh = i - 1;
			}
		}
		return(i);
	}
	int FindKey(KEY_TYPE key) const
	{
		// Find exact key
		int iCompareRes;
		int index = FindKeyNear(key, iCompareRes);
		if (iCompareRes)
			return(-1);
		return(index);
	}
	int AddPresorted(int index, int iCompareRes, TYPE pNew)
	{
		if (iCompareRes > 0)
		{
			index++;
		}
		InsertAt(index, pNew);
		return(index);
	}
	int AddSortKey(TYPE pNew, KEY_TYPE key)
	{
		// Insertion sort.
		int iCompareRes;
		int index = FindKeyNear(key, iCompareRes);
		if (!iCompareRes)
		{
			// duplicate should not happen ?!?
			// DestructElements is called automatically for previous.
			SetAt(index, pNew);
			return(-1);
		}
		return AddPresorted(index, iCompareRes, pNew);
	}

	virtual int CompareKey(KEY_TYPE, TYPE) const = 0;
	void DeleteKey(KEY_TYPE key)
	{
		DeleteAt(FindKey(key));
	}
#ifdef _DEBUG
	bool TestSort() const;
#endif
};
