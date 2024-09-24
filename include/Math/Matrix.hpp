#pragma once

#include "Dependencies.hpp"
#include "Math/Vector.hpp"

// Mathematical matrix
template<class Ty, unsigned Rw, unsigned Cn>
class MathMatrix
{
public:
	// Value uninitialized construction
	constexpr MathMatrix() noexcept = default;

	// Identity matrix multiplied by value
	explicit constexpr MathMatrix(Ty value) noexcept
	{
		std::fill(m_columns.begin(), m_columns.end(), MathVector<Ty, Rw>(0));
		constexpr unsigned max = Rw > Cn ? Rw : Cn;
		for (unsigned i = 0; i < max; i++)
			m_columns[i][i] = value;
	}

	// Construct with Rw * Cn number of arguments
	template<class... Tys> requires (sizeof...(Tys) == Cn)
	constexpr MathMatrix(MathVector<Tys, Rw> const&... columns) noexcept
		: m_columns { static_cast<MathVector<Ty, Rw>>(columns)... }
	{
	}

	// Copy constructor
	template<class Ty2>
	constexpr MathMatrix(MathMatrix<Ty2, Rw, Cn> const& mat) noexcept
	{
		for (unsigned i = 0; i < Cn; i++)
			m_columns[i] = mat.m_columns[i];
	}

	// Access modifiers
	constexpr MathVector<Ty, Rw>& operator[](unsigned i)
	{
		return m_columns[i];
	}

	// Constant access
	constexpr MathVector<Ty, Rw> const& operator[](unsigned i) const
	{
		return m_columns[i];
	}

	template<class Ty2>
	MathMatrix& operator+=(MathMatrix<Ty2, Rw, Cn> const& mat)
	{
		for (unsigned i = 0; i < Cn; i++)
			m_columns[i] += mat[i];
		return *this;
	}

	template<class Ty2>
	MathMatrix& operator-=(MathMatrix<Ty2, Rw, Cn> const& mat)
	{
		for (unsigned i = 0; i < Cn; i++)
			m_columns[i] -= mat[i];
		return *this;
	}

	MathMatrix& operator*=(auto scale)
	{
		for (unsigned i = 0; i < Cn; i++)
			m_columns[i] *= scale;
		return *this;
	}

private:
	// array of column vectors
	std::array<MathVector<Ty, Rw>, Cn> m_columns;
};

// -------------------- Typedefs ---------------------- //

using Imat2x2 = MathMatrix<int, 2, 2>;
using Imat2x3 = MathMatrix<int, 2, 3>;
using Imat2x4 = MathMatrix<int, 2, 4>;
using Imat3x2 = MathMatrix<int, 3, 2>;
using Imat3x3 = MathMatrix<int, 3, 3>;
using Imat3x4 = MathMatrix<int, 3, 4>;
using Imat4x2 = MathMatrix<int, 4, 2>;
using Imat4x3 = MathMatrix<int, 4, 3>;
using Imat4x4 = MathMatrix<int, 4, 4>;

using Umat2x2 = MathMatrix<unsigned, 2, 2>;
using Umat2x3 = MathMatrix<unsigned, 2, 3>;
using Umat2x4 = MathMatrix<unsigned, 2, 4>;
using Umat3x2 = MathMatrix<unsigned, 3, 2>;
using Umat3x3 = MathMatrix<unsigned, 3, 3>;
using Umat3x4 = MathMatrix<unsigned, 3, 4>;
using Umat4x2 = MathMatrix<unsigned, 4, 2>;
using Umat4x3 = MathMatrix<unsigned, 4, 3>;
using Umat4x4 = MathMatrix<unsigned, 4, 4>;

using Fmat2x2 = MathMatrix<float, 2, 2>;
using Fmat2x3 = MathMatrix<float, 2, 3>;
using Fmat2x4 = MathMatrix<float, 2, 4>;
using Fmat3x2 = MathMatrix<float, 3, 2>;
using Fmat3x3 = MathMatrix<float, 3, 3>;
using Fmat3x4 = MathMatrix<float, 3, 4>;
using Fmat4x2 = MathMatrix<float, 4, 2>;
using Fmat4x3 = MathMatrix<float, 4, 3>;
using Fmat4x4 = MathMatrix<float, 4, 4>;

using Imat2 = Imat2x2;
using Imat3 = Imat3x3;
using Imat4 = Imat4x4;

using Umat2 = Umat2x2;
using Umat3 = Umat3x3;
using Umat4 = Umat4x4;

using Fmat2 = Fmat2x2;
using Fmat3 = Fmat3x3;
using Fmat4 = Fmat4x4;

// ------------------------------------ Functions ------------------------------------ //

// Compare equality of 2 matrices
template<class Ty1, class Ty2, unsigned Rw, unsigned Cn>
constexpr auto operator==(MathMatrix<Ty1, Rw, Cn> const& mat1, MathMatrix<Ty2, Rw, Cn> const& mat2)
{
	for (unsigned i = 0; i < Cn; i++)
		if (mat1[i] != mat2[i])
			return false;
	return true;
}

