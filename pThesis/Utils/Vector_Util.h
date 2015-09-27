#pragma once

#include <stdint.h>

#ifdef DXVECTOR 
	#include <DirectXMath.h>
	using namespace DirectX;
#endif

namespace VECTOR
{
	/* Vector2 template 
		#x, y : template */
	template<typename T>
	struct VEC2
	{
		/* Members */
	public:
		T x, y;
		
		/* Methods */
	public:
		/* Default constructor */
		VEC2(T _x = 0, T _y = 0)
			: x(_x), y(_y)
		{ 	}

		/* Copy constructor */
		VEC2(const VEC2& _v)
			: x(_v.x), y(_v.y)
		{	}

		/* Conversion constructor */
		template<typename U>
		VEC2(const VEC2<U>& _v)
			: x(_v.x), y(_v.y)
		{	}

		/* Operators */
	public:
		/* Add */
		VEC2 operator	+ (const VEC2& _v)	{ return VEC2(_v.x + x, _v.y + y); }
		VEC2& operator	+=(const VEC2& _v)	{ x += _v.x; y += _v.y; return *this; }

		/* Subtract */
		VEC2 operator	- (const VEC2& _v)	{ return VEC2(x - _v.x, y - _v.y); }
		VEC2& operator	-=(const VEC2& _v)	{ x -= _v.x; y -= _v.y; return *this; }

		/* Scalar multiply */
		VEC2 operator	* (const T& _r)		{ return VEC2(x * _r, y * _r); }
		VEC2& operator	*=(const T& _r)		{ x *= _r; y *= _r; return *this; }

		/* Scalar divide */
		VEC2 operator	/ (const T& _r)		{ T d = T(1) / _r;  return VEC2(x * d, y * d); }
		VEC2& operator	/=(const T& _r)		{ T d = T(1) / _r; x *= d; y *= d; return *this; }

		/* Equals */
		VEC2& operator	= (const VEC2& _v)	{ x = _v.x; y = _v.y; return *this; }

		/* Illegal scalar operations */
		VEC2 operator +	(const T& _r) = delete;
		VEC2 operator -	(const T& _r) = delete;
		VEC2 operator +=(const T& _r) = delete;
		VEC2 operator -=(const T& _r) = delete;

		bool operator < (const VEC2& _v) const { return TVectorLength(FVEC2(x,y)) < TVectorLength(_v); }
		bool operator > (const VEC2& _v) const { return TVectorLength(FVEC2(x,y)) > TVectorLength(_v); }

		/* DirectX conversions */
#ifdef DXVECTOR
		operator XMVECTOR() { return XMLoadFloat4((XMFLOAT2*)this); }
		operator XMFLOAT2() { return XMFLOAT2(x, y); }
#endif
	};

	/* Vector3 template
	#x, y, z : template */
	template<typename T>
	struct VEC3
	{
		/* Members */
	public:
		T x, y, z;

		/* Methods */
	public:
		/* Default constructor */
		VEC3(T _x = 0, T _y = 0, T _z = 0)
			:x(_x), y(_y), z(_z)
		{	}

		/* Copy constructor */
		VEC3(const VEC3& _v)
				: x(_v.x), y(_v.y), z(_v.z)
			{	}

		/* Conversion constructor */
		template<typename U>
		VEC3(const VEC3<U>& _v)
			: x(_v.x), y(_v.y), z(_v.z)
		{	}

		/* Operators */
	public:
		/* Add */
		VEC3 operator	+ (const VEC3& _v)	{ return VEC3(_v.x + x, _v.y + y, _v.z + z); }
		VEC3& operator	+=(const VEC3& _v)	{ x += _v.x; y += _v.y; z += _v.z return *this; }

		/* Subtract */
		VEC3 operator	- (const VEC3& _v)	{ return VEC3(x - _v.x, y - _v.y, z - _v.z); }
		VEC3& operator	-=(const VEC3& _v)	{ x -= _v.x; y -= _v.y; z -= _v.z; return *this; }

