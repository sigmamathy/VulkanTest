#pragma once

#include "Dependencies.hpp"

// Mathematical vectors
template<class Ty, unsigned Dim>
class MathVector
{
public:

	// Value uninitialized.
	constexpr MathVector() noexcept = default;

	// Copy constructor.
	template<class Ty2, unsigned Dim2> requires (Dim2 >= Dim)
	constexpr MathVector(MathVector<Ty2, Dim2> const& vec) noexcept
	{
		for (unsigned i = 0; i < Dim; i++)
			m_elems[i] = static_cast<Ty>(vec[i]);
	}

	// Value fill initialization.
	explicit constexpr MathVector(auto value) noexcept
	{
		for (unsigned i = 0; i < Dim; i++)
			m_elems[i] = static_cast<Ty>(value);
	}

	// Construct with Dim number of arguments.
	template<class... Tys>
		requires (sizeof...(Tys) == Dim && (std::is_convertible_v<Tys, Ty> && ...))
	constexpr MathVector(Tys... args) noexcept
		: m_elems{ static_cast<Ty>(args)... }
	{
	}

	// Access elements.
	constexpr Ty& operator[](unsigned i)
	{
		return m_elems[i];
	}

	// Access elements const.
	constexpr Ty const& operator[](unsigned i) const
	{
		return m_elems[i];
	}

	// Calculate the magnitude.
	auto Norm() const
	{
		using common = std::common_type_t<Ty, float>;
		common sum = 0;
		for (unsigned i = 0; i < Dim; i++)
			sum += static_cast<common>(m_elems[i] * m_elems[i]);
		return std::sqrt(sum);
	}

	template<class Ty2>
	MathVector& operator+=(MathVector<Ty2, Dim> const& vec)
	{
		for (unsigned i = 0; i < Dim; i++)
			m_elems[i] += static_cast<Ty>(vec[i]);
		return *this;
	}

	template<class Ty2>
	MathVector& operator*=(MathVector<Ty2, Dim> const& vec)
	{
		for (unsigned i = 0; i < Dim; i++)
			m_elems[i] -= static_cast<Ty>(vec[i]);
		return *this;
	}

	MathVector& operator*=(auto scale)
	{
		for (unsigned i = 0; i < Dim; i++)
			m_elems[i] *= scale;
		return *this;
	}

private:
	// array implementation
	std::array<Ty, Dim> m_elems;
};

// 1D vector variation (x)
template<class Ty>
class MathVector<Ty, 1>
{
public:

	Ty x;

	constexpr MathVector() noexcept = default;

	constexpr MathVector(auto x) noexcept
	{
		this->x = static_cast<Ty>(x);
	}

	template<class Ty2, unsigned Dim2> requires (Dim2 >= 1)
	constexpr MathVector(MathVector<Ty2, Dim2> const& vec) noexcept
	{
		x = static_cast<Ty>(vec[0]);
	}

	constexpr Ty& operator[](unsigned i)
	{
		return x;
	}

	constexpr Ty const& operator[](unsigned i) const
	{
		return x;
	}

	auto Norm() const
	{
		return x;
	}

	template<class Ty2>
	MathVector& operator+=(MathVector<Ty2, 1> const& vec)
	{
		x += static_cast<Ty>(vec.x);
		return *this;
	}

	template<class Ty2>
	MathVector& operator-=(MathVector<Ty2, 1> const& vec)
	{
		x -= static_cast<Ty>(vec.x);
		return *this;
	}

	MathVector& operator*=(auto scale)
	{
		x *= static_cast<Ty>(scale);
		return *this;
	}
};

// 2D vector variation (x, y)
template<class Ty>
class MathVector<Ty, 2>
{
public:

	Ty x, y;

	constexpr MathVector() noexcept = default;

	explicit constexpr MathVector(auto value) noexcept
	{
		x = y = static_cast<Ty>(value);
	}

