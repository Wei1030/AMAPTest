#include "CycleBuffer.h"


CycleBuffer::CycleBuffer(unsigned int size)
	: m_nLogicSize(size+1)
	, m_nWritePos(0)
	, m_nReadPos(0)
{
	m_pBuffer = new unsigned char[m_nLogicSize];
}


CycleBuffer::~CycleBuffer()
{
	if (m_pBuffer)
	{
		delete m_pBuffer;
	}
}

void CycleBuffer::ClearBuffer()
{
	m_nWritePos = 0;
	m_nReadPos = 0;
}

bool CycleBuffer::IsBufferFull()
{
	if (NextPos(m_nWritePos) == m_nReadPos)
		return true;
	return false;
}

bool CycleBuffer::IsBufferEmpty()
{
	if (m_nWritePos == m_nReadPos)
		return true;
	return false;

}

unsigned int CycleBuffer::BufferLength()
{
	return (m_nWritePos - m_nReadPos + m_nLogicSize) % m_nLogicSize;
}

unsigned int CycleBuffer::ReadBuffer(unsigned char *data, unsigned int len)
{
	unsigned int cur = m_nReadPos;
	unsigned int i = 0;
	while (cur != m_nWritePos && i < len)
	{
		data[i++] = m_pBuffer[cur];
		cur = NextPos(cur);
	}
	m_nReadPos = cur;
	return i;

}

unsigned int CycleBuffer::WriteBuffer(unsigned char *data, unsigned int len)
{
	unsigned int i = 0;
	unsigned int cur = m_nWritePos;
	while (NextPos(cur) != m_nReadPos && i < len)
	{
		m_pBuffer[cur] = data[i++];
		cur = NextPos(cur);
	}
	
	m_nWritePos = cur;
	return i;	
}

unsigned char* CycleBuffer::GetBufferToWrite(unsigned int len)
{	
	unsigned int i = 0;
	unsigned int cur = m_nWritePos;
	unsigned char* retbuf = m_pBuffer + cur;

	while (NextPos(cur) != m_nReadPos && i < len)
	{
		m_pBuffer[cur] = 0;
		i++;
		cur = NextPos(cur);
	}

	if (i < len)
	{
		return 0;
	}

	m_nWantTo = cur;

	return retbuf;
}