		/* Scalar multiply */
		VEC3 operator	* (const T& _r)		{ return VEC3(x * _r, y * _r, z * _r); }
		VEC3& operator	*=(const T& _r)		{ x *= _r; y *= _r; z *= _r; return *this; }
		
		/* Scalar divide */
		VEC3 operator	/ (const T& _r)		{ double d = 1 / (double)(_r);  return VEC3(x * d, y * d, z * d); }
		VEC3& operator	/=(const T& _r)		{ double d = 1 / (double)(_r); x *= d; y *= d; z *= d; return *this; }

		/* Equals */
		VEC3& operator	= (const VEC3& _v)	{ x = _v.x; y = _v.y; z = _v.z; return *this; }

		/* illegal scalar operations */
		VEC3 operator +	(const T& _r) = delete;
		VEC3 operator -	(const T& _r) = delete;
		VEC3 operator +=(const T& _r) = delete;
		VEC3 operator -=(const T& _r) = delete;

		/* DirectX conversions */
#ifdef DXVECTOR
		VEC3& operator = (const XMVECTOR& _v) { x = _v.m128_f32[0]; y = _v.m128_f32[1]; z = _v.m128_f32[2]; return *this; }
		operator XMFLOAT3() { return XMFLOAT3(x, y, z); }
#endif
	};

	/* Vector4 template
	#x, y, z, w : template */
	template<typename T>
	struct VEC4
	{
		/* Members */
	public:
		T x, y, z, w;

		/* Methods */
	public:
		/* Default constructor */
		VEC4(T _x = 0, T _y = 0, T _z = 0, T _w = 0)
			:x(_x), y(_y), z(_z), w(_w)
		{	}

		/* Copy constructor */
		VEC4(const VEC4& _v)
			: x(_v.x), y(_v.y), z(_v.z), w(_v.w)
			{	}

		/* Conversion constructor */
		template<typename U>
		VEC4(const VEC4<U>& _v)
			: x(_v.x), y(_v.y), z(_v.z), w(_v.w)
		{	}

		/* Operators */
	public:
		/* Add */
		VEC4 operator	+ (const VEC4& _v)	{ return VEC4(_v.x + x, _v.y + y, _v.z + z, _v.w + w); }
		VEC4& operator	+=(const VEC4& _v)	{ x += _v.x; y += _v.y; z += _v.z; w += _v.w; return *this; }

		/* Subtract */
		VEC4 operator	- (const VEC4& _v)	{ return VEC4(x - _v.x, y - _v.y, z - _v.z, w - _v.w); }
		VEC4& operator	-=(const VEC4& _v)	{ x -= _v.x; y -= _v.y; z -= _v.z; w - _v.w; return *this; }

		/* Scalar multiply */
		VEC4 operator	* (const T& _r)		{ return VEC4(x * _r, y * _r, z * _r, w * _r); }
		VEC4& operator	*=(const T& _r)		{ x *= _r; y *= _r; z *= _r; w *= _r; return *this; }

		/* Scalar divide */
		VEC4 operator	/ (const T& _r)		{ T d = T(1) / _r;  return VEC4(x * d, y * d, z * d, w * d); }
		VEC4& operator	/=(const T& _r)		{ T d = T(1) / _r; x *= d; y *= d; z *= d; w *= d; return *this; }

		/* Equals */
		VEC4& operator	= (const VEC4& _v)	{ x = _v.x; y = _v.y; z = _v.z; w = _v.w; return *this; }

		/* illegal scalar operations */
		VEC4 operator +	(const T& _r) = delete;
		VEC4 operator -	(const T& _r) = delete;
		VEC4 operator +=(const T& _r) = delete;
		VEC4 operator -=(const T& _r) = delete;

