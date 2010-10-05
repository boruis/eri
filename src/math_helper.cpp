/*
 *  math_helper.cpp
 *  eri
 *
 *  Created by exe on 11/29/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "math_helper.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

namespace ERI {
	
#pragma mark Geometry
	
	const float Math::PI = 4 * atan(1.0f);
	const float Math::TWO_PI = PI * 2;
	const float Math::HALF_PI = PI * 0.5f;
	
	static const float inverse_degree = 1.0f / 360;
	static const float inverse_radian = 1.0f / Math::TWO_PI;
	
	float Math::ToRadian(float degree)
	{
		return degree * inverse_degree * TWO_PI;
	}

	float Math::ToDegree(float radian)
	{
		return radian * inverse_radian * 360;
	}
	
	float Vector2::Length() const
	{
		return sqrt(x * x + y * y);
	}
	
	float Vector2::LengthSquared() const
	{
		return (x * x + y * y);
	}
	
	float Vector2::Normalize()
	{
		float length = Length();
		
		// Will also work for zero-sized vectors, but will change nothing
		if (length > 1e-08)
		{
			float inv_length = 1.0f / length;
			x *= inv_length;
			y *= inv_length;
		}
		
		return length;
	}
	
	void Vector2::Rotate(float degree)
	{
		float radian = Math::ToRadian(degree);
		float cos_value = cos(radian);
		float sin_value = sin(radian);
		Vector2 v(x * cos_value - y * sin_value, y * cos_value + x * sin_value);
		(*this) = v;
	}
	
	float Vector3::Length() const
	{
		return sqrt(x * x + y * y + z * z);
	}
	
	float Vector3::LengthSquared() const
	{
		return (x * x + y * y + z * z);
	}
	
	float Vector3::Normalize()
	{
		float length = Length();
		
		// Will also work for zero-sized vectors, but will change nothing
		if (length > 1e-08)
		{
			float inv_length = 1.0f / length;
			x *= inv_length;
			y *= inv_length;
			z *= inv_length;
		}
		
		return length;
	}

	const Matrix4 Matrix4::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f);
	
	void Matrix4::Multiply(Matrix4& out_m, const Matrix4& m1, const Matrix4& m2)
	{
		ASSERT((&out_m != &m1) && (&out_m != &m2));
		
		out_m.m[_00] = m1.m[_00] * m2.m[_00] + m1.m[_01] * m2.m[_10] + m1.m[_02] * m2.m[_20] + m1.m[_03] * m2.m[_30];
		out_m.m[_01] = m1.m[_00] * m2.m[_01] + m1.m[_01] * m2.m[_11] + m1.m[_02] * m2.m[_21] + m1.m[_03] * m2.m[_31];
		out_m.m[_02] = m1.m[_00] * m2.m[_02] + m1.m[_01] * m2.m[_12] + m1.m[_02] * m2.m[_22] + m1.m[_03] * m2.m[_32];
		out_m.m[_03] = m1.m[_00] * m2.m[_03] + m1.m[_01] * m2.m[_13] + m1.m[_02] * m2.m[_23] + m1.m[_03] * m2.m[_33];
		
		out_m.m[_10] = m1.m[_10] * m2.m[_00] + m1.m[_11] * m2.m[_10] + m1.m[_12] * m2.m[_20] + m1.m[_13] * m2.m[_30];
		out_m.m[_11] = m1.m[_10] * m2.m[_01] + m1.m[_11] * m2.m[_11] + m1.m[_12] * m2.m[_21] + m1.m[_13] * m2.m[_31];
		out_m.m[_12] = m1.m[_10] * m2.m[_02] + m1.m[_11] * m2.m[_12] + m1.m[_12] * m2.m[_22] + m1.m[_13] * m2.m[_32];
		out_m.m[_13] = m1.m[_10] * m2.m[_03] + m1.m[_11] * m2.m[_13] + m1.m[_12] * m2.m[_23] + m1.m[_13] * m2.m[_33];
		
		out_m.m[_20] = m1.m[_20] * m2.m[_00] + m1.m[_21] * m2.m[_10] + m1.m[_22] * m2.m[_20] + m1.m[_23] * m2.m[_30];
		out_m.m[_21] = m1.m[_20] * m2.m[_01] + m1.m[_21] * m2.m[_11] + m1.m[_22] * m2.m[_21] + m1.m[_23] * m2.m[_31];
		out_m.m[_22] = m1.m[_20] * m2.m[_02] + m1.m[_21] * m2.m[_12] + m1.m[_22] * m2.m[_22] + m1.m[_23] * m2.m[_32];
		out_m.m[_23] = m1.m[_20] * m2.m[_03] + m1.m[_21] * m2.m[_13] + m1.m[_22] * m2.m[_23] + m1.m[_23] * m2.m[_33];
		
		out_m.m[_30] = m1.m[_30] * m2.m[_00] + m1.m[_31] * m2.m[_10] + m1.m[_32] * m2.m[_20] + m1.m[_33] * m2.m[_30];
		out_m.m[_31] = m1.m[_30] * m2.m[_01] + m1.m[_31] * m2.m[_11] + m1.m[_32] * m2.m[_21] + m1.m[_33] * m2.m[_31];
		out_m.m[_32] = m1.m[_30] * m2.m[_02] + m1.m[_31] * m2.m[_12] + m1.m[_32] * m2.m[_22] + m1.m[_33] * m2.m[_32];
		out_m.m[_33] = m1.m[_30] * m2.m[_03] + m1.m[_31] * m2.m[_13] + m1.m[_32] * m2.m[_23] + m1.m[_33] * m2.m[_33];
	}
	
	void Matrix4::Multiply(Vector3& out_v, const Matrix4& m, const Vector3& v)
	{
		ASSERT(&out_v != &v);
		
		float inv_w = 1.0f / (m.m[_30] * v.x + m.m[_31] * v.y + m.m[_32] * v.z + m.m[_33]);
		
		out_v.x = (m.m[_00] * v.x + m.m[_01] * v.y + m.m[_02] * v.z + m.m[_03]) * inv_w;
		out_v.y = (m.m[_10] * v.x + m.m[_11] * v.y + m.m[_12] * v.z + m.m[_13]) * inv_w;
		out_v.z = (m.m[_20] * v.x + m.m[_21] * v.y + m.m[_22] * v.z + m.m[_23]) * inv_w;
	}
	
	void Matrix4::Inverse(Matrix4& out_m, const Matrix4& m)
	{
		double	det_1;
		double	pos, neg, temp;
		
		/* Calculate the determinant of submatrix A and determine if the
		 the matrix is singular as limited by the double precision
		 floating-point data representation. */
		pos = neg = 0.0;
		temp =  m.m[_00] * m.m[_11] * m.m[_22];
		if (temp >= 0.0) pos += temp; else neg += temp;
		temp =  m.m[_01] * m.m[_12] * m.m[_20];
		if (temp >= 0.0) pos += temp; else neg += temp;
		temp =  m.m[_02] * m.m[_10] * m.m[_21];
		if (temp >= 0.0) pos += temp; else neg += temp;
		temp = -m.m[_02] * m.m[_11] * m.m[_20];
		if (temp >= 0.0) pos += temp; else neg += temp;
		temp = -m.m[_01] * m.m[_10] * m.m[_22];
		if (temp >= 0.0) pos += temp; else neg += temp;
		temp = -m.m[_00] * m.m[_12] * m.m[_21];
		if (temp >= 0.0) pos += temp; else neg += temp;
		det_1 = pos + neg;
		
		/* Is the submatrix A singular? */
		if ((det_1 == 0.0) || (Abs(det_1 / (pos - neg)) < 1.0e-15))
		{
			/* Matrix M has no inverse */
			printf("Matrix has no inverse : singular matrix\n");
			return;
		}
		else
		{
			/* Calculate inverse(A) = adj(A) / det(A) */
			det_1 = 1.0 / det_1;
			out_m.m[_00] =   (m.m[_11] * m.m[_22] - m.m[_12] * m.m[_21]) * (float)det_1;
			out_m.m[_10] = - (m.m[_10] * m.m[_22] - m.m[_12] * m.m[_20]) * (float)det_1;
			out_m.m[_20] =   (m.m[_10] * m.m[_21] - m.m[_11] * m.m[_20]) * (float)det_1;
			out_m.m[_01] = - (m.m[_01] * m.m[_22] - m.m[_02] * m.m[_21]) * (float)det_1;
			out_m.m[_11] =   (m.m[_00] * m.m[_22] - m.m[_02] * m.m[_20]) * (float)det_1;
			out_m.m[_21] = - (m.m[_00] * m.m[_21] - m.m[_01] * m.m[_20]) * (float)det_1;
			out_m.m[_02] =   (m.m[_01] * m.m[_12] - m.m[_02] * m.m[_11]) * (float)det_1;
			out_m.m[_12] = - (m.m[_00] * m.m[_12] - m.m[_02] * m.m[_10]) * (float)det_1;
			out_m.m[_22] =   (m.m[_00] * m.m[_11] - m.m[_01] * m.m[_10]) * (float)det_1;
			
			/* Calculate -C * inverse(A) */
			out_m.m[_03] = - (m.m[_03] * out_m.m[_00] + m.m[_13] * out_m.m[_01] + m.m[_23] * out_m.m[_02]);
			out_m.m[_13] = - (m.m[_03] * out_m.m[_10] + m.m[_13] * out_m.m[_11] + m.m[_23] * out_m.m[_12]);
			out_m.m[_23] = - (m.m[_03] * out_m.m[_20] + m.m[_13] * out_m.m[_21] + m.m[_23] * out_m.m[_22]);
			
			/* Fill in last row */
			out_m.m[_30]  = 0.0f;
			out_m.m[_31]  = 0.0f;
			out_m.m[_32] = 0.0f;
			out_m.m[_33] = 1.0f;
		}
	}
	
	void Matrix4::Translate(Matrix4& out_m, const Vector3& translate)
	{
		out_m.m[_00] = 1.0f; out_m.m[_01] = 0.0f; out_m.m[_02] = 0.0f; out_m.m[_03] = translate.x;
		out_m.m[_10] = 0.0f; out_m.m[_11] = 1.0f; out_m.m[_12] = 0.0f; out_m.m[_13] = translate.y;
		out_m.m[_20] = 0.0f; out_m.m[_21] = 0.0f; out_m.m[_22] = 1.0f; out_m.m[_23] = translate.z;
		out_m.m[_30] = 0.0f; out_m.m[_31] = 0.0f; out_m.m[_32] = 0.0f; out_m.m[_33] = 1.0f;
	}
	
	void Matrix4::RotateAxis(Matrix4& out_m, float degree, const Vector3& axis)
	{
		float angle = degree / 360 * Math::TWO_PI;
		float s = sin(angle);
		float c = cos(angle);

		float x, y, z;
		x = axis.x;
		y = axis.y;
		z = axis.z;
		
		float length = axis.Length();
		if (length > 1e-08)
		{
			float inv_length = 1.0f / length;
			x *= inv_length;
			y *= inv_length;
			z *= inv_length;
		}
		
		out_m.m[_00]  = x * x * (1 - c) + c;
		out_m.m[_01]  = x * y * (1 - c) - (z * s);
		out_m.m[_02]  = x * z * (1 - c) + (y * s);
		out_m.m[_03] = 0;
		
		out_m.m[_10]  = y * x * (1 - c) + (z * s);
		out_m.m[_11]  = y * y * (1 - c) + c;
		out_m.m[_12]  = y * z * (1 - c) - (x * s);
		out_m.m[_13] = 0;
		
		out_m.m[_20]  = z * x * (1 - c) - (y * s);
		out_m.m[_21]  = z * y * (1 - c) + (x * s);
		out_m.m[_22] = z * z * (1 - c) + c;
		out_m.m[_23] = 0.0f;
		
		out_m.m[_30]  = 0.0f;
		out_m.m[_31]  = 0.0f;
		out_m.m[_32] = 0.0f;
		out_m.m[_33] = 1.0f;
	}
	
	void Matrix4::Scale(Matrix4& out_m, const Vector3& scale)
	{
		out_m.m[_00] = scale.x;	out_m.m[_01] = 0.0f;	out_m.m[_02] = 0.0f;	out_m.m[_03] = 0.0f;
		out_m.m[_10] = 0.0f;	out_m.m[_11] = scale.y;	out_m.m[_12] = 0.0f;	out_m.m[_13] = 0.0f;
		out_m.m[_20] = 0.0f;	out_m.m[_21] = 0.0f;	out_m.m[_22] = scale.z;	out_m.m[_23] = 0.0f;
		out_m.m[_30] = 0.0f;	out_m.m[_31] = 0.0f;	out_m.m[_32] = 0.0f;	out_m.m[_33] = 1.0f;
	}
	
	void MatrixLookAtRH(Matrix4& out_m,
						const Vector3& eye,
						const Vector3& at,
						const Vector3& up)
	{
		Vector3 f, up_actual, s, u;

		f = at - eye;
		f.Normalize();

		up_actual = up;
		up_actual.Normalize();

		s = f.CrossProduct(up_actual);
		u = s.CrossProduct(f);
		
		Matrix4	t1(  s.x,  s.y,  s.z,    0,
				     u.x,  u.y,  u.z,    0,
				    -f.x, -f.y, -f.z,    0,
				       0,    0,    0,    1);
		
		Matrix4	t2;
		Matrix4::Translate(t2, eye * -1);
		
		Matrix4::Multiply(out_m, t1, t2);
	}
	
	void MatrixPerspectiveFovRH(Matrix4	&out_m,
								const float	fov_y,
								const float	aspect,
								const float	near_z,
								const float	far_z)
	{
		float f, n;
		
		f = 1.0f / (float)tan(fov_y * 0.5f);
		n = 1.0f / (near_z - far_z);
		
		out_m.m[_00] = f / aspect;
		out_m.m[_10] = 0;
		out_m.m[_20] = 0;
		out_m.m[_30] = 0;
		
		out_m.m[_01] = 0;
		out_m.m[_11] = f;
		out_m.m[_21] = 0;
		out_m.m[_31] = 0;
		
		out_m.m[_02] = 0;
		out_m.m[_12] = 0;
		out_m.m[_22] = (far_z + near_z) * n;
		out_m.m[_32] = -1;
		
		out_m.m[_03] = 0;
		out_m.m[_13] = 0;
		out_m.m[_23] = (2 * far_z * near_z) * n;
		out_m.m[_33] = 0;
	}
	
	void MatrixOrthoRH(Matrix4	&out_m,
					   const float w,
					   const float h,
					   const float near_z,
					   const float far_z)
	{
		out_m.m[_00] = 2 / w;
		out_m.m[_10] = 0;
		out_m.m[_20] = 0;
		out_m.m[_30] = 0;
		
		out_m.m[_01] = 0;
		out_m.m[_11] = 2 / h;
		out_m.m[_21] = 0;
		out_m.m[_31] = 0;
		
		out_m.m[_02] = 0;
		out_m.m[_12] = 0;
		out_m.m[_22] = -2 / (far_z - near_z);
		out_m.m[_32] = 0;
		
		out_m.m[_03] = 0;
		out_m.m[_13] = 0;
		out_m.m[_23] = (far_z + near_z) / (far_z - near_z);
		out_m.m[_33] = 1;
	}
	
	Quaternion::Quaternion(float degree, const Vector3& axis)
	{
		float sin_value = static_cast<float>(sin(degree / 360 * Math::TWO_PI * 0.5f));
		float cos_value = static_cast<float>(cos(degree / 360 * Math::TWO_PI * 0.5f));
		
		/* Create quaternion */
		x = axis.x * sin_value;
		y = axis.y * sin_value;
		z = axis.z * sin_value;
		w = cos_value;
		
		Normalize();
	}
	
	void Quaternion::Normalize()
	{
		/* Compute quaternion magnitude */
		double temp = w * w + x * x + y * y + z * z;
		float magnitude = static_cast<float>(sqrt(temp));
		
		/* Divide each quaternion component by this magnitude */
		if (magnitude != 0.0f)
		{
			magnitude = 1.0f / magnitude;
			x *= magnitude;
			y *= magnitude;
			z *= magnitude;
			w *= magnitude;
		}
	}
	
	void Quaternion::GetRotationAxis(float& out_degree, Vector3& out_axis)
	{
		/* Compute some values */
		float cos_angle	= w;
		double temp	= 1.0f - cos_angle * cos_angle;
		float sin_angle	= static_cast<float>(sqrt(temp));
		
		/* This is to avoid a division by zero */
		if (static_cast<float>(fabs(sin_angle)) < 0.0005f)
			sin_angle = 1.0f;
		
		/* Get axis vector */
		out_axis.x = x / sin_angle;
		out_axis.y = y / sin_angle;
		out_axis.z = z / sin_angle;
		
		out_degree = static_cast<float>(acos(cos_angle) * 2) / Math::TWO_PI * 360;
	}
	
	void Quaternion::GetRotationMatrix(Matrix4& out_m)
	{
		out_m.m[_00] = 1.0f - 2.0f * y * y - 2.0f * z * z;
		out_m.m[_10] = 2.0f * x * y - 2.0f * z * w;
		out_m.m[_20] = 2.0f * x * z + 2.0f * y * w;
		out_m.m[_30] = 0.0f;
		
		out_m.m[_01] = 2.0f * x * y + 2.0f * z * w;
		out_m.m[_11] = 1.0f - 2.0f * x * x - 2.0f * z * z;
		out_m.m[_21] = 2.0f * y * z - 2.0f * x * w;
		out_m.m[_31] = 0.0f;
		
		out_m.m[_02] = 2.0f * x * z - 2.0f * y * w;
		out_m.m[_12] = 2.0f * y * z + 2.0f * x * w;
		out_m.m[_22] = 1.0f - 2.0f * x * x - 2.0f * y * y;
		out_m.m[_32] = 0.0f;
		
		out_m.m[_03] = 0.0f;
		out_m.m[_13] = 0.0f;
		out_m.m[_23] = 0.0f;
		out_m.m[_33] = 1.0f;
	}
	
	void Quaternion::Multiply(Quaternion& out_q, const Quaternion& q1, const Quaternion& q2)
	{
		Vector3	cross_product;
		
		/* Compute scalar component */
		out_q.w = (q1.w * q2.w) - (q1.x * q2.x + q1.y * q2.y + q1.z * q2.z);
		
		/* Compute cross product */
		cross_product.x = q1.y * q2.z - q1.z * q2.y;
		cross_product.y = q1.z * q2.x - q1.x * q2.z;
		cross_product.z = q1.x * q2.y - q1.y * q2.x;
		
		/* Compute result vector */
		out_q.x = (q1.w * q2.x) + (q2.w * q1.x) + cross_product.x;
		out_q.y = (q1.w * q2.y) + (q2.w * q1.y) + cross_product.y;
		out_q.z = (q1.w * q2.z) + (q2.w * q1.z) + cross_product.z;
		
		/* Normalize resulting quaternion */
		out_q.Normalize();
	}
	
