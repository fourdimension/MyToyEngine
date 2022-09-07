#pragma once
#include "pch.h"
#include <deque>

class RingBuffer
{
public:
	typedef size_t OffsetType;
	struct FrameTailAttribs
	{
		FrameTailAttribs(uint64_t fenceValue, OffsetType offset, OffsetType sz) :
			FenceValue(fenceValue),
			Offset(offset),
			Size(sz)
		{}

		uint64_t FenceValue;
		OffsetType Offset;
		OffsetType Size;
	};

	static const OffsetType InvalidOffset = static_cast<OffsetType>(-1); // largest integer
	
	RingBuffer(OffsetType maxSize) noexcept:
		m_CompletedFrameTails(1, {0,0,0}),
		m_Head(0), 
		m_Tail(0),
		m_MaxSize(0),
		m_UsedSize(0),
		m_CurFrameSize(0)
	{}

	RingBuffer(RingBuffer&& rhs) noexcept :
		m_CompletedFrameTails(std::move(rhs.m_CompletedFrameTails)),
		m_Head(rhs.m_Head),
		m_Tail(rhs.m_Tail),
		m_MaxSize(rhs.m_MaxSize),
		m_UsedSize(rhs.m_UsedSize),
		m_CurFrameSize(rhs.m_CurFrameSize)
	{
		rhs.m_MaxSize = 0;
		rhs.m_Head = 0;
		rhs.m_Tail = 0;
		rhs.m_MaxSize = 0;
		rhs.m_UsedSize = 0;
		rhs.m_CurFrameSize = 0;
	}

	RingBuffer& operator = (RingBuffer&& rhs) noexcept
	{
		m_CompletedFrameTails = std::move(rhs.m_CompletedFrameTails);
		m_Head = rhs.m_Head;
		m_Tail = rhs.m_Tail;
		m_MaxSize = rhs.m_MaxSize;
		m_UsedSize = rhs.m_UsedSize;
		m_CurFrameSize = rhs.m_CurFrameSize;

		rhs.m_MaxSize = 0;
		rhs.m_Head = 0;
		rhs.m_Tail = 0;
		rhs.m_UsedSize = 0;
		rhs.m_CurFrameSize = 0;

		return *this;
	}


	RingBuffer(const RingBuffer&) = delete;
	RingBuffer& operator = (const RingBuffer&) = delete;

	~RingBuffer()
	{
		ASSERT(m_UsedSize == 0, "Memory in ring buffer is not fully released!");
	}

	OffsetType Allocate(OffsetType size);

	void FinishCurFrame(uint64_t FenceValue);
	void ReleaseCompletedFrames(uint64_t CompletedFenceValue);

	OffsetType GetMaxSize() const { return m_MaxSize; }
	bool IsFull() const { return m_UsedSize == m_MaxSize; }
	bool IsEmpty() const { return m_UsedSize == 0; }
	OffsetType GetUsedSize() const { return m_UsedSize; }

private:
	std::deque<FrameTailAttribs> m_CompletedFrameTails;
	OffsetType m_Head;
	OffsetType m_Tail;
	OffsetType m_MaxSize;
	OffsetType m_UsedSize;
	OffsetType m_CurFrameSize;
	
};