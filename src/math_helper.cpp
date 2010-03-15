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
