/*
 * Copyright (c) 2015 Dmytro Shestakov
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "utils.h"
#include "select_size.h"

template<int SIZE, class DATA_T = unsigned char>
class CircularBuffer
{
public:
	typedef typename Mcucpp::SelectSizeForLength<SIZE>::type INDEX_T;

private:
	STATIC_ASSERT((SIZE&(SIZE - 1)) == 0);//SIZE must be a power of 2
	DATA_T _data[SIZE];
	volatile INDEX_T _readCount;
	volatile INDEX_T _writeCount;
	static const INDEX_T _mask = SIZE - 1;
public:

	bool Write(DATA_T value)
	{
		if(IsFull())
			return 0;
		_data[_writeCount++ & _mask] = value;
		return true;
	}

	bool Read(DATA_T &value)
	{
		if(IsEmpty())
			return 0;
		value = _data[_readCount++ & _mask];
		return true;
	}

	DATA_T First()const
	{
		return operator[](0);
	}

	DATA_T Last()const
	{
		return operator[](Count());
	}

	DATA_T& operator[] (INDEX_T i)
	{
		if(IsEmpty() || i > Count())
			return DATA_T();
		return _data[(_readCount + i) & _mask];
	}

	const DATA_T operator[] (INDEX_T i) const
	{
		if(IsEmpty() || i > Count())
			return DATA_T();
		return _data[(_readCount + i) & _mask];
	}

	bool IsEmpty()const
	{
		INDEX_T temp = _readCount;
		return _writeCount == temp;
	}

	bool IsFull()const
	{
		INDEX_T temp = _readCount;
		return ((_writeCount - temp) & (INDEX_T)~(_mask)) != 0;
	}

	INDEX_T Count()const
	{
		return (_writeCount - _readCount) & _mask;
	}

	void Clear()
	{
		_readCount = 0;
		_writeCount = 0;
	}

	unsigned Size()
	{
		return SIZE;
	}
};
