#pragma once

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
