/*     Copyright 2015-2017 Egor Yusov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

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