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

#include <vector>

#include "pch.h"

namespace ERI {
	
#pragma mark Math
	
	struct Math
	{
		static const double ZERO_TOLERANCE;
		static const float PI;
		static const float TWO_PI;
		static const float HALF_PI;
		static float ToRadian(float degree);
		static float ToDegree(float radian);
	};
	
	template<typename T>
	inline const T& Min(const T& a, const T& b) { return (a > b) ? b : a; }
	template<typename T>
	inline const T& Max(const T& a, const T& b) { return (a > b) ? a : b; }
	template<typename T>
	inline T Clamp(T a, const T& min_value, const T& max_value)
	{
		ASSERT(min_value < max_value);
		if (a < min_value) a = min_value;
		else if (a > max_value) a = max_value;
		return a;
	}
	template<typename T>
	inline T Abs(const T& a) { return (a > 0) ? a : -a; }
	
	inline int Round(float a)
	{
		if (a > 0.0f)
			return static_cast<int>(a + 0.5f);
		else if (a < 0.0f)
			return static_cast<int>(a - 0.5f);
		else return 0;
	}
	
	inline bool is_power_of_2(uint32_t val)
	{
		return (val & (val - 1)) == 0;
	}
	
	inline uint32_t nex_power_of_2(uint32_t val)
	{
		--val;
		val = val | (val >> 1);
		val = val | (val >> 2);
		val = val | (val >> 4);
		val = val | (val >> 8);
		val = val | (val >> 16);
		return val + 1;
	}

	inline uint32_t prev_power_of_2(uint32_t val)
	{
		--val;
		val = val | (val >> 1);
		val = val | (val >> 2);
		val = val | (val >> 4);
		val = val | (val >> 8);
		val = val | (val >> 16);
		return val - (val >> 1);
	}
	
#pragma mark Vector2
	
	struct Vector3;

	struct Vector2
	{
		Vector2() : x(0), y(0) {}
		Vector2(float _x, float _y) : x(_x), y(_y) {}
		explicit Vector2(const Vector3& v);
		
