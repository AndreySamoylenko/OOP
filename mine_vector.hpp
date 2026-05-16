#pragma once

#include <cstddef> // Для std::size_t
#include <stdexcept>

#include <iostream>

template <typename T>
class Iterator
{
public:
    virtual ~Iterator() = default;

    virtual T &operator*() const = 0;
    virtual Iterator<T> &operator++() = 0;

    virtual bool operator!=(const Iterator<T> &other) const = 0;
    virtual bool operator==(const Iterator<T> &other) const = 0;
};

template <typename T>
class iterator : public Iterator<T>
{
private:
    T *ptr;

public:
    iterator(T *p = nullptr) : ptr(p) {}

    T &operator*() const override
    {
        return *ptr;
    }

    Iterator<T> &operator++() override
    {
        ptr++;
        return *this;
    }

    Iterator<T> &operator--()
    {
        ptr--;
        return *this;
    }

    bool operator!=(const Iterator<T> &other) const override
    {
        const iterator &o = static_cast<const iterator &>(other);
        return ptr != o.ptr;
    }

    bool operator==(const Iterator<T> &other) const override
    {
        const iterator &o = static_cast<const iterator &>(other);
        return ptr == o.ptr;
    }

    Iterator<T> &operator=(const Iterator<T> &other)
    {
        const iterator &o = static_cast<const iterator &>(other);
        ptr = other.ptr;
        return *this;
    }
};

template <typename T>
class filter : public Iterator<T>
{
private:
    T *ptr;
    T *end;
    T value;

    void find_next()
    {
        while (ptr != end && *ptr != value)
            ++ptr;
    }

public:
    filter(T *b, T *e, T v) : ptr(b), end(e), value(v)
    {
        find_next();
    }

    T &operator*() const override
    {
        return *ptr;
    }

    Iterator<T> &operator++() override
    {
        ++ptr;
        find_next();
        return *this;
    }

    bool operator!=(const Iterator<T> &other) const override
    {
        const filter &o = static_cast<const filter &>(other);
        return ptr != o.ptr;
    }

    bool operator==(const Iterator<T> &other) const override
    {
        const filter &o = static_cast<const filter &>(other);
        return ptr == o.ptr;
    }
};

template <class T>
class Vector
{
private:
    T *data = nullptr;
    std::size_t v_size = 0;
    std::size_t v_capacity = 0;

    void reserve(size_t new_cap);

public:
    Vector() : data(nullptr), v_size(0), v_capacity(0) {}
    Vector(const Vector &other);
    Vector(size_t n, const T &val);

    Vector &operator=(const Vector &other);

    ~Vector()
    {
        delete[] data;
    }

    void insert(const T &value, const size_t index);
    bool remove(const T &value);
    void push_back(const T &value);
    void append_range(const T &value);
    void pop_back();

    T &operator[](size_t index)       { return data[index]; }
    const T &operator[](size_t index) const { return data[index]; }
    T &at(std::size_t index)
    {
        if (index >= v_size)
            throw std::out_of_range("mine_vector::at: index out of range");
        return data[index];
    }
    T &at(std::size_t index) const
    {
        if (index >= v_size)
            throw std::out_of_range("mine_vector::at: index out of range");
        return data[index];
    }

    bool empty() { return v_size == 0; }

    size_t size() const { return v_size; }
    size_t capacity() const { return v_capacity; }

    void shrink_to_fit();
    void clear() { v_size = 0; }

    iterator<T> begin() const { return iterator<T>(data); }
    iterator<T> end()   const { return iterator<T>(data + v_size); }

    iterator<T> rbegin() const { return iterator<T>(data + v_size - 1); }
    iterator<T> rend()   const { return iterator<T>(data - 1); }

    filter<T> f_begin(T value) const { return filter<T>(data, data + v_size, value); }
    filter<T> f_end(T value)   const { return filter<T>(data + v_size, data + v_size, value); }

    bool check_presence(T value)
    {
        return f_begin(value) != f_end(value);
    }

    // Move semantics для Vector<FSItem*>
    Vector(Vector &&other) noexcept
        : data(other.data), v_size(other.v_size), v_capacity(other.v_capacity)
    {
        other.data = nullptr;
        other.v_size = 0;
        other.v_capacity = 0;
    }

    Vector &operator=(Vector &&other) noexcept
    {
        if (this == &other) return *this;
        delete[] data;
        data = other.data;
        v_size = other.v_size;
        v_capacity = other.v_capacity;
        other.data = nullptr;
        other.v_size = 0;
        other.v_capacity = 0;
        return *this;
    }
};

template <typename T>
Vector<T>::Vector(const Vector &other)
{
    v_size = other.v_size;
    v_capacity = other.v_size;
    if (v_capacity > 0) {
        data = new T[v_capacity];
        for (size_t i = 0; i < v_size; ++i)
            data[i] = other.data[i];
    } else {
        data = nullptr;
    }
}

template <typename T>
Vector<T>::Vector(size_t n, const T &val)
{
    v_size = n;
    v_capacity = n;
    if (v_capacity > 0) {
        data = new T[v_capacity];
        for (size_t i = 0; i < v_size; ++i)
            data[i] = val;
    } else {
        data = nullptr;
    }
}

template <typename T>
void Vector<T>::reserve(std::size_t new_cap)
{
    if (new_cap <= v_capacity) return;
    T *new_data = new T[new_cap];
    for (size_t i = 0; i < v_size; ++i)
        new_data[i] = data[i];
    delete[] data;
    data = new_data;
    v_capacity = new_cap;
}

template <typename T>
Vector<T> &Vector<T>::operator=(const Vector &other)
{
    if (this == &other) return *this;
    delete[] data;
    v_size = other.v_size;
    v_capacity = other.v_size;
    if (v_capacity > 0) {
        data = new T[v_capacity];
        for (size_t i = 0; i < v_size; ++i)
            data[i] = other.data[i];
    } else {
        data = nullptr;
    }
    return *this;
}

template <typename T>
void Vector<T>::insert(const T &value, const size_t index)
{
    if (v_size == v_capacity)
        reserve(v_capacity == 0 ? 1 : v_capacity * 2);
    if (index >= v_size) {
        data[v_size++] = value;
        return;
    }
    for (size_t i = v_size; i > index; i--)
        data[i] = data[i - 1];
    data[index] = value;
    v_size++;
}

template <typename T>
bool Vector<T>::remove(const T &value)
{
    if (v_size == 0) return false;
    size_t pos = v_size; // sentinel = not found
    for (size_t i = 0; i < v_size; i++) {
        if (data[i] == value) { pos = i; break; }
    }
    if (pos == v_size) return false;
    for (size_t i = pos; i + 1 < v_size; i++)
        data[i] = data[i + 1];
    v_size--;
    return true;
}

template <typename T>
void Vector<T>::push_back(const T &value)
{
    if (v_size == v_capacity)
        reserve(v_capacity == 0 ? 1 : v_capacity * 2);
    data[v_size++] = value;
}

template <typename T>
void Vector<T>::append_range(const T &value)
{
    for (size_t i = v_size; i < v_capacity; i++)
        data[i] = value;
}

template <typename T>
void Vector<T>::pop_back()
{
    if (v_size == 0)
        throw std::out_of_range("Popping out of empty vector");
    v_size--;
}

template <typename T>
void Vector<T>::shrink_to_fit()
{
    if (v_size == v_capacity) return;
    T *new_data = new T[v_size];
    for (size_t i = 0; i < v_size; ++i)
        new_data[i] = data[i];
    delete[] data;
    data = new_data;
    v_capacity = v_size;
}