	constexpr MathVector(auto x, auto y) noexcept
	{
		this->x = static_cast<Ty>(x);
		this->y = static_cast<Ty>(y);
	}

	template<class Ty2, unsigned Dim2> requires (Dim2 >= 2)
	constexpr MathVector(MathVector<Ty2, Dim2> const& vec) noexcept
	{
		x = static_cast<Ty>(vec[0]);
		y = static_cast<Ty>(vec[1]);
	}

	constexpr Ty& operator[](unsigned i)
	{
		return i ? y : x;
	}

	constexpr Ty const& operator[](unsigned i) const
	{
		return i ? y : x;
	}

	auto Norm() const
	{
		return std::sqrt(x * x + y * y);
	}

	template<class Ty2>
	MathVector& operator+=(MathVector<Ty2, 2> const& vec)
	{
		x += static_cast<Ty>(vec.x);
		y += static_cast<Ty>(vec.y);
		return *this;
	}

	template<class Ty2>
	MathVector& operator-=(MathVector<Ty2, 2> const& vec)
	{
		x -= static_cast<Ty>(vec.x);
		y -= static_cast<Ty>(vec.y);
		return *this;
	}

	MathVector& operator*=(auto scale)
	{
		x *= static_cast<Ty>(scale);
		y *= static_cast<Ty>(scale);
		return *this;
	}
};

// 3D vector variation (x, y, z)
template<class Ty>
class MathVector<Ty, 3>
{
public:

	Ty x, y, z;

	constexpr MathVector() noexcept = default;

	explicit constexpr MathVector(auto value) noexcept
	{
		x = y = z = static_cast<Ty>(value);
	}

	constexpr MathVector(auto x, auto y, auto z) noexcept
	{
		this->x = static_cast<Ty>(x);
		this->y = static_cast<Ty>(y);
		this->z = static_cast<Ty>(z);
	}

	template<class Ty2, unsigned Dim2> requires (Dim2 >= 3)
	constexpr MathVector(MathVector<Ty2, Dim2> const& vec) noexcept
	{
		x = static_cast<Ty>(vec[0]);
		y = static_cast<Ty>(vec[1]);
		z = static_cast<Ty>(vec[2]);
	}

	constexpr Ty& operator[](unsigned i)
	{
		return i == 2 ? z : (i ? y : x);
	}

	constexpr Ty const& operator[](unsigned i) const
	{
		return i == 2 ? z : (i ? y : x);
	}

	auto Norm() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	template<class Ty2>
	MathVector& operator+=(MathVector<Ty2, 3> const& vec)
	{
		x += static_cast<Ty>(vec.x);
		y += static_cast<Ty>(vec.y);
		z += static_cast<Ty>(vec.z);
		return *this;
	}

	template<class Ty2>
	MathVector& operator-=(MathVector<Ty2, 3> const& vec)
	{
		x -= static_cast<Ty>(vec.x);
		y -= static_cast<Ty>(vec.y);
		z -= static_cast<Ty>(vec.z);
		return *this;
	}

	MathVector& operator*=(auto scale)
	{
		x *= static_cast<Ty>(scale);
		y *= static_cast<Ty>(scale);
		z *= static_cast<Ty>(scale);
		return *this;
	}
};

// 4D vector variation (x, y, z, w)
template<class Ty>
class MathVector<Ty, 4>
{
public:

	Ty x, y, z, w;

	constexpr MathVector() noexcept = default;

	explicit constexpr MathVector(auto value) noexcept
	{
		x = y = z = w = static_cast<Ty>(value);
	}

	constexpr MathVector(auto x, auto y, auto z, auto w) noexcept
	{
		this->x = static_cast<Ty>(x);
		this->y = static_cast<Ty>(y);
		this->z = static_cast<Ty>(z);
		this->w = static_cast<Ty>(w);
	}

	template<class Ty2, unsigned Dim2> requires (Dim2 >= 4)
	constexpr MathVector(MathVector<Ty2, Dim2> const& vec) noexcept
	{
		x = static_cast<Ty>(vec[0]);
		y = static_cast<Ty>(vec[1]);
		z = static_cast<Ty>(vec[2]);
		w = static_cast<Ty>(vec[3]);
	}

	constexpr Ty& operator[](unsigned i)
	{
		return i == 3 ? w : (i == 2 ? z : (i ? y : x));
	}

	constexpr Ty const& operator[](unsigned i) const
	{
		return i == 3 ? w : (i == 2 ? z : (i ? y : x));
	}

	auto Norm() const
	{
		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	template<class Ty2>
	MathVector& operator+=(MathVector<Ty2, 4> const& vec)
	{
		x += static_cast<Ty>(vec.x);
		y += static_cast<Ty>(vec.y);
		z += static_cast<Ty>(vec.z);
		w += static_cast<Ty>(vec.w);
		return *this;
	}

	template<class Ty2>
	MathVector& operator-=(MathVector<Ty2, 4> const& vec)
	{
		x -= static_cast<Ty>(vec.x);
		y -= static_cast<Ty>(vec.y);
		z -= static_cast<Ty>(vec.z);
		w -= static_cast<Ty>(vec.w);
		return *this;
	}

	MathVector& operator*=(auto scale)
	{
		x *= static_cast<Ty>(scale);
		y *= static_cast<Ty>(scale);
		z *= static_cast<Ty>(scale);
		w *= static_cast<Ty>(scale);
		return *this;
	}
};

// -------------------- Typedefs ---------------------- //

using Ivec2 = MathVector<int, 2>;
using Ivec3 = MathVector<int, 3>;
using Ivec4 = MathVector<int, 4>;

using Uvec2 = MathVector<unsigned, 2>;
using Uvec3 = MathVector<unsigned, 3>;
using Uvec4 = MathVector<unsigned, 4>;

using Fvec2 = MathVector<float, 2>;
using Fvec3 = MathVector<float, 3>;
using Fvec4 = MathVector<float, 4>;

// ------------------------------------ Functions ------------------------------------ //

// Returns true if 2 vector are equal
template<class Ty1, class Ty2, unsigned Dim>
constexpr bool operator==(MathVector<Ty1, Dim> const& vec1, MathVector<Ty2, Dim> const& vec2)
{
	for (unsigned i = 0; i < Dim; i++)
		if (vec1[i] != vec2[i])
			return false;
	return true;
}

// Addition operation bewteen 2 vectors
template<class Ty1, class Ty2, unsigned Dim>
constexpr auto operator+(MathVector<Ty1, Dim> const& vec1, MathVector<Ty2, Dim> const& vec2)
{
	MathVector<decltype(vec1[0] + vec2[0]), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec1[i] + vec2[i];
	return res;
}

// Subtraction operation bewteen 2 vectors
template<class Ty1, class Ty2, unsigned Dim>
constexpr auto operator-(MathVector<Ty1, Dim> const& vec1, MathVector<Ty2, Dim> const& vec2)
{
	MathVector<decltype(vec1[0] - vec2[0]), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec1[i] - vec2[i];
	return res;
}

// Negate operation of vector
template<class Ty, unsigned Dim>
constexpr auto operator-(MathVector<Ty, Dim> const& vec)
{
	MathVector<decltype(-vec[0]), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = -vec[i];
	return res;
}

// Scaler multiplication operation of vector
template<class Ty, class ScTy, unsigned Dim> requires std::is_arithmetic_v<ScTy>
constexpr auto operator*(MathVector<Ty, Dim> const& vec, ScTy scale)
{
	MathVector<decltype(vec[0] * scale), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec[i] * scale;
	return res;
}

// Scaler multiplication operation of vector
template<class Ty, class ScTy, unsigned Dim> requires std::is_arithmetic_v<ScTy>
constexpr auto operator*(ScTy scale, MathVector<Ty, Dim> const& vec)
{
	return vec * scale;
}

// Scaler division operation of vector
template<class Ty, class ScTy, unsigned Dim> requires std::is_arithmetic_v<ScTy>
constexpr auto operator/(MathVector<Ty, Dim> const& vec, ScTy scale)
{
	MathVector<decltype(vec[0] / scale), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec[i] / scale;
	return res;
}

// Concatenate two vectors into a larger vector
template<class Ty1, unsigned Dim1, class Ty2, unsigned Dim2>
constexpr auto operator&(MathVector<Ty1, Dim1> const& vec1, MathVector<Ty2, Dim2> const& vec2)
{
	MathVector<std::common_type_t<Ty1, Ty2>, Dim1 + Dim2> res;
	unsigned n = 0;
	for (unsigned i = 0; i < Dim1; i++) res[n++] = vec1[i];
	for (unsigned i = 0; i < Dim2; i++) res[n++] = vec2[i];
	return res;
}

// Concatenate a vector and a number into a larger vector
template<class Ty, unsigned Dim, class ScTy> requires std::is_arithmetic_v<ScTy>
constexpr auto operator&(MathVector<Ty, Dim> const& vec, ScTy num)
{
	MathVector<std::common_type_t<Ty, ScTy>, Dim + 1> res;
	for (unsigned i = 0; i < Dim; i++) res[i] = vec[i];
	res[Dim] = static_cast<Ty>(num);
	return res;
}

// Concatenate a vector and a number into a larger vector
template<class Ty, unsigned Dim, class ScTy> requires std::is_arithmetic_v<ScTy>
constexpr auto operator&(ScTy num, MathVector<Ty, Dim> const& vec)
{
	MathVector<std::common_type_t<Ty, ScTy>, Dim + 1> res;
	res[0] = static_cast<Ty>(num);
	for (unsigned i = 0; i < Dim; i++) res[i + 1] = vec[i];
	return res;
}

// Dot product operation
template<class Ty1, class Ty2, unsigned Dim>
constexpr auto Dot(MathVector<Ty1, Dim> const& vec1, MathVector<Ty2, Dim> const& vec2)
{
	decltype(vec1[0] * vec2[0]) res = 0;
	for (unsigned i = 0; i < Dim; i++)
		res += vec1[i] * vec2[i];
	return res;
}

// Cross product operation
template<class Ty1, class Ty2>
constexpr auto Cross(MathVector<Ty1, 3> const& vec1, MathVector<Ty2, 3> const& vec2)
{
	return MathVector<decltype(vec1[0] * vec2[0]), 3>
	{
		vec1[1] * vec2[2] - vec1[2] * vec2[1],
		vec1[2] * vec2[0] - vec1[0] * vec2[2],
		vec1[0] * vec2[1] - vec1[1] * vec2[0]
	};
}

// Cross product operation on 2D (special case)
template<class Ty1, class Ty2>
constexpr auto Cross2D(MathVector<Ty1, 2> const& vec1, MathVector<Ty2, 2> const& vec2)
{
	return vec1[0] * vec2[1] - vec1[1] * vec2[0];
}

// Hadamard product.
template<class Ty1, class Ty2, unsigned Dim>
constexpr auto Hadamard(MathVector<Ty1, Dim> const& vec1, MathVector<Ty2, Dim> const& vec2)
{
	MathVector<decltype(vec1[0] * vec2[0]), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec1[i] * vec2[i];
	return res;
}

// Compute a normalized vector
template<class Ty, unsigned Dim>
constexpr auto Normalize(MathVector<Ty, Dim> const& vec)
{
	using common_type = std::common_type_t<Ty, float>;
	Ty sum = 0;
	for (unsigned i = 0; i < Dim; i++)
		sum += vec[i] * vec[i];
	auto invsqrt = 1.0f / std::sqrt(static_cast<common_type>(sum));
	MathVector<common_type, Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec[i] * invsqrt;
	return res;
}