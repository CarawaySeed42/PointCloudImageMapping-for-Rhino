#pragma once
template <typename T>
class Array2D
{
    T** data_ptr;
    size_t m_rows;
    size_t m_cols;

    inline T** create2DArray(size_t nrows, size_t ncols, const T& val = T())
    {
        T** ptr = nullptr;
        T* pool = nullptr;
        try
        {
            ptr = new T*[nrows];  // allocate pointers (can throw here)
            pool = new T[(uint64_t)nrows*ncols]{ val };  // allocate pool (can throw here)

            // now point the row pointers to the appropriate positions in
            // the memory pool
            for (size_t i = 0; i < nrows; ++i, pool += ncols)
                ptr[i] = pool;

            // Done.
            return ptr;
        }
        catch (std::bad_alloc& ex)
        {
            delete[] ptr; // either this is nullptr or it was allocated
            throw ex;  // memory allocation error
        }
    }

public:
    typedef T value_type;
    T** data() {
        return data_ptr;
    }

    size_t get_rows() const {
        return m_rows;
    }

    size_t get_cols() const {
        return m_cols;
    }

    Array2D() : data_ptr(nullptr), m_rows(0), m_cols(0) {}

    Array2D(size_t rows, size_t cols, const T& val = T()) : data_ptr(nullptr), m_rows(0), m_cols(0)
    {
        if (rows == 0)
            return;
        if (cols == 0)
            return;

        data_ptr = create2DArray(rows, cols, val);

        if (data_ptr) {
            m_rows = rows;
            m_cols = cols;
        }
        
    }

    ~Array2D()
    {
        if (data_ptr)
        {
            delete[] data_ptr[0];  // remove the pool
            delete[] data_ptr;     // remove the pointers
            data_ptr = nullptr;
        }
    }

    void Destroy() noexcept {
        this->~Array2D();
    }

    Array2D(const Array2D& rhs) : m_rows(rhs.m_rows), m_cols(rhs.m_cols)
    {
        data_ptr = create2DArray(m_rows, m_cols);
        std::copy(&rhs.data_ptr[0][0], &rhs.data_ptr[m_rows-1][m_cols], &data_ptr[0][0]);
    }

    // untested, taken from the internet
    Array2D(Array2D&& rhs) noexcept
    {
        data_ptr = rhs.data_ptr;
        m_rows = rhs.m_rows;
        m_cols = rhs.m_cols;
        rhs.data_ptr = nullptr;
    }

    Array2D& operator=(Array2D&& rhs) noexcept
    {
        if (&rhs != this)
        {
            swap(rhs, *this);
            rhs.~Array2D();
        }
        return *this;
    }

    void swap(Array2D& left, Array2D& right)
    {
        std::swap(left.data_ptr, right.data_ptr);
        std::swap(left.m_cols, right.m_cols);
        std::swap(left.m_rows, right.m_rows);
    }

    Array2D& operator = (const Array2D& rhs)
    {
        if (&rhs != this)
        {
            Array2D temp(rhs);
            swap(*this, temp);
        }
        return *this;
    }

    inline T* operator[](size_t row)
    {
        return data_ptr[row];
    }

    inline const T* operator[](size_t row) const
    {
        return data_ptr[row];
    }

    bool isValid() { return nullptr != data_ptr; }

    explicit operator bool() const { return nullptr != data_ptr; }

    void create(size_t rows, size_t cols, const T& val = T())
    {
        if (data_ptr) {
            this->Destroy();
        }
        *this = Array2D(rows, cols, val);
    }
};
