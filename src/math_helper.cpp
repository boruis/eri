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
#include <cmath>
#include <ctime>

namespace ERI {
	
	const float Math::PI = 4 * atan(1.0f);
	const float Math::TWO_PI = PI * 2;
	const float Math::HALF_PI = PI * 0.5f;
	bool Math::is_rand_seed_set = false;
	
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
		out_m.m[0]  = m1.m[0]  * m2.m[0] + m1.m[1]  * m2.m[4] + m1.m[2]  * m2.m[8]  + m1.m[3]  * m2.m[12];
		out_m.m[1]  = m1.m[0]  * m2.m[1] + m1.m[1]  * m2.m[5] + m1.m[2]  * m2.m[9]  + m1.m[3]  * m2.m[13];
		out_m.m[2]  = m1.m[0]  * m2.m[2] + m1.m[1]  * m2.m[6] + m1.m[2]  * m2.m[10] + m1.m[3]  * m2.m[14];
		out_m.m[3]  = m1.m[0]  * m2.m[3] + m1.m[1]  * m2.m[7] + m1.m[2]  * m2.m[11] + m1.m[3]  * m2.m[15];
		
		out_m.m[4]  = m1.m[4]  * m2.m[0] + m1.m[5]  * m2.m[4] + m1.m[6]  * m2.m[8]  + m1.m[7]  * m2.m[12];
		out_m.m[5]  = m1.m[4]  * m2.m[1] + m1.m[5]  * m2.m[5] + m1.m[6]  * m2.m[9]  + m1.m[7]  * m2.m[13];
		out_m.m[6]  = m1.m[4]  * m2.m[2] + m1.m[5]  * m2.m[6] + m1.m[6]  * m2.m[10] + m1.m[7]  * m2.m[14];
		out_m.m[7]  = m1.m[4]  * m2.m[3] + m1.m[5]  * m2.m[7] + m1.m[6]  * m2.m[11] + m1.m[7]  * m2.m[15];
		
		out_m.m[8]  = m1.m[8]  * m2.m[0] + m1.m[9]  * m2.m[4] + m1.m[10] * m2.m[8]  + m1.m[11] * m2.m[12];
		out_m.m[9]  = m1.m[8]  * m2.m[1] + m1.m[9]  * m2.m[5] + m1.m[10] * m2.m[9]  + m1.m[11] * m2.m[13];
		out_m.m[10] = m1.m[8]  * m2.m[2] + m1.m[9]  * m2.m[6] + m1.m[10] * m2.m[10] + m1.m[11] * m2.m[14];
		out_m.m[11] = m1.m[8]  * m2.m[3] + m1.m[9]  * m2.m[7] + m1.m[10] * m2.m[11] + m1.m[11] * m2.m[15];
		