// Addition operation between 2 matrices
template<class Ty1, class Ty2, unsigned Rw, unsigned Cn>
constexpr auto operator+(MathMatrix<Ty1, Rw, Cn> const& mat1, MathMatrix<Ty2, Rw, Cn> const& mat2)
{
	MathMatrix<decltype(mat1[0][0] + mat2[0][0]), Rw, Cn> res;
	for (unsigned i = 0; i < Cn; i++)
		res[i] = mat1[i] + mat2[i];
	return res;
}

// Subtraction operation between 2 matrices
template<class Ty1, class Ty2, unsigned Rw, unsigned Cn>
constexpr auto operator-(MathMatrix<Ty1, Rw, Cn> const& mat1, MathMatrix<Ty2, Rw, Cn> const& mat2)
{
	MathMatrix<decltype(mat1[0][0] - mat2[0][0]), Rw, Cn> res;
	for (unsigned i = 0; i < Cn; i++)
		res[i] = mat1[i] - mat2[i];
	return res;
}

// Negate operation of matrices
template<class Ty, unsigned Rw, unsigned Cn>
constexpr auto operator-(MathMatrix<Ty, Rw, Cn> const& mat)
{
	MathMatrix<decltype(-mat[0][0]), Rw, Cn> res;
	for (unsigned i = 0; i < Cn; i++)
		res[i] = -mat[i];
	return res;
}

// Scalar mulitplication operation of matrices
template<class Ty1, class ScTy, unsigned Rw, unsigned Cn>
constexpr auto operator*(MathMatrix<Ty1, Rw, Cn> const& mat, ScTy scale)
{
	MathMatrix<decltype(mat[0][0] * scale), Rw, Cn> res;
	for (unsigned i = 0; i < Cn; i++)
		res[i] = mat * scale;
	return res;
}

// Scalar mulitplication operation of matrices
template<class Ty1, class ScTy, unsigned Rw, unsigned Cn>
constexpr auto operator*(ScTy scale, MathMatrix<Ty1, Rw, Cn> const& mat)
{
	return mat * scale;
}

// Scalar division operation of matrices
template<class Ty1, class ScTy, unsigned Rw, unsigned Cn>
constexpr auto operator/(MathMatrix<Ty1, Rw, Cn> const& mat, ScTy scale)
{
	MathMatrix<decltype(mat[0][0] / scale), Rw, Cn> res;
	for (unsigned i = 0; i < Cn; i++)
		res[i] = mat[i] / scale;
	return res;
}

// Matrix multiplication operation
template<class Ty1, class Ty2, unsigned Rw1, unsigned Rw2Cn1, unsigned Cn2>
constexpr auto operator*(MathMatrix<Ty1, Rw1, Rw2Cn1> const& mat1, MathMatrix<Ty2, Rw2Cn1, Cn2> const& mat2)
{
	MathMatrix<decltype(mat1[0][0] * mat2[0][0]), Rw1, Cn2> res(0);
	for (unsigned i = 0; i < Cn2; i++)
		for (unsigned j = 0; j < Rw1; j++)
			for (unsigned k = 0; k < Rw2Cn1; k++)
				res[i][j] += mat1[k][j] * mat2[i][k];
	return res;
}

// Matrix multiplication operation (with vector outcome)
template<class Ty1, class Ty2, unsigned Rw1, unsigned Dim>
constexpr auto operator*(MathMatrix<Ty1, Rw1, Dim> const& mat, MathVector<Ty2, Dim> const& vec)
{
	MathVector<decltype(mat[0][0] * vec[0]), Rw1> res(0);
	for (unsigned j = 0; j < Rw1; j++)
		for (unsigned i = 0; i < Dim; i++)
			res[j] += mat[i][j] * vec[i];
	return res;
}

// Matrix transpose operation
template<class Ty, unsigned Rw, unsigned Cn>
constexpr auto Transpose(MathMatrix<Ty, Rw, Cn> const& mat)
{
	MathMatrix<Ty, Cn, Rw> res;
	for (unsigned j = 0; j < Rw; j++)
		for (unsigned i = 0; i < Cn; i++)
			res[j][i] = mat[i][j];
	return res;
}

// Addition operation between 2 matrices
template<class Ty1, class Ty2, unsigned Rw, unsigned Cn>
constexpr auto Hadamard(MathMatrix<Ty1, Rw, Cn> const& mat1, MathMatrix<Ty2, Rw, Cn> const& mat2)
{
	MathMatrix<decltype(mat1[0][0] * mat2[0][0]), Rw, Cn> res;
	for (unsigned i = 0; i < Cn; i++)
		res[i] = Hadamard(mat1[i], mat2[i]);
	return res;
}