		/* DirectX conversions */
#ifdef DXVECTOR
		explicit operator XMVECTOR() { return XMLoadFloat4((XMFLOAT4*)this); }

		operator XMFLOAT4() { return XMFLOAT4(x,y,z,w); }

#endif
	};

	/* Matrix4 template
		#x, y : template */
	template<typename T>
	struct MATRIX4X4
	{
		/* Members */
	public:
		T m[4][4];

		/* Methods */
	public:
		MATRIX4X4()
		{
			for (uint32_t i = 0; i < 4; ++i)
			{
				for (uint32_t j = 0; j < 4; ++j)
					m[i][j] = T(0);
			}
		}

		MATRIX4X4(const MATRIX4X4& _mat)
		{
			for (uint32_t i = 0; i < 4; ++i)
				for (uint32_t j = 0; j < 4; ++j)
					m[i][j]  = _mat.m[i][j];
		}

		/* Operators */
	public:
		/* Add */
		MATRIX4X4 operator+(const MATRIX4X4& _mat)
		{
			MATRIX4X4 mat;
			for (uint32_t i = 0; i < 4; ++i)
				for (uint32_t j = 0; j < 4; ++j)
					mat.m[i][j] = m[i][j] + _mat.m[i][j];
				return mat;
		}

		MATRIX4X4& operator+=(const MATRIX4X4& _mat)
		{
			for (uint32_t i = 0; i < 4; ++i)
				for (uint32_t j = 0; j < 4; ++j)
					m[i][j] = m[i][j] + _mat.m[i][j];
			return *this;
		}

		/* Subtract */
		MATRIX4X4 operator-(const MATRIX4X4& _mat)
		{
			MATRIX4X4 mat;
			for (uint32_t i = 0; i < 4; ++i)
				for (uint32_t j = 0; j < 4; ++j)
					mat.m[i][j] = m[i][j] - _mat.m[i][j];
			return mat;
		}

		MATRIX4X4& operator-=(const MATRIX4X4& _mat)
		{
			for (uint32_t i = 0; i < 4; ++i)
			for (uint32_t j = 0; j < 4; ++j)
				m[i][j] -= _mat.m[i][j];
			return *this;
		}

		/* Matrix multiply */
		MATRIX4X4 operator*(const MATRIX4X4& _mat)
		{
			MATRIX4X4 mat;
			for (uint32_t i = 0; i < 4; ++i)
				for (uint32_t j = 0; j < 4; ++j)
					for (uint32_t k = 0; k < 4; ++k)
						mat.m[i][j] += m[i][k] * _mat.m[k][j];
			return mat;
		}

		MATRIX4X4& operator*=(const MATRIX4X4& _mat)
		{
			MATRIX4X4 res = (*this) * _mat;
			(*this) = res;
			return *this;
		}

		/* Vector multiply */
		VEC4<T> operator*(const VEC4<T>& _v)
		{
			VEC4<T> v;
			for (uint32_t i = 0; i < 4; ++i)
			{
				v.x = m[i][0] * _v.x;
				v.y = m[i][1] * _v.y;
				v.z = m[i][2] * _v.z;
				v.w = m[i][3] * _v.w;
			}			
			return v;
		}

		/* Equals */
		MATRIX4X4& operator=(const MATRIX4X4& _mat)
		{
			if (&_mat == this)
				return *this;
			for (uint32_t i = 0; i < 4; ++i)
				for (uint32_t j = 0; j < 4; ++j)
					m[i][j] = _mat.m[i][j];
			return *this;
		}

#ifdef DXVECTOR
		operator XMMATRIX() { return XMLoadFloat4x4((XMFLOAT4X4*)&m); }
		operator XMFLOAT4X4() { return XMFLOAT4X4(*m); }

		MATRIX4X4& operator=(const XMMATRIX& _mat) 
		{ 
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					m[i][j] = _mat.r[i].m128_f32[j];
			
			return *this; 
		}
	