#pragma mark Intersection
	
	float GetPointBox2DistanceSquared(const Vector2& point, const Box2& box)
	{
		// Work in the box's coordinate system.
		Vector2 diff = point - box.center;
		
		// Compute squared distance and closest point on box.
		float sqr_distance = 0.0f;
		float delta;
		float closest[2];

		for (int i = 0; i < 2; ++i)
		{
			closest[i] = diff.DotProduct(box.axis[i]);
			if (closest[i] < -box.extent[i])
			{
				delta = closest[i] + box.extent[i];
				sqr_distance += delta*delta;
				closest[i] = -box.extent[i];
			}
			else if (closest[i] > box.extent[i])
			{
				delta = closest[i] - box.extent[i];
				sqr_distance += delta*delta;
				closest[i] = box.extent[i];
			}
		}

		return sqr_distance;
	}	
	
	bool IsIntersectBoxCircle2(const Box2& box, const Circle2& circle)
	{
		float distance_squared = GetPointBox2DistanceSquared(circle.center, box);
		return distance_squared <= (circle.radius * circle.radius);
	}
	
	bool IsIntersectBoxBox2(const Box2& box1, const Box2& box2)
	{
		// Convenience variables.
		const Vector2* A = box1.axis;
		const Vector2* B = box2.axis;
		const float* EA = box1.extent;
		const float* EB = box2.extent;
		
		// Compute difference of box centers, D = C1-C0.
		Vector2 D = box2.center - box1.center;
		
		float AbsAdB[2][2], AbsAdD, RSum;
		
		// axis C0+t*A0
		AbsAdB[0][0] = Abs(A[0].DotProduct(B[0]));
		AbsAdB[0][1] = Abs(A[0].DotProduct(B[1]));
		AbsAdD = Abs(A[0].DotProduct(D));
		RSum = EA[0] + EB[0]*AbsAdB[0][0] + EB[1]*AbsAdB[0][1];
		if (AbsAdD > RSum)
		{
			return false;
		}
		
		// axis C0+t*A1
		AbsAdB[1][0] = Abs(A[1].DotProduct(B[0]));
		AbsAdB[1][1] = Abs(A[1].DotProduct(B[1]));
		AbsAdD = Abs(A[1].DotProduct(D));
		RSum = EA[1] + EB[0]*AbsAdB[1][0] + EB[1]*AbsAdB[1][1];
		if (AbsAdD > RSum)
		{
			return false;
		}
		
		// axis C0+t*B0
		AbsAdD = Abs(B[0].DotProduct(D));
		RSum = EB[0] + EA[0]*AbsAdB[0][0] + EA[1]*AbsAdB[1][0];
		if (AbsAdD > RSum)
		{
			return false;
		}
		
		// axis C0+t*B1
		AbsAdD = Abs(B[1].DotProduct(D));
		RSum = EB[1] + EA[0]*AbsAdB[0][1] + EA[1]*AbsAdB[1][1];
		if (AbsAdD > RSum)
		{
			return false;
		}
		
		return true;
	}
	
#pragma mark Random
	
	static bool is_rand_seed_set = false;

	static void SetRandomSeed()
	{
		if (false == is_rand_seed_set)
		{
			srand(static_cast<unsigned int>(time(NULL)));
			is_rand_seed_set = true;
		}
	}

	float UnitRandom()
	{
		SetRandomSeed();
		
		return static_cast<float>(rand()) / RAND_MAX;
	}
	
	int RangeRandom(int min, int max)
	{
		SetRandomSeed();

		if (min > max)
		{
			int tmp = min;
			min = max;
			max = tmp;
		}
		
		return min + rand() % (max - min + 1);
	}
	
	float RangeRandom(float min, float max)
	{
		SetRandomSeed();
		
		if (min > max)
		{
			float tmp = min;
			min = max;
			max = tmp;
		}
		
		return min + UnitRandom() * (max - min);
	}

}
