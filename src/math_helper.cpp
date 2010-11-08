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
	
	const double Math::ZERO_TOLERANCE = 1e-06;

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
		if (length > Math::ZERO_TOLERANCE)
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
		if (length > Math::ZERO_TOLERANCE)
		{
			float inv_length = 1.0f / length;
			x *= inv_length;
			y *= inv_length;
			z *= inv_length;
		}
		
		return length;
	}

	const Matrix3 Matrix3::IDENTITY(1.0f, 0.0f, 0.0f,
									0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 1.0f);
	
	void Matrix3::QDUDecomposition(Matrix3& mQ, Vector3& vD, Vector3& vU) const
    {
        // Factor M = QR = QDU where Q is orthogonal, D is diagonal,
        // and U is upper triangular with ones on its diagonal.  Algorithm uses
        // Gram-Schmidt orthogonalization (the QR algorithm).
        //
        // If M = [ m0 | m1 | m2 ] and Q = [ q0 | q1 | q2 ], then
        //
        //   q0 = m0/|m0|
        //   q1 = (m1-(q0*m1)q0)/|m1-(q0*m1)q0|
        //   q2 = (m2-(q0*m2)q0-(q1*m2)q1)/|m2-(q0*m2)q0-(q1*m2)q1|
        //
        // where |V| indicates length of vector V and A*B indicates dot
        // product of vectors A and B.  The matrix R has entries
        //
        //   r00 = q0*m0  r01 = q0*m1  r02 = q0*m2
        //   r10 = 0      r11 = q1*m1  r12 = q1*m2
        //   r20 = 0      r21 = 0      r22 = q2*m2
        //
        // so D = diag(r00,r11,r22) and U has entries u01 = r01/r00,
        // u02 = r02/r00, and u12 = r12/r11.
		
        // Q = rotation
        // D = scaling
        // U = shear
		
        // D stores the three diagonal entries r00, r11, r22
        // U stores the entries U[0] = u01, U[1] = u02, U[2] = u12
		
        // build orthogonal matrix Q
        float inv_length = 1.0f / sqrt(m[TRI_00] * m[TRI_00] + m[TRI_10] * m[TRI_10] + m[TRI_20] * m[TRI_20]);
		
        mQ.m[TRI_00] = m[TRI_00] * inv_length;
        mQ.m[TRI_10] = m[TRI_10] * inv_length;
        mQ.m[TRI_20] = m[TRI_20] * inv_length;
		
        float dot = mQ.m[TRI_00] * m[TRI_01] + mQ.m[TRI_10] * m[TRI_11] + mQ.m[TRI_20] * m[TRI_21];
        mQ.m[TRI_01] = m[TRI_01] - dot * mQ.m[TRI_00];
        mQ.m[TRI_11] = m[TRI_11] - dot * mQ.m[TRI_10];
        mQ.m[TRI_21] = m[TRI_21] - dot * mQ.m[TRI_20];
        inv_length = 1.0f / sqrt(mQ.m[TRI_01] * mQ.m[TRI_01] + mQ.m[TRI_11] * mQ.m[TRI_11] + mQ.m[TRI_21] * mQ.m[TRI_21]);
        mQ.m[TRI_01] *= inv_length;
        mQ.m[TRI_11] *= inv_length;
        mQ.m[TRI_21] *= inv_length;
		
        dot = mQ.m[TRI_00] * m[TRI_02] + mQ.m[TRI_10] * m[TRI_12] + mQ.m[TRI_20] * m[TRI_22];
        mQ.m[TRI_02] = m[TRI_02] - dot * mQ.m[TRI_00];
        mQ.m[TRI_12] = m[TRI_12] - dot * mQ.m[TRI_10];
        mQ.m[TRI_22] = m[TRI_22] - dot * mQ.m[TRI_20];
        dot = mQ.m[TRI_01] * m[TRI_02] + mQ.m[TRI_11] * m[TRI_12] +	mQ.m[TRI_21] * m[TRI_22];
        mQ.m[TRI_02] -= dot * mQ.m[TRI_01];
        mQ.m[TRI_12] -= dot * mQ.m[TRI_11];
        mQ.m[TRI_22] -= dot * mQ.m[TRI_21];
        inv_length = 1.0f / sqrt(mQ.m[TRI_02] * mQ.m[TRI_02] + mQ.m[TRI_12] * mQ.m[TRI_12] + mQ.m[TRI_22] * mQ.m[TRI_22]);
        mQ.m[TRI_02] *= inv_length;
        mQ.m[TRI_12] *= inv_length;
        mQ.m[TRI_22] *= inv_length;
		
        // guarantee that orthogonal matrix has determinant 1 (no reflections)
        float det = mQ.m[TRI_00] * mQ.m[TRI_11] * mQ.m[TRI_22] + mQ.m[TRI_01] * mQ.m[TRI_12] * mQ.m[TRI_20] +
			mQ.m[TRI_02] * mQ.m[TRI_10] * mQ.m[TRI_21] - mQ.m[TRI_02] * mQ.m[TRI_11] * mQ.m[TRI_20] -
			mQ.m[TRI_01] * mQ.m[TRI_10] * mQ.m[TRI_22] - mQ.m[TRI_00] * mQ.m[TRI_12] * mQ.m[TRI_21];
		
        if (det < 0.0f)
        {
            for (size_t i = 0; i < 9; i++)
				mQ.m[i] *= -1;
        }
		
        // build "right" matrix R
        Matrix3 mR;
        mR.m[TRI_00] = mQ.m[TRI_00] * m[TRI_00] + mQ.m[TRI_10] * m[TRI_10] + mQ.m[TRI_20] * m[TRI_20];
        mR.m[TRI_01] = mQ.m[TRI_00] * m[TRI_01] + mQ.m[TRI_10] * m[TRI_11] + mQ.m[TRI_20] * m[TRI_21];
        mR.m[TRI_11] = mQ.m[TRI_01] * m[TRI_01] + mQ.m[TRI_11] * m[TRI_11] + mQ.m[TRI_21] * m[TRI_21];
        mR.m[TRI_02] = mQ.m[TRI_00] * m[TRI_02] + mQ.m[TRI_10] * m[TRI_12] + mQ.m[TRI_20] * m[TRI_22];
        mR.m[TRI_12] = mQ.m[TRI_01] * m[TRI_02] + mQ.m[TRI_11] * m[TRI_12] + mQ.m[TRI_21] * m[TRI_22];
        mR.m[TRI_22] = mQ.m[TRI_02] * m[TRI_02] + mQ.m[TRI_12] * m[TRI_12] + mQ.m[TRI_22] * m[TRI_22];
		
        // the scaling component
        vD.x = mR.m[TRI_00];
        vD.y = mR.m[TRI_11];
        vD.z = mR.m[TRI_22];
		
        // the shear component
        float inv_D0 = 1.0f / vD.x;
        vU.x = mR.m[TRI_01] * inv_D0;
        vU.y = mR.m[TRI_02] * inv_D0;
        vU.z = mR.m[TRI_12] / vD.y;
    }
	
	const Matrix4 Matrix4::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f);
	
	void Matrix4::ExtractMatrix3(Matrix3& m3)
	{
		m3.m[TRI_00] = m[_00]; m3.m[TRI_01] = m[_01]; m3.m[TRI_02] = m[_02];
		m3.m[TRI_10] = m[_10]; m3.m[TRI_11] = m[_11]; m3.m[TRI_12] = m[_12];
		m3.m[TRI_20] = m[_20]; m3.m[TRI_21] = m[_21]; m3.m[TRI_22] = m[_22];
	}
	
	void Matrix4::ExtractTransform(Vector3& scale, Quaternion& rotate, Vector3& translate)
	{
		ASSERT(is_affine());
		
		Matrix3 m3;
		ExtractMatrix3(m3);
		
		Matrix3 mQ;
		Vector3 vU;
		m3.QDUDecomposition(mQ, scale, vU);
		
		rotate = Quaternion(Matrix4(mQ));
		translate = Vector3(m[_03], m[_13], m[_23]);
	}
	
	void Matrix4::MakeTransform(const Vector3& scale, const Quaternion& rotate, const Vector3& translate)
	{
		// Ordering:
        //    1. Scale
        //    2. Rotate
        //    3. Translate
		
        Matrix3 rot3;
        rotate.ToRotationMatrix(rot3);
		
        // Set up final matrix with scale, rotation and translation
        m[_00] = scale.x * rot3.m[TRI_00]; m[_01] = scale.y * rot3.m[TRI_01]; m[_02] = scale.z * rot3.m[TRI_02]; m[_03] = translate.x;
        m[_10] = scale.x * rot3.m[TRI_10]; m[_11] = scale.y * rot3.m[TRI_11]; m[_12] = scale.z * rot3.m[TRI_12]; m[_13] = translate.y;
        m[_20] = scale.x * rot3.m[TRI_20]; m[_21] = scale.y * rot3.m[TRI_21]; m[_22] = scale.z * rot3.m[TRI_22]; m[_23] = translate.z;
		
        // No projection term
        m[_30] = 0; m[_31] = 0; m[_32] = 0; m[_33] = 1;
	}
	
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
		float radian = Math::ToRadian(degree);
		float s = sin(radian);
		float c = cos(radian);

		float x, y, z;
		x = axis.x;
		y = axis.y;
		z = axis.z;
		
		float length = axis.Length();
		if (length > Math::ZERO_TOLERANCE)
		{
			float inv_length = 1.0f / length;
			x *= inv_length;
			y *= inv_length;
			z *= inv_length;
		}
		
		out_m.m[_00] = x * x * (1 - c) + c;
		out_m.m[_01] = x * y * (1 - c) - (z * s);
		out_m.m[_02] = x * z * (1 - c) + (y * s);
		out_m.m[_03] = 0;
		
		out_m.m[_10] = y * x * (1 - c) + (z * s);
		out_m.m[_11] = y * y * (1 - c) + c;
		out_m.m[_12] = y * z * (1 - c) - (x * s);
		out_m.m[_13] = 0;
		
		out_m.m[_20] = z * x * (1 - c) - (y * s);
		out_m.m[_21] = z * y * (1 - c) + (x * s);
		out_m.m[_22] = z * z * (1 - c) + c;
		out_m.m[_23] = 0.0f;
		
		out_m.m[_30] = 0.0f;
		out_m.m[_31] = 0.0f;
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
		FromRotationAxis(degree, axis);
	}
	
	Quaternion::Quaternion(const Matrix4& rotate)
	{
		FromRotationMatrix(rotate);
	}
	
	void Quaternion::Normalize()
	{
		/* Compute quaternion magnitude */
		double temp = w * w + x * x + y * y + z * z;
		float magnitude = static_cast<float>(sqrt(temp));
		
		/* Divide each quaternion component by this magnitude */
		if (magnitude > Math::ZERO_TOLERANCE)
		{
			magnitude = 1.0f / magnitude;
			x *= magnitude;
			y *= magnitude;
			z *= magnitude;
			w *= magnitude;
		}
	}
	
	void Quaternion::FromRotationAxis(float degree, const Vector3& axis)
	{
		// assert:  axis[] is unit length
        //
        // The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
		
		float half_radian = Math::ToRadian(degree) * 0.5f;
		float sin_value = sin(half_radian);
		float cos_value = cos(half_radian);
		
		x = axis.x * sin_value;
		y = axis.y * sin_value;
		z = axis.z * sin_value;
		w = cos_value;
	}
	
	void Quaternion::ToRotationAxis(float& out_degree, Vector3& out_axis) const
	{
		// The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
		
        float sqr_length = x * x + y * y + z * z;
        if (sqr_length > Math::ZERO_TOLERANCE)
        {
			out_degree = Math::ToDegree(2.0f * acos(w));
            float inv_length = 1.0f / sqrt(sqr_length);
            out_axis.x = x * inv_length;
            out_axis.y = y * inv_length;
            out_axis.z = z * inv_length;
        }
        else
        {
            // angle is 0 (mod 2*pi), so any axis will do
            out_degree = 0.0f;
            out_axis.x = 1.0f;
            out_axis.y = 0.0f;
            out_axis.z = 0.0f;
        }
	}
	
	void Quaternion::FromRotationMatrix(const Matrix4& rotate)
	{
		// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
		// article "Quaternion Calculus and Fast Animation".
		
		float trace = rotate.m[_00] + rotate.m[_11] + rotate.m[_22];
		float root;
		
		if (trace > 0.0f)
		{
			// |w| > 1/2, may as well choose w > 1/2
			root = sqrt(trace + 1.0f);  // 2w
			w = 0.5f * root;
			root = 0.5f / root;  // 1/(4w)
			x = (rotate.m[_21] - rotate.m[_12]) * root;
			y = (rotate.m[_02] - rotate.m[_20]) * root;
			z = (rotate.m[_10] - rotate.m[_01]) * root;
		}
		else
		{
			// |w| <= 1/2
			const int next[3] = { 1, 2, 0 };
			
			int i = 0;
			if (rotate.m[_11] > rotate.m[_00])
				i = 1;
			if (rotate.m[_22] > rotate.Get(i, i))
				i = 2;
			
			int j = next[i];
			int k = next[j];
			
			root = sqrt(rotate.Get(i, i) - rotate.Get(j, j) - rotate.Get(k, k) + 1.0f);
			float* ref_quat[3] = { &x, &y, &z };
			*ref_quat[i] = 0.5f * root;
			root = 0.5f / root;
			w = (rotate.Get(k, j) - rotate.Get(j, k)) * root;
			*ref_quat[j] = (rotate.Get(j, i) + rotate.Get(i, j)) * root;
			*ref_quat[k] = (rotate.Get(k, i) + rotate.Get(i, k)) * root;
		}
	}
	
	void Quaternion::ToRotationMatrix(Matrix3& out_rotate) const
	{
		float Tx  = x + x;
        float Ty  = y + y;
        float Tz  = z + z;
        float Twx = Tx * w;
        float Twy = Ty * w;
        float Twz = Tz * w;
        float Txx = Tx * x;
        float Txy = Ty * x;
        float Txz = Tz * x;
        float Tyy = Ty * y;
        float Tyz = Tz * y;
        float Tzz = Tz * z;
		
        out_rotate.m[TRI_00] = 1.0f - (Tyy + Tzz);
        out_rotate.m[TRI_01] = Txy - Twz;
        out_rotate.m[TRI_02] = Txz + Twy;
		
        out_rotate.m[TRI_10] = Txy + Twz;
        out_rotate.m[TRI_11] = 1.0f - (Txx + Tzz);
        out_rotate.m[TRI_12] = Tyz - Twx;
		
        out_rotate.m[TRI_20] = Txz - Twy;
        out_rotate.m[TRI_21] = Tyz + Twx;
        out_rotate.m[TRI_22] = 1.0f - (Txx + Tyy);
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
	
	void Quaternion::Slerp(Quaternion& out_q, float t, const Quaternion& q1, const Quaternion& q2, bool shortest_path /*= false*/)
	{
		float cos_value = q1.DotProduct(q2);
        Quaternion temp_q = q2;
		
        // Do we need to invert rotation?
        if (cos_value < 0.0f && shortest_path)
        {
            cos_value = -cos_value;
            temp_q *= -1;
        }
		
        if (Abs(cos_value) < 1 - 1e-03)
        {
            // Standard case (slerp)
            float sin_value = sqrt(1 - cos_value * cos_value);
            float radian = atan2(sin_value, cos_value);
            float inv_sin = 1.0f / sin_value;
            float coeff0 = sin((1.0f - t) * radian) * inv_sin;
            float coeff1 = sin(t * radian) * inv_sin;
            out_q = q1 * coeff0 + temp_q * coeff1;
        }
        else
        {
            // There are two situations:
            // 1. "q1" and "q2" are very close (fCos ~= +1), so we can do a linear
            //    interpolation safely.
            // 2. "q1" and "q2" are almost inverse of each other (fCos ~= -1), there
            //    are an infinite number of possibilities interpolation. but we haven't
            //    have method to fix this case, so just use linear interpolation here.
            out_q = q1 * (1.0f - t) + temp_q * t;
            // taking the complement requires renormalisation
            out_q.Normalize();
        }
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
	
	IntersectionType CheckIntersectRayRay2(const Ray2& ray1, const Ray2& ray2, Vector2* out_intersect_pos)
	{
		Vector2 origin_diff = ray2.origin - ray1.origin;
		
		float dir1_cross_dir2 = ray1.dir.CrossProduct(ray2.dir);
		if (Abs(dir1_cross_dir2) > Math::ZERO_TOLERANCE)
		{
			// Lines intersect in a single point.
			float inv_cross = 1.0f / dir1_cross_dir2;
			float diff_cross_dir1 = origin_diff.CrossProduct(ray1.dir);
			float diff_cross_dir2 = origin_diff.CrossProduct(ray2.dir);
				
			float parameter[2];
			parameter[0] = diff_cross_dir2 * inv_cross;
			parameter[1] = diff_cross_dir1 * inv_cross;
				
			// Test whether the line-line intersection is on the rays.
			if (parameter[0] >= 0.0f && parameter[1] >= 0.0f)
			{
				if (out_intersect_pos)
				{
					(*out_intersect_pos) = ray1.origin + ray1.dir * parameter[0];
				}
				
				return IT_POINT;
			}
			else
			{
				return IT_EMPTY;
			}
		}
		
		float diff_cross_dir2 = origin_diff.CrossProduct(ray2.dir);
		if (Abs(diff_cross_dir2) <= Math::ZERO_TOLERANCE)
		{
			// Lines are colinear.
			return IT_COLINEAR;
		}
		
		// Lines are parallel, but distinct.
		return IT_EMPTY;
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