		inline Vector2 operator + (const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
		inline Vector2 operator - (const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
		inline Vector2 operator * (const Vector2& v) const { return Vector2(x * v.x, y * v.y); }
		inline Vector2 operator * (float scalar) const { return Vector2(x * scalar, y * scalar); }
		inline Vector2 operator / (const Vector2& v) const { return Vector2(x / v.x, y / v.y); }
		inline Vector2& operator += (const Vector2& v) { x += v.x; y += v.y; return *this; }
		inline Vector2& operator -= (const Vector2& v) { x -= v.x; y -= v.y; return *this; }
		inline Vector2& operator *= (const Vector2& v) { x *= v.x; y *= v.y; return *this; }
		inline Vector2& operator *= (float scalar) { x *= scalar; y *= scalar; return *this; }
		inline Vector2& operator /= (const Vector2& v) { x /= v.x; y /= v.y; return *this; }

		inline bool operator == (const Vector2& v) { return (x == v.x && y == v.y); }
		inline bool operator != (const Vector2& v) { return (x != v.x || y != v.y); }
		
		inline float DotProduct(const Vector2& v) const
        {
            return x * v.x + y * v.y;
        }
		
		inline float CrossProduct(const Vector2& v) const
		{
			return x * v.y - y * v.x;
		}
		
		float Length() const;
		float LengthSquared() const;
		float Normalize();
		
		void Rotate(float degree);
		float GetRotateToDegree(const Vector2& target);
		
		float x, y;
	};
	
#pragma mark Vector3
	
	struct Vector3
	{
		Vector3() : x(0), y(0), z(0) {}
		Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
		explicit Vector3(const Vector2& v) : x(v.x), y(v.y), z(0) {}
		
		inline Vector3 operator + (const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
		inline Vector3 operator - (const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
		inline Vector3 operator * (const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
		inline Vector3 operator * (float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
		inline Vector3 operator / (const Vector3& v) const { return Vector3(x / v.x, y / v.y, z / v.z); }
		inline Vector3& operator += (const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
		inline Vector3& operator -= (const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
		inline Vector3& operator *= (const Vector3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
		inline Vector3& operator *= (float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
		inline Vector3& operator /= (const Vector3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
		inline bool operator == (const Vector3& v) { return (x == v.x && y == v.y && z == v.z); }
		inline bool operator != (const Vector3& v) { return (x != v.x || y != v.y || z != v.z); }
		
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
	
	enum Matrix3RowCol
	{
		TRI_00 = 0,
		TRI_10,
		TRI_20,
		
		TRI_01,
		TRI_11,
		TRI_21,
		
		TRI_02,
		TRI_12,
		TRI_22
	};
	
#pragma mark Matrix3
	
	struct Matrix3
	{
		Matrix3() { *this = IDENTITY; }
		Matrix3(float m00, float m01, float m02,
				float m10, float m11, float m12,
				float m20, float m21, float m22)
		{
			m[TRI_00] = m00; m[TRI_01] = m01; m[TRI_02] = m02;
			m[TRI_10] = m10; m[TRI_11] = m11; m[TRI_12] = m12;
			m[TRI_20] = m20; m[TRI_21] = m21; m[TRI_22] = m22;
		}
		
		void QDUDecomposition(Matrix3& mQ, Vector3& vD, Vector3& vU) const;
		
		float m[9];
		
		static const Matrix3 IDENTITY;
	};
	
#pragma mark Matrix4
	
	struct Quaternion;
	struct Plane;
	
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
		explicit Matrix4(const Matrix3& m3)
        {
            m[_00] = m3.m[TRI_00]; m[_01] = m3.m[TRI_01]; m[_02] = m3.m[TRI_02]; m[_03] = 0.0f;
            m[_10] = m3.m[TRI_10]; m[_11] = m3.m[TRI_11]; m[_12] = m3.m[TRI_12]; m[_13] = 0.0f;
            m[_20] = m3.m[TRI_20]; m[_21] = m3.m[TRI_21]; m[_22] = m3.m[TRI_22]; m[_23] = 0.0f;
            m[_30] = 0.0f; m[_31] = 0.0f; m[_32] = 0.0f; m[_33] = 1.0f;
        }
		
		inline float Get(int row, int col) const
		{
			return m[col * 4 + row];
		}
		
		inline void Set(int row, int col, float f)
		{
			m[col * 4 + row] = f;
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
		
		inline Vector3 GetTranslate() const
		{
			ASSERT(is_affine());
			return Vector3(m[_03], m[_13], m[_23]);
		}
		
		inline bool is_affine() const
        {
            return m[_30] == 0 && m[_31] == 0 && m[_32] == 0 && m[_33] == 1;
        }
		
		void ExtractMatrix3(Matrix3& m3) const;
		void ExtractTransform(Vector3& scale, Quaternion& rotate, Vector3& translate) const;
		void MakeTransform(const Vector3& scale, const Quaternion& rotate, const Vector3& translate);
		
		float m[16];
		
		static void Multiply(Matrix4& out_m, const Matrix4& m1, const Matrix4& m2);
		static void Multiply(Vector3& out_v, const Matrix4& m, const Vector3& v);
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
	
	void ExtractFrustum(const Matrix4& view_matrix, const Matrix4& projection_matrix, Plane* out_frustum);
	
#pragma mark Quaterion
	
	struct Quaternion
	{
		Quaternion() : x(0), y(0), z(0), w(1) {}
		Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
		Quaternion(float degree, const Vector3& axis);
		Quaternion(const Matrix4& rotate);
		
		inline Quaternion operator + (const Quaternion& q) const { return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w); }
		inline Quaternion operator - (const Quaternion& q) const { return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w); }
		inline Quaternion operator * (const Quaternion& q) const { Quaternion ret; Multiply(ret, *this, q); return ret; }
		inline Quaternion operator * (float scalar) const { return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar); }
		inline Quaternion& operator += (const Quaternion& q) { x += q.x; y += q.y; z += q.z; w += q.w; return *this; }
		inline Quaternion& operator -= (const Quaternion& q) { x -= q.x; y -= q.y; z -= q.z; w -= q.w; return *this; }
		inline Quaternion& operator *= (const Quaternion& q) { Multiply(*this, Quaternion(*this), q); return *this; }
		inline Quaternion& operator *= (float scalar) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
		
		inline float DotProduct(const Quaternion& q) const
        {
            return x * q.x + y * q.y + z * q.z + w * q.w;
        }
		
		void Normalize();
		
		void FromRotationAxis(float degree, const Vector3& axis);
		void ToRotationAxis(float& out_degree, Vector3& out_axis) const;

		void FromRotationMatrix(const Matrix4& rotate);
		void ToRotationMatrix(Matrix3& out_rotate) const;
		
		static void Multiply(Quaternion& out_q, const Quaternion& q1, const Quaternion& q2);
		static void Slerp(Quaternion& out_q, float t, const Quaternion& q1, const Quaternion& q2, bool shortest_path = false);
		
		float x, y, z, w;
	};
	
#pragma Geometry
	
	struct Line2
	{
		Vector2	origin;
		Vector2	dir;
	};
	
	struct Ray2
	{
		Vector2	origin;
		Vector2	dir;
	};
	
	struct Ray3
	{
		Vector3	origin;
		Vector3	dir;
	};
	
	struct Segment2
	{
		Segment2(const Vector2& _begin, const Vector2& _end) : begin(_begin), end(_end)
		{
			ComputeCenterDirExtent();
		}
		
		void ComputeCenterDirExtent()
		{
			center = (begin + end) * 0.5f;
			dir = end - begin;
			extent = dir.Normalize() * 0.5f;
		}
		
		Vector2	begin, end;
		Vector2	center;
		Vector2	dir;
		float	extent;
	};
	
	struct Circle
	{
		Vector2	center;
		float	radius;
	};
	
	struct Sphere
	{
		Vector3	center;
		float	radius;
	};
	
	struct Box2
	{
		Box2()
		{
			axis[0] = Vector2(1.0f, 0.0f);
			axis[1] = Vector2(0.0f, 1.0f);
		}
		
		Vector2	center;
		Vector2	axis[2];
		float	extent[2];
	};

	struct Box3
	{
		Box3()
		{
			axis[0] = Vector3(1.0f, 0.0f, 0.0f);
			axis[1] = Vector3(0.0f, 1.0f, 0.0f);
			axis[2] = Vector3(0.0f, 0.0f, 1.0f);
		}

		void GetVertices(Vector3* vertices) const
		{
			Vector3 ext_axis0 = axis[0] * extent[0];
			Vector3 ext_axis1 = axis[1] * extent[1];
			Vector3 ext_axis2 = axis[2] * extent[2];
			
			vertices[0] = center - ext_axis0 - ext_axis1 - ext_axis2;
			vertices[1] = center + ext_axis0 - ext_axis1 - ext_axis2;
			vertices[2] = center + ext_axis0 + ext_axis1 - ext_axis2;
			vertices[3] = center - ext_axis0 + ext_axis1 - ext_axis2;
			vertices[4] = center - ext_axis0 - ext_axis1 + ext_axis2;
			vertices[5] = center + ext_axis0 - ext_axis1 + ext_axis2;
			vertices[6] = center + ext_axis0 + ext_axis1 + ext_axis2;
			vertices[7] = center - ext_axis0 + ext_axis1 + ext_axis2;
		}
		
		Vector3	center;
		Vector3	axis[3];
		float	extent[3];
	};

	struct AABox2
	{
		Vector2	min;
		Vector2 max;
	};
	
	struct Plane
	{
		Vector3 normal;
		float	d;
	};
	
	void CalculateSmallestAABox2(const std::vector<Vector2> points, AABox2& out_box);
	
#pragma mark Intersection
	
	enum IntersectionType
	{
		IT_EMPTY,
		IT_POINT,
		IT_COLINEAR
	};

	float GetPointSegment2DistanceSquared(const Vector2& point, const Segment2& segment);
	float GetPointBox2DistanceSquared(const Vector2& point, const Box2& box);
	IntersectionType CheckIntersectRayRay2(const Ray2& ray1, const Ray2& ray2, Vector2* out_intersect_pos);
	bool IsIntersectLineCircle2(const Line2& line, const Circle& circle, std::vector<float>* out_intersect_length);
	bool IsIntersectRayCircle2(const Ray2& ray, const Circle& circle, std::vector<Vector2>* out_intersect_pos);
	bool IsIntersectBoxCircle2(const Box2& box, const Circle& circle);
	bool IsIntersectBoxBox2(const Box2& box1, const Box2& box2);
	bool IsIntersectAABoxCircle2(const AABox2& box, const Circle& circle);
	bool IsIntersectAABoxAABox2(const AABox2& box1, const AABox2& box2);
	bool IsIntersectRayBox3(const Ray3& ray, const Box3& box);
	float SphereInFrustum(const Sphere& sphere, const Plane* frustum);
	bool BoxInFrustum(const Box3& box, const Plane* frustum);

#pragma mark Random
	
	float UnitRandom();
	int RangeRandom(int min, int max);
	float RangeRandom(float min, float max);
	
#pragma mark CatmullRomSpline
	
	class CatmullRomSpline
	{
	public:
		void SetControlPoints(const Vector2& p0,
							  const Vector2& p1,
							  const Vector2& p2,
							  const Vector2& p3);
		
		Vector2 GetPoint(float t);
		
	private:
		Vector2	f1, f2, f3, f4;
	};
	
#pragma mark CubicBezierSpline
	
	class CubicBezierSpline
	{
	public:
		void SetControlPoints(const Vector2& p0,
							  const Vector2& p1,
							  const Vector2& p2,
							  const Vector2& p3);
		
		Vector2 GetPoint(float t);
		
	private:
		Vector2	f1, f2, f3, f4;
	};
	
#pragma mark Color
	
	struct Color
	{
		Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
		Color(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
		
		inline Color operator + (const Color& c) const { return Color(r + c.r, g + c.g, b + c.b, a + c.a); }
		inline Color operator - (const Color& c) const { return Color(r - c.r, g - c.g, b - c.b, a - c.a); }
		inline Color operator * (const Color& c) const { return Color(r * c.r, g * c.g, b * c.b, a * c.a); }
		inline Color operator * (float value) const { return Color(r * value, g * value, b * value, a * value); }
		inline Color& operator += (const Color& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
		inline Color& operator -= (const Color& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
		inline Color& operator *= (float value) { r *= value; g *= value; b *= value; a *= value; return *this; }
		inline bool operator == (const Color& c) const { return (r == c.r && g == c.g && b == c.b && a == c.a); }
		inline bool operator != (const Color& c) const { return (r != c.r || g != c.g || b != c.b || a != c.a); }
		
		float r, g, b, a;
	};
}

#endif // ERI_MATH_HELPER_H
