
#pragma once

#include <assert.h>
#include "types.h"

template <typename T>
class Array
{
public:
    T* data;
    int32 size;
    int32 capacity;

    Array<T>(int initialSize = 2)
    {
        data = new T[initialSize];
        size = 0;
        capacity = initialSize;
    }

    ~Array<T>()
    {
        delete[] data;
    }

    void Reserve(int newSize)
    {
        if (newSize > capacity)
        {
            T* old = data;
            data = new T[newSize];
            memcpy(data, old, sizeof(T) * size);
            delete[] old;
            capacity = newSize;
        }
    }

    // Does not free the memory, just invalidates the members
    void Clear()
    {
        size = 0;
    }

    void Reset(int newSize = 2)
    {
        delete[] data;
        data = new T[newSize];
    }

    void Reallocate()
    {
        T* old = data;
        capacity *= 2;
        data = new T[capacity];
        memcpy(data, old, sizeof(T) * size);
        delete[] old;
    }

    T& Last()
    {
        T& result = data[size - 1];
        return result;
    }

    void Append(T d)
    {
        if (size >= capacity)
        {
            Reallocate();
        }
        data[size++] = d;
    }

    void Remove(int index)
    {
        assert(index < size);
        memcpy(&data[index], &data[size - 1], sizeof(T));
        size -= 1;
    }

    void RemoveBack()
    {
        if(size > 0)
            size -= 1;
    }

    T& Get(int index)
    {
        T& result = data[index];
        return result;
    }

    T& operator[](int index)
    {
        T& result = data[index];
        return result;
    }
};
