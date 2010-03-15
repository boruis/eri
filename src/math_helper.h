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
		static bool is_rand_seed_set;
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
	
	struct Matrix4
	{
		Matrix4() { *this = IDENTITY; }
		Matrix4(float m00, float m01, float m02, float m03,
				float m10, float m11, float m12, float m13,
				float m20, float m21, float m22, float m23,
				float m30, float m31, float m32, float m33)
		{
			m[0] = m00; m[4] = m01; m[8]  = m02; m[12] = m03;
			m[1] = m10; m[5] = m11; m[9]  = m12; m[13] = m13;
			m[2] = m20; m[6] = m21; m[10] = m22; m[14] = m23;
			m[3] = m30; m[7] = m31; m[11] = m32; m[15] = m33;
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
			Multiply(ret, v, *this);
			return ret;
		}
		
		float m[16];
		
		static void Multiply(Matrix4& out_m, const Matrix4& m1, const Matrix4& m2);
		static void Multiply(Vector3& out_v, const Vector3& v, const Matrix4& m);
		static void Inverse(Matrix4& out_m, const Matrix4& m);
		
		static void Translate(Matrix4& out_m, const Vector3& translate);
		static void RotateAxis(Matrix4& out_m, float degree, const Vector3& axis);
		static void Scale(Matrix4& out_m, const Vector3& scale);
		
		static const Matrix4 IDENTITY;
	};
	
	void MatrixLookAtRH(Matrix4& out_m,
						const Vector3& eye,
						const Vector3& at,
						const Vector3& up);
	
	struct Color
	{
		Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
		Color(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
		
		inline bool operator == (const Color& c) const { return (r == c.r && g == c.g && b == c.b && a == c.a); }
		inline bool operator != (const Color& c) const { return (r != c.r || g != c.g || b != c.b || a != c.a); }
	
		float r, g, b, a;
	};
	
	float UnitRandom();
	int RangeRandom(int min, int max);
	float RangeRandom(float min, float max);
}

#endif // ERI_MATH_HELPER_H