		//VEC3& operator = (const XMVECTOR& _v) { x = _v.m128_f32[0]; y = _v.m128_f32[1]; z = _v.m128_f32[2]; return *this; }
#endif
	};

	/************************
	--- Matrix Operations ---
	*************************/

	/* Get Diagonal VEC4<T> */
	template<typename T>
	inline VEC4<T> GetDiagonal(const MATRIX4X4<T>& _mat)
	{
		return VEC4<T>(_mat[0][0], _mat[1][1], _mat[2][2], _mat[3][3]);
	}

	/* Set identity matrix */
	template<typename T>
	inline void TMatrixIdentity(MATRIX4X4<T>& _mat)
	{
		/* make sure all elements are zerod out */
		_mat = MATRIX4X4<T>();

		for (uint32_t i = 0; i < 4; ++i)
			_mat.m[i][i] = (T)(1);
	}

	/* Create identity matrix */
	template<typename T>
	inline MATRIX4X4<T> TMatrixIdentity(void)
	{
		MATRIX4X4<T> I;
		for (uint32_t i = 0; i < 4; ++i)
			I.m[i][i] = (T)(1);
		return I;
	}

	/* Transpose matrix */
	template<typename T>
	inline MATRIX4X4<T> TMatrixTranspose(const MATRIX4X4<T>& _mat)
	{
		MATRIX4X4<T> mat;
		for (uint32_t i = 0; i < 4; ++i)
			for (uint32_t j = 0; j < 4; ++j)
				mat.m[i][j] = _mat.m[j][i];
		return mat;
	}

	/* Calculate cofactor elements */
	template<typename T>
	inline void SetMinorElements(const MATRIX4X4<T>& _mat, MATRIX4X4<T>& _co, const uint32_t _row, const uint32_t _col)
	{
		int nCol = 0;
		int nRow = 0;

		for (uint32_t i = 0; i < 4; ++i)
		{
			if (i != _row)
			{
				nCol = 0;
				for (uint32_t j = 0; j < 4; ++j)
				{
					if (j != _col)
					{
						_co.m[nRow][nCol] = _mat.m[i][j];
					}
				}
				nRow++;
			}
		}
	}

	/* Matrix Determinant */
	template<typename T>
	inline T TDeterminant(const MATRIX4X4<T>& _mat)
	{
		T det = 0;
		MATRIX4X4<T> cofactor;
		for (uint32_t i = 0; i < 4; ++i)
		{
			SetMinorElements(_mat, cofactor, 0, 1);
			det += (i % 2 == 1 ? -1 : 1) * _mat.m[0][i] * TDeterminant(cofactor);
		}
		return det;
	}

	/* Inverse Matrix */
	template<typename T>
	inline MATRIX4X4<T> TMatrixInverse(const MATRIX4X4<T>& _mat)
	{
		MATRIX4X4<T> inv, cofactor;
		T det = (T)(1) / TDeterminant(_mat);

		for (uint32_t j = 0; j < 4; ++j)
		{
			for (uint32_t i = 0; i < 4; ++i)
			{
				SetMinorElements(_mat, cofactor, j, i);
				inv.m[i][j] = det * TDeterminant(cofactor);
				if ((i + j) % 2 == 1)
					inv.m[i][j] = -inv.m[i][j];
			}
		}
		return inv;
	}

	/************************
	--- Vector Operations ---
	*************************/
	
	/* Transform VEC3<T> */
	template<typename T>
	inline VEC3<T> TVectorTransform(const VEC3<T>& _v, const MATRIX4X4<T>& _mat)
	{
		T r[3];
		for (uint32_t i = 0; i < 3; ++i) {
			r[i] = _v.x * _mat.m[0][i] + _v.y * _mat.m[1][i] + _v.z * _mat.m[2][i];
		}

		T d = (T)(1) / _mat.m[3][3];
		r[0] *= d;
		r[1] *= d;
		r[2] *= d;
		
		return VEC3<T>(r[0], r[1], r[2]);
	}

