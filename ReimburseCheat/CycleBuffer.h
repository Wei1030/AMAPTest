#pragma once

class CycleBuffer
{
public:
	CycleBuffer(unsigned int size);
	~CycleBuffer();

	void ClearBuffer();//clear the buffer to init state, just modify the indicator
	bool IsBufferFull();//to judge if the buffer is full
	bool IsBufferEmpty();//to judge if the buffer is empty
	unsigned int BufferLength();//calculate buffer's length
	unsigned int ReadBuffer(unsigned char *data, unsigned int len);//read data from the buffer, return the actual data length
	unsigned int WriteBuffer(unsigned char *data, unsigned int len);//write data into the buffer, return the actual written-in data length
	
	//get data address to write external.
	//note: after this method called success , don't call WriteBuffer() untill  ReleaseBuffer() called
	unsigned char* GetBufferToWrite(unsigned int len);
	void	ReleaseBuffer() { m_nWritePos = m_nWantTo; }


private:

	inline unsigned int NextPos(unsigned int cur) //get the next pos of write or read
	{
		return (cur + 1) % m_nLogicSize;
	}

private:
	unsigned char* m_pBuffer;
	unsigned int m_nLogicSize;
	unsigned int m_nReadPos;
	unsigned int m_nWritePos;
	unsigned int m_nWantTo;
};

