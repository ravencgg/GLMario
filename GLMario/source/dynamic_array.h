#pragma once


template<class T>
class DynamicArray
{
public:
	
	DynamicArray(uint32 start_size = 16)
	: capacity(start_size),
		write_pos(0)
	{
		data = new T[start_size] {};
	}

	~DynamicArray()
	{
		delete[] data;
	}

	T operator[](uint32 loc)
	{
		return this->get(loc);
	}

	void add(T value)
	{
		if(write_pos >= capacity)
		{
			reallocate();
		}

		data[write_pos++] = value;
	}

	T get(uint32 position)
	{
		assert(position < write_pos);

		return data[position];
	}

	void remove(uint32 position)
	{
		assert(position < write_pos);	

		//TODO(chris): Replace this with a memcopy call!
		for(uint32 i = position; i < write_pos; ++i)
		{
			data[i] = data[i + 1];
		}

		--write_pos;
	}
	
	void clear()
	{
		//NOTE(chris): not actually clearing, but items are not accessible
		write_pos = 0;
	}

	uint32 size()
	{
		return write_pos;
	}

private:

	void reallocate()
	{
		T* temp = data;
		uint32 old_size = capacity;
		capacity *= 2;
		data = new T[capacity] {}; 

		//TODO(chris): Replace this with a memcopy call!
		for(uint32 i = 0; i < old_size; ++i)
		{
			data[i] = temp[i];
		}

		delete[] temp;
	}

	T* data;	
	uint32 capacity;
	uint32 write_pos;
};
