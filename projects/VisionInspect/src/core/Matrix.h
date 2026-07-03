#pragma once
#include <vector>
#include <cmath>
#include <cassert>

template<typename T>
class Matrix {
public:
    int rows, cols;
    std::vector<T> data;

    Matrix() : rows(0), cols(0) {}
    Matrix(int r, int c) : rows(r), cols(c), data(r * c, T(0)) {}
    Matrix(int r, int c, const T* init_data) : rows(r), cols(c), data(init_data, init_data + r * c) {}

    T& operator()(int r, int c) { return data[r * cols + c]; }
    const T& operator()(int r, int c) const { return data[r * cols + c]; }

    Matrix<T> operator*(const Matrix<T>& other) const {
        assert(cols == other.rows);
        Matrix<T> result(rows, other.cols);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < other.cols; ++j) {
                T sum = 0;
                for (int k = 0; k < cols; ++k) {
                    sum += (*this)(i, k) * other(k, j);
                }
                result(i, j) = sum;
            }
        }
        return result;
    }

    Matrix<T> transpose() const {
        Matrix<T> result(cols, rows);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                result(j, i) = (*this)(i, j);
        return result;
    }

    Matrix<T> inverse() const {
        assert(rows == 3 && cols == 3);
        Matrix<T> result(3, 3);
        T det = (*this)(0,0) * ((*this)(1,1) * (*this)(2,2) - (*this)(1,2) * (*this)(2,1))
              - (*this)(0,1) * ((*this)(1,0) * (*this)(2,2) - (*this)(1,2) * (*this)(2,0))
              + (*this)(0,2) * ((*this)(1,0) * (*this)(2,1) - (*this)(1,1) * (*this)(2,0));
        if (std::abs(det) < 1e-12) return *this;
        T invDet = 1.0 / det;
        result(0,0) = ((*this)(1,1) * (*this)(2,2) - (*this)(1,2) * (*this)(2,1)) * invDet;
        result(0,1) = ((*this)(0,2) * (*this)(2,1) - (*this)(0,1) * (*this)(2,2)) * invDet;
        result(0,2) = ((*this)(0,1) * (*this)(1,2) - (*this)(0,2) * (*this)(1,1)) * invDet;
        result(1,0) = ((*this)(1,2) * (*this)(2,0) - (*this)(1,0) * (*this)(2,2)) * invDet;
        result(1,1) = ((*this)(0,0) * (*this)(2,2) - (*this)(0,2) * (*this)(2,0)) * invDet;
        result(1,2) = ((*this)(0,2) * (*this)(1,0) - (*this)(0,0) * (*this)(1,2)) * invDet;
        result(2,0) = ((*this)(1,0) * (*this)(2,1) - (*this)(1,1) * (*this)(2,0)) * invDet;
        result(2,1) = ((*this)(0,1) * (*this)(2,0) - (*this)(0,0) * (*this)(2,1)) * invDet;
        result(2,2) = ((*this)(0,0) * (*this)(1,1) - (*this)(0,1) * (*this)(1,0)) * invDet;
        return result;
    }

    static Matrix<T> Identity(int n) {
        Matrix<T> I(n, n);
        for (int i = 0; i < n; ++i) I(i, i) = 1;
        return I;
    }
};
