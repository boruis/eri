/*
 *  math_helper.h
 *  eri
 *
 *  Created by exe on 11/29/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_MATH_HELPER_H
#define ERI_MATH_HELPER_H

namespace ERI {
	
	struct Math
	{
		static const float PI;
		static const float TWO_PI;
		static const float HALF_PI;
	};

	struct Vector2
	{
		Vector2() : x(0), y(0) {}
		Vector2(float _x, float _y) : x(_x), y(_y) {}
		
		inline Vector2 operator + (const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
		inline Vector2 operator - (const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
		inline Vector2 operator * (const Vector2& v) const { return Vector2(x * v.x, y * v.y); }
		inline Vector2 operator * (float value) const { return Vector2(x * value, y * value); }
		inline Vector2 operator / (const Vector2& v) const { return Vector2(x / v.x, y / v.y); }
		inline Vector2& operator += (const Vector2& v) { x += v.x; y += v.y; return *this; }
		inline Vector2& operator -= (const Vector2& v) { x -= v.x; y -= v.y; return *this; }
		inline Vector2& operator *= (float value) { x *= value; y *= value; return *this; }
		
		inline float DotProduct(const Vector2& v) const
        {
            return x * v.x + y * v.y;
        }
		
		float Length() const;
		float LengthSquared() const;
		float Normalize();
		
		float x, y;
	};
	
	struct Vector3
	{
		Vector3() : x(0), y(0), z(0) {}
		Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
		
		inline Vector3 operator + (const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
		inline Vector3 operator - (const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
		inline Vector3 operator * (const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
		inline Vector3 operator * (float value) const { return Vector3(x * value, y * value, z * value); }
		inline Vector3 operator / (const Vector3& v) const { return Vector3(x / v.x, y / v.y, z / v.z); }
		inline Vector3& operator += (const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
		inline Vector3& operator -= (const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
		inline Vector3& operator *= (float value) { x *= value; y *= value; z *= value; return *this; }
		
		inline float DotProduct(const Vector3& v) const
        {
            return x * v.x + y * v.y + z * v.z;
        }
		inline Vector3 CrossProduct(const Vector3& v) const
        {
            return Vector3(y * v.z - z * v.y,
						   z * v.x - x * v.z,
						   x * v.y - y * v.x);
        }
		
		float Length() const;
		float LengthSquared() const;
		float Normalize();
		
		float x, y, z;
	};
	
	enum Matrix4RowCol	// opengl use column-major
	{
		_00 = 0,
		_10,
		_20,
		_30,
		
		_01,
		_11,
		_21,
		_31,
		
		_02,
		_12,
		_22,
		_32,
		
		_03,
		_13,
		_23,
		_33
	};
	
	struct Matrix4
	{
		Matrix4() { *this = IDENTITY; }
		Matrix4(float m00, float m01, float m02, float m03,
				float m10, float m11, float m12, float m13,
				float m20, float m21, float m22, float m23,
				float m30, float m31, float m32, float m33)
		{
			m[_00] = m00; m[_01] = m01; m[_02] = m02; m[_03] = m03;
			m[_10] = m10; m[_11] = m11; m[_12] = m12; m[_13] = m13;
			m[_20] = m20; m[_21] = m21; m[_22] = m22; m[_23] = m23;
			m[_30] = m30; m[_31] = m31; m[_32] = m32; m[_33] = m33;
		}
		
		inline Matrix4 operator * (const Matrix4& m2) const
		{
			Matrix4 ret;
			Multiply(ret, *this, m2);
			return ret;
		}
		
		inline Vector3 operator * (const Vector3 &v) const
		{
			Vector3 ret;
			Multiply(ret, *this, v);
			return ret;
		}
		
		float m[16];
		
		static void Multiply(Matrix4& out_m, const Matrix4& m1, const Matrix4& m2);
		static void Multiply(Vector3& out_v, const Matrix4& m, const Vector3& v);
		static void Inverse(Matrix4& out_m, const Matrix4& m);
		
		static void Translate(Matrix4& out_m, const Vector3& translate);
		static void RotateAxis(Matrix4& out_m, float degree, const Vector3& axis);
		static void Scale(Matrix4& out_m, const Vector3& scale);
		
		static const Matrix4 IDENTITY;
	};
	
	struct Quaternion
	{
		Quaternion() : x(0), y(0), z(0), w(1) {}
		Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
		Quaternion(float degree, const Vector3& axis);
		
		inline Quaternion operator * (const Quaternion& q) const { Quaternion ret; Multiply(ret, *this, q); return ret; }
		inline Quaternion& operator *= (const Quaternion& q) { Multiply(*this, Quaternion(*this), q); return *this; }
		
		void Normalize();
		
		void GetRotationAxis(float& out_degree, Vector3& out_axis);
		void GetRotationMatrix(Matrix4& out_m);
		
		static void Multiply(Quaternion& out_q, const Quaternion& q1, const Quaternion& q2);
		
		float x, y, z, w;
	};
	
	void MatrixLookAtRH(Matrix4& out_m,
						const Vector3& eye,
						const Vector3& at,
						const Vector3& up);
	
	void MatrixPerspectiveFovRH(Matrix4	&out_m,
								const float	fov_y,
								const float	aspect,
								const float	near,
								const float	far);
	
	void MatrixOrthoRH(Matrix4	&out_m,
					   const float w,
					   const float h,
					   const float zn,
					   const float zf);
	
	float UnitRandom();
	int RangeRandom(int min, int max);
	float RangeRandom(float min, float max);
	
	struct Color
	{
		Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
		Color(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
		
		inline Color operator * (float value) const { return Color(r * value, g * value, b * value, a * value); }
		inline bool operator == (const Color& c) const { return (r == c.r && g == c.g && b == c.b && a == c.a); }
		inline bool operator != (const Color& c) const { return (r != c.r || g != c.g || b != c.b || a != c.a); }
		
		float r, g, b, a;
	};
}

#endif // ERI_MATH_HELPER_H