		out_m.m[12] = m1.m[12] * m2.m[0] + m1.m[13] * m2.m[4] + m1.m[14] * m2.m[8]  + m1.m[15] * m2.m[12];
		out_m.m[13] = m1.m[12] * m2.m[1] + m1.m[13] * m2.m[5] + m1.m[14] * m2.m[9]  + m1.m[15] * m2.m[13];
		out_m.m[14] = m1.m[12] * m2.m[2] + m1.m[13] * m2.m[6] + m1.m[14] * m2.m[10] + m1.m[15] * m2.m[14];
		out_m.m[15] = m1.m[12] * m2.m[3] + m1.m[13] * m2.m[7] + m1.m[14] * m2.m[11] + m1.m[15] * m2.m[15];
	}
	
	void Matrix4::Multiply(Vector3& out_v, const Vector3& v, const Matrix4& m)
	{
		out_v.x = m.m[0] * v.x + m.m[4] * v.y + m.m[8]  * v.z + m.m[12];
		out_v.y = m.m[1] * v.x + m.m[5] * v.y + m.m[9]  * v.z + m.m[13];
		out_v.z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14];
		
		float inv_w = 1.0f / (m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15]);
		
		out_v *= inv_w;
	}
	
	#define _ABS(a)	((a) < 0 ? -(a) : (a))
	
	void Matrix4::Inverse(Matrix4& out_m, const Matrix4& m)
	{
		double	det_1;
		double	pos, neg, temp;
		
		/* Calculate the determinant of submatrix A and determine if the
		 the matrix is singular as limited by the double precision
		 floating-point data representation. */
		pos = neg = 0.0;
		temp =  m.m[0] * m.m[5] * m.m[10];
		if (temp >= 0.0) pos += temp; else neg += temp;
		temp =  m.m[4] * m.m[9] * m.m[2];
		if (temp >= 0.0) pos += temp; else neg += temp;
		temp =  m.m[8] * m.m[1] * m.m[6];
		if (temp >= 0.0) pos += temp; else neg += temp;
		temp = -m.m[8] * m.m[5] * m.m[2];
		if (temp >= 0.0) pos += temp; else neg += temp;
		temp = -m.m[4] * m.m[1] * m.m[10];
		if (temp >= 0.0) pos += temp; else neg += temp;
		temp = -m.m[0] * m.m[9] * m.m[6];
		if (temp >= 0.0) pos += temp; else neg += temp;
		det_1 = pos + neg;
		
		/* Is the submatrix A singular? */
		if ((det_1 == 0.0) || (_ABS(det_1 / (pos - neg)) < 1.0e-15))
		{
			/* Matrix M has no inverse */
			printf("Matrix has no inverse : singular matrix\n");
			return;
		}
		else
		{
			/* Calculate inverse(A) = adj(A) / det(A) */
			det_1 = 1.0 / det_1;
			out_m.m[0]  =   (m.m[5] * m.m[10] - m.m[9] * m.m[6]) * (float)det_1;
			out_m.m[1]  = - (m.m[1] * m.m[10] - m.m[9] * m.m[2]) * (float)det_1;
			out_m.m[2]  =   (m.m[1] * m.m[6]  - m.m[5] * m.m[2]) * (float)det_1;
			out_m.m[4]  = - (m.m[4] * m.m[10] - m.m[8] * m.m[6]) * (float)det_1;
			out_m.m[5]  =   (m.m[0] * m.m[10] - m.m[8] * m.m[2]) * (float)det_1;
			out_m.m[6]  = - (m.m[0] * m.m[6]  - m.m[4] * m.m[2]) * (float)det_1;
			out_m.m[8]  =   (m.m[4] * m.m[9]  - m.m[8] * m.m[5]) * (float)det_1;
			out_m.m[9]  = - (m.m[0] * m.m[9]  - m.m[8] * m.m[1]) * (float)det_1;
			out_m.m[10] =   (m.m[0] * m.m[5]  - m.m[4] * m.m[1]) * (float)det_1;
			
			/* Calculate -C * inverse(A) */
			out_m.m[12] = - (m.m[12] * out_m.m[0] + m.m[13] * out_m.m[4] + m.m[14] * out_m.m[8]);
			out_m.m[13] = - (m.m[12] * out_m.m[1] + m.m[13] * out_m.m[5] + m.m[14] * out_m.m[9]);
			out_m.m[14] = - (m.m[12] * out_m.m[2] + m.m[13] * out_m.m[6] + m.m[14] * out_m.m[10]);
			
			/* Fill in last row */
			out_m.m[3]  = 0.0f;
			out_m.m[7]  = 0.0f;
			out_m.m[11] = 0.0f;
			out_m.m[15] = 1.0f;
		}
	}
	
	void Matrix4::Translate(Matrix4& out_m, const Vector3& translate)
	{
		out_m.m[0] = 1.0f; out_m.m[4] = 0.0f; out_m.m[8]  = 0.0f; out_m.m[12]= translate.x;
		out_m.m[1] = 0.0f; out_m.m[5] = 1.0f; out_m.m[9]  = 0.0f; out_m.m[13]= translate.y;
		out_m.m[2] = 0.0f; out_m.m[6] = 0.0f; out_m.m[10] = 1.0f; out_m.m[14]= translate.z;
		out_m.m[3] = 0.0f; out_m.m[7] = 0.0f; out_m.m[11] = 0.0f; out_m.m[15]= 1.0f;
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
		
		out_m.m[0]  = x * x * (1 - c) + c;
		out_m.m[4]  = x * y * (1 - c) - (z * s);
		out_m.m[8]  = x * z * (1 - c) + (y * s);
		out_m.m[12] = 0;
		
		out_m.m[1]  = y * x * (1 - c) + (z * s);
		out_m.m[5]  = y * y * (1 - c) + c;
		out_m.m[9]  = y * z * (1 - c) - (x * s);
		out_m.m[13] = 0;
		
		out_m.m[2]  = z * x * (1 - c) - (y * s);
		out_m.m[6]  = z * y * (1 - c) + (x * s);
		out_m.m[10] = z * z * (1 - c) + c;
		out_m.m[14] = 0.0f;
		
		out_m.m[3]  = 0.0f;
		out_m.m[7]  = 0.0f;
		out_m.m[11] = 0.0f;
		out_m.m[15] = 1.0f;
	}
	
	void Matrix4::Scale(Matrix4& out_m, const Vector3& scale)
	{
		out_m.m[0] = scale.x;	out_m.m[4] = 0.0f;		out_m.m[8]  = 0.0f;		out_m.m[12] = 0.0f;
		out_m.m[1] = 0.0f;		out_m.m[5] = scale.y;	out_m.m[9]  = 0.0f;		out_m.m[13] = 0.0f;
		out_m.m[2] = 0.0f;		out_m.m[6] = 0.0f;		out_m.m[10] = scale.z;	out_m.m[14] = 0.0f;
		out_m.m[3] = 0.0f;		out_m.m[7] = 0.0f;		out_m.m[11] = 0.0f;		out_m.m[15] = 1.0f;
	}
	
	void MatrixLookAtRH(Matrix4& out_m,
						const Vector3& eye,
						const Vector3& at,
						const Vector3& up)
	{
		Vector3 f, up_actual, s, u;
		Matrix4	t;

		f = at - eye;
		f.Normalize();

		up_actual = up;
		up_actual.Normalize();

		s = f.CrossProduct(up_actual);
		u = s.CrossProduct(f);

		out_m.m[0]  = s.x;
		out_m.m[1]  = u.x;
		out_m.m[2]  = -f.x;
		out_m.m[3]  = 0;
		
		out_m.m[4]  = s.y;
		out_m.m[5]  = u.y;
		out_m.m[6]  = -f.y;
		out_m.m[7]  = 0;
		
		out_m.m[8]  = s.z;
		out_m.m[9]  = u.z;
		out_m.m[10] = -f.z;
		out_m.m[11] = 0;
		
		out_m.m[12] = 0;
		out_m.m[13] = 0;
		out_m.m[14] = 0;
		out_m.m[15] = 1;
		
		Matrix4::Translate(t, eye * -1);
		Matrix4::Multiply(out_m, t, out_m);
	}
	
	void MatrixPerspectiveFovRH(Matrix4	&out_m,
								const float	fov_y,
								const float	aspect,
								const float	near,
								const float	far)
	{
		float f, n;
		
		// cotangent(a) == 1.0f / tan(a);
		f = 1.0f / (float)tan(fov_y * 0.5f);
		n = 1.0f / (near - far);
		
		out_m.m[ 0] = f / aspect;
		out_m.m[ 1] = 0;
		out_m.m[ 2] = 0;
		out_m.m[ 3] = 0;
		
		out_m.m[ 4] = 0;
		out_m.m[ 5] = f;
		out_m.m[ 6] = 0;
		out_m.m[ 7] = 0;
		
		out_m.m[ 8] = 0;
		out_m.m[ 9] = 0;
		out_m.m[10] = (far + near) * n;
		out_m.m[11] = -1;
		
		out_m.m[12] = 0;
		out_m.m[13] = 0;
		out_m.m[14] = (2 * far * near) * n;
		out_m.m[15] = 0;
	}
	
	void MatrixOrthoRH(Matrix4	&out_m,
					   const float w,
					   const float h,
					   const float zn,
					   const float zf)
	{
		out_m.m[ 0] = 2 / w;
		out_m.m[ 1] = 0;
		out_m.m[ 2] = 0;
		out_m.m[ 3] = 0;
		
		out_m.m[ 4] = 0;
		out_m.m[ 5] = 2 / h;
		out_m.m[ 6] = 0;
		out_m.m[ 7] = 0;
		
		out_m.m[ 8] = 0;
		out_m.m[ 9] = 0;
		out_m.m[10] = 1 / (zn - zf);
		out_m.m[11] = zn / (zn - zf);
		
		out_m.m[12] = 0;
		out_m.m[13] = 0;
		out_m.m[14] = 0;
		out_m.m[15] = 1;
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
		out_m.m[0]  = 1.0f - 2.0f * y * y - 2.0f * z * z;
		out_m.m[1]  = 2.0f * x * y - 2.0f * z * w;
		out_m.m[2]  = 2.0f * x * z + 2.0f * y * w;
		out_m.m[3]  = 0.0f;
		
		out_m.m[4]  = 2.0f * x * y + 2.0f * z * w;
		out_m.m[5]  = 1.0f - 2.0f * x * x - 2.0f * z * z;
		out_m.m[6]  = 2.0f * y * z - 2.0f * x * w;
		out_m.m[7]  = 0.0f;
		
		out_m.m[8]  = 2.0f * x * z - 2.0f * y * w;
		out_m.m[9]  = 2.0f * y * z + 2.0f * x * w;
		out_m.m[10] = 1.0f - 2.0f * x * x - 2.0f * y * y;
		out_m.m[11] = 0.0f;
		
		out_m.m[12] = 0.0f;
		out_m.m[13] = 0.0f;
		out_m.m[14] = 0.0f;
		out_m.m[15] = 1.0f;
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
	

	float UnitRandom()
	{
		if (false == Math::is_rand_seed_set)
		{
			srand(time(NULL));
			Math::is_rand_seed_set = true;
		}
		
		return static_cast<float>(rand()) / RAND_MAX;
	}
	
	int RangeRandom(int min, int max)
	{
		if (false == Math::is_rand_seed_set)
		{
			srand(time(NULL));
			Math::is_rand_seed_set = true;
		}

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
		if (false == Math::is_rand_seed_set)
		{
			srand(time(NULL));
			Math::is_rand_seed_set = true;
		}
		
		if (min > max)
		{
			float tmp = min;
			min = max;
			max = tmp;
		}
		
		return min + UnitRandom() * (max - min);
	}

}
