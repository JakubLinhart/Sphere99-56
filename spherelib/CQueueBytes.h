#pragma once

class CGQueueBytes
{
	// Create an arbitrary queue of data.
	// NOTE: I know this is not a real queue yet, but i'm working on it.
private:
	CMemLenBlock m_Mem;  ///< Data buffer.
	size_t m_iDataQty;  ///< Item count of the data queue.

public:
	// Peak into/read from the Queue's data.
	int GetDataQty() const
	{
		// How much data is avail?
		return m_iDataQty;
	}
};