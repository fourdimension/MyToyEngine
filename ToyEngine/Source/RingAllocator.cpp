#include "RingAllocator.h"

typedef size_t OffsetType;

OffsetType RingBuffer::Allocate(OffsetType size)
{
	if (IsFull()) return InvalidOffset;

	if (m_Tail >= m_Head)
	{
		//                     Head             Tail     MaxSize
		//                     |                |        |
		//  [                  xxxxxxxxxxxxxxxxx         ]
		//                                        
		
		if (m_Tail + size <= m_MaxSize) 
		{
			OffsetType offset = m_Tail;
			m_Tail += size;
			m_UsedSize += size;
			m_CurFrameSize += size;
			return offset;
		}
		else if (size <= m_Head)
		{
			// Allocate from the beginning of the buffer
			OffsetType addSize = (m_MaxSize - m_Tail + size);
			m_UsedSize += addSize;
			m_CurFrameSize += addSize;
			m_Tail = size;
			return 0;
		}
	} else if(m_Tail + size <= m_Head)
	{
		//       Tail          Head            
		//       |             |            
		//  [xxxx              xxxxxxxxxxxxxxxxxxxxxxxxxx]

		OffsetType Offset = m_Tail;
		m_Tail += size;
		m_UsedSize += size;
		m_CurFrameSize += size;
		return Offset;
	}

	return InvalidOffset;
}

void RingBuffer::FinishCurFrame(uint64_t FenceValue)
{
	m_CompletedFrameTails.emplace_back(FenceValue, m_Tail, m_CurFrameSize);
	m_CurFrameSize = 0;
}

void RingBuffer::ReleaseCompletedFrames(uint64_t CompletedFenceValue)
{

	// We can release all tails whose associated fence value is less 
	// than or equal to CompletedFenceValue
	while (!m_CompletedFrameTails.empty() &&
		m_CompletedFrameTails.front().FenceValue <= CompletedFenceValue)
	{
		const auto& OldestFrameTail = m_CompletedFrameTails.front();
		ASSERT(OldestFrameTail.Size <= m_UsedSize);
		m_UsedSize -= OldestFrameTail.Size;
		m_Head = OldestFrameTail.Offset;
		m_CompletedFrameTails.pop_front();
	}
}