	template<typename T>
	inline float TVectorLength(const VEC2<T>& _v)
	{
		return std::sqrtf(_v.x * _v.x + _v.y * _v.y);
	}

	template<typename T>
	inline float TVectorLength(const VEC3<T>& _v)
	{
		return std::sqrtf(_v.x * _v.x + _v.y * _v.y + _v.z * _v.z);
	}

	template<typename T>
	inline float TVectorLength(const VEC4<T>& _v)
	{
		return std::sqrtf(_v.x * _v.x + _v.y * _v.y + _v.z * _v.z + _v.w * _v.w);
	}

	/* Cross VEC2<T> */
	template<typename T>
	inline VEC2<T> Cross(const VEC2<T>& _v1, const VEC2<T>& _v2)
	{
		return VEC2<T>(_v1.y * _v2.x, _v1.x * _v2.y);
	}

	/* Cross (const VEC3<T>&, const VEC3<T>&) 
		#_v1	: A
		#_v2	: B
		return	: VEC3(AyBz-AzBy, AzBx-AxBz, AxBy-AyBx)
	*/
	template<typename T>
	inline VEC3<T> Cross(const VEC3<T>&_v1, const VEC3<T>& _v2)
	{
		return VEC3<T>(	_v1.y * _v2.z - _v1.z * _v2.y, 
						_v1.z * _v2.x - _v1.x * _v2.z, 
						_v1.x * _v2.y - _v1.y * _v2.x);
	}

	/* Dot VEC2<T> */
	template<typename T>
	inline T Dot(const VEC2<T>& _v1, const VEC2<T>& _v2)
	{
		return (T)(_v1.x * _v2.x + _v1.y * _v2.y);
	}

	/* Dot VEC3<T> */
	template<typename T>
	inline T Dot(const VEC3<T>& _v1, const VEC3<T>& _v2)
	{
		return (T)(_v1.x * _v2.x + _v1.y * _v2.y + _v1.z * _v2.z);
	}

	/* Dot VEC4<T> */
	template<typename T>
	inline T Dot(const VEC4<T>& _v1, const VEC4<T>& _v2)
	{
		return (T)(_v1.x * _v2.x + _v1.y * _v2.y + _v1.z * _v2.z + _v1.w * _v2.w);
	}

	/* PlaneDotCoord VEC3<T>, VEC4<T> 
		#_v		: VEC3{x,y,z}
		#_plane	: VEC4(A,B,C,D}
		return	: Ax + By + Cz + D*1	*/
	template<typename T>
	inline T PlaneDotCoord(const VEC3<T>& _v, const VEC4<T>& _plane)
	{
		return (T)(_v.x * _plane.x + _v.y * _plane.y + _v.z * _plane.z + _plane.w * 1);
	}

	/* Normalize VEC2<T> */
	template<typename T>
	inline VEC2<T> Normalize(const VEC2<T>& _v)
	{
		T d = (const T)(1) / std::sqrt(_v.x * _v.x + _v.y * _v.y);
		return VEC2<T>(_v.x * d, _v.y * d);
	}

	/* Normalize VEC3<T> */
	template<typename T>
	inline VEC3<T> Normalize(const VEC3<T>& _v)
	{
		T d = (const T)(1) / (T)std::sqrt(_v.x * _v.x + _v.y * _v.y + _v.z * _v.z);
		return VEC3<T>(_v.x * d, _v.y * d, _v.z * d);
	}

	/* Normalize VEC4<T> */
	template<typename T>
	inline VEC4<T> Normalize(const VEC4<T>& _v)
	{
		T d = (const T)(1) / std::sqrt(_v.x * _v.x + _v.y * _v.y + _v.z * _v.z + _v.w * _v.w);
		return VEC4<T>(_v.x * d, _v.y * d, _v.z * d, _v.w * d);
	}
};

