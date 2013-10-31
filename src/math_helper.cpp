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

#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <ctime>

namespace ERI {
	
#pragma mark Geometry
	
	const double Math::ZERO_TOLERANCE = 1e-06;
	const float Math::FLOAT_MAX = FLT_MAX;

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
	
	const Vector2 Vector2::ZERO;
	const Vector2 Vector2::UNIT = Vector2(1.f, 1.f);
	const Vector2 Vector2::UNIT_X = Vector2(1.f, 0.f);
	const Vector2 Vector2::UNIT_Y = Vector2(0.f, 1.f);
	
	Vector2::Vector2(const Vector3& v) : x(v.x), y(v.y)
	{
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
	
	float Vector2::GetRotateToDegree(const Vector2& target) const
	{
		Vector2 start = *this;
		Vector2 end = target;

		start.Normalize();
		end.Normalize();
		float cos_value = start.DotProduct(end);
		
		if (Abs(cos_value) > Math::ZERO_TOLERANCE)
		{
			float radian = acos(cos_value);
			if (start.CrossProduct(end) < 0) radian *= -1;
			
			return Math::ToDegree(radian);
		}
		else
		{
			return (start.CrossProduct(end) < 0) ? -90.0f : 90.0f;
		}
	}
	
	const Vector3 Vector3::ZERO;
	const Vector3 Vector3::UNIT = Vector3(1.f, 1.f, 1.f);
	const Vector3 Vector3::UNIT_X = Vector3(1.f, 0.f, 0.f);
	const Vector3 Vector3::UNIT_Y = Vector3(0.f, 1.f, 0.f);
	const Vector3 Vector3::UNIT_Z = Vector3(0.f, 0.f, 1.f);

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
	
	void Matrix4::ExtractMatrix3(Matrix3& m3) const
	{
		m3.m[TRI_00] = m[_00]; m3.m[TRI_01] = m[_01]; m3.m[TRI_02] = m[_02];
		m3.m[TRI_10] = m[_10]; m3.m[TRI_11] = m[_11]; m3.m[TRI_12] = m[_12];
		m3.m[TRI_20] = m[_20]; m3.m[TRI_21] = m[_21]; m3.m[TRI_22] = m[_22];
	}
	
	void Matrix4::ExtractTransform(Vector3& scale, Quaternion& rotate, Vector3& translate) const
	{
		ASSERT(is_affine());
		
		Matrix3 m3;
		ExtractMatrix3(m3);
		
		Matrix3 mQ;
		Vector3 vU;
		m3.QDUDecomposition(mQ, scale, vU);
		
		if (scale.x != scale.x) scale.x = 0.0f;
		if (scale.y != scale.y) scale.y = 0.0f;
		if (scale.z != scale.z) scale.z = 0.0f;
		
		rotate = Quaternion(Matrix4(mQ));
		translate = Vector3(m[_03], m[_13], m[_23]);
		
		if (rotate.x != rotate.x) rotate.x = 0.0f;
		if (rotate.y != rotate.y) rotate.y = 0.0f;
		if (rotate.z != rotate.z) rotate.z = 0.0f;
		if (rotate.w != rotate.w) rotate.w = 0.0f;

		if (translate.x != translate.x) translate.x = 0.0f;
		if (translate.y != translate.y) translate.y = 0.0f;
		if (translate.z != translate.z) translate.z = 0.0f;
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
			LOGW("Matrix has no inverse : singular matrix");
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
	
	void ExtractFrustum(const Matrix4& view_matrix, const Matrix4& projection_matrix, Plane* out_frustum)
	{
		const float* modl = &view_matrix.m[0];
		const float* proj = &projection_matrix.m[0];
		float clip[16];
	
		/* Combine the two matrices (multiply projection by modelview) */
		clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
		clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
		clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
		clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];
		
		clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
		clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
		clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
		clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];
		
		clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
		clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
		clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
		clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];
		
		clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
		clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
		clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
		clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];
		
		/* Extract the numbers for the RIGHT plane */
		out_frustum[0].normal.x = clip[ 3] - clip[ 0];
		out_frustum[0].normal.y = clip[ 7] - clip[ 4];
		out_frustum[0].normal.z = clip[11] - clip[ 8];
		out_frustum[0].d = clip[15] - clip[12];
		
		/* Normalize the result */
		out_frustum[0].d /= out_frustum[0].normal.Normalize();
		
		/* Extract the numbers for the LEFT plane */
		out_frustum[1].normal.x = clip[ 3] + clip[ 0];
		out_frustum[1].normal.y = clip[ 7] + clip[ 4];
		out_frustum[1].normal.z = clip[11] + clip[ 8];
		out_frustum[1].d = clip[15] + clip[12];
		
		/* Normalize the result */
		out_frustum[1].d /= out_frustum[1].normal.Normalize();
		
		/* Extract the BOTTOM plane */
		out_frustum[2].normal.x = clip[ 3] + clip[ 1];
		out_frustum[2].normal.y = clip[ 7] + clip[ 5];
		out_frustum[2].normal.z = clip[11] + clip[ 9];
		out_frustum[2].d = clip[15] + clip[13];
		
		/* Normalize the result */
		out_frustum[2].d /= out_frustum[2].normal.Normalize();
		
		/* Extract the TOP plane */
		out_frustum[3].normal.x = clip[ 3] - clip[ 1];
		out_frustum[3].normal.y = clip[ 7] - clip[ 5];
		out_frustum[3].normal.z = clip[11] - clip[ 9];
		out_frustum[3].d = clip[15] - clip[13];
		
		/* Normalize the result */
		out_frustum[3].d /= out_frustum[3].normal.Normalize();
		
		/* Extract the FAR plane */
		out_frustum[4].normal.x = clip[ 3] - clip[ 2];
		out_frustum[4].normal.y = clip[ 7] - clip[ 6];
		out_frustum[4].normal.z = clip[11] - clip[10];
		out_frustum[4].d = clip[15] - clip[14];
		
		/* Normalize the result */
		out_frustum[4].d /= out_frustum[4].normal.Normalize();
		
		/* Extract the NEAR plane */
		out_frustum[5].normal.x = clip[ 3] + clip[ 2];
		out_frustum[5].normal.y = clip[ 7] + clip[ 6];
		out_frustum[5].normal.z = clip[11] + clip[10];
		out_frustum[5].d = clip[15] + clip[14];
		
		/* Normalize the result */
		out_frustum[5].d /= out_frustum[5].normal.Normalize();
	}
	
#pragma mark Quaternion
	
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
	
#pragma mark Geometry
	
	void CalculateSmallestAABox2(const std::vector<Vector2> points, AABox2& out_box)
	{
		if (points.empty())
			return;
		
		size_t num = points.size();
		
		out_box.min = out_box.max = points[0];
		
		for (int i = 1; i < num; ++i)
		{
			if (points[i].x < out_box.min.x) out_box.min.x = points[i].x;
			else if (points[i].x > out_box.max.x) out_box.max.x = points[i].x;

			if (points[i].y < out_box.min.y) out_box.min.y = points[i].y;
			else if (points[i].y > out_box.max.y) out_box.max.y = points[i].y;
		}
	}
	
#pragma mark Intersection
	
	float GetPointSegment2DistanceSquared(const Vector2& point, const Segment2& segment)
	{
		Vector2 diff = point - segment.center;
		float param = segment.dir.DotProduct(diff);
		
		Vector2 closest_point;
		
		if (-segment.extent < param)
		{
			if (param < segment.extent)
				closest_point = segment.center + segment.dir * param;
			else
				closest_point = segment.end;
		}
		else
		{
			closest_point = segment.begin;
		}
		
		return (closest_point - point).LengthSquared();
	}
	
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
	
	IntersectionType CheckIntersectLineLine2(const Line2& line1, const Line2& line2,
											 Vector2* out_intersect_pos)
	{
		// The intersection of two lines is a solution to P0+s0*D0 = P1+s1*D1.
		// Rewrite this as s0*D0 - s1*D1 = P1 - P0 = Q.  If D0.Dot(Perp(D1)) = 0,
		// the lines are parallel.  Additionally, if Q.Dot(Perp(D1)) = 0, the
		// lines are the same.  If D0.Dot(Perp(D1)) is not zero, then
		//   s0 = Q.Dot(Perp(D1))/D0.Dot(Perp(D1))
		// produces the point of intersection.  Also,
		//   s1 = Q.Dot(Perp(D0))/D0.Dot(Perp(D1))
		
		Vector2 origin_diff = line2.origin - line1.origin;
		
		float dir1_cross_dir2 = line1.dir.CrossProduct(line2.dir);
		if (Abs(dir1_cross_dir2) > Math::ZERO_TOLERANCE)
		{
			// Lines intersect in a single point.
			
			if (out_intersect_pos)
			{
				float inv_cross = 1.0f / dir1_cross_dir2;
				float diff_cross_dir2 = origin_diff.CrossProduct(line2.dir);
				
				float parameter = diff_cross_dir2 * inv_cross;
				
				(*out_intersect_pos) = line1.origin + line1.dir * parameter;
			}
			
			return IT_POINT;
		}
		
		// Lines are parallel.
		origin_diff.Normalize();
		
		float diff_cross_dir2 = origin_diff.CrossProduct(line2.dir);
		if (Abs(diff_cross_dir2) <= Math::ZERO_TOLERANCE)
		{
			// Lines are colinear.
			return IT_COLINEAR;
		}
		
		// Lines are parallel, but distinct.
		return IT_EMPTY;
	}
	
	IntersectionType CheckIntersectRayRay2(const Ray2& ray1, const Ray2& ray2,
										   Vector2* out_intersect_pos)
	{
		// The intersection of two lines is a solution to P0+s0*D0 = P1+s1*D1.
		// Rewrite this as s0*D0 - s1*D1 = P1 - P0 = Q.  If D0.Dot(Perp(D1)) = 0,
		// the lines are parallel.  Additionally, if Q.Dot(Perp(D1)) = 0, the
		// lines are the same.  If D0.Dot(Perp(D1)) is not zero, then
		//   s0 = Q.Dot(Perp(D1))/D0.Dot(Perp(D1))
		// produces the point of intersection.  Also,
		//   s1 = Q.Dot(Perp(D0))/D0.Dot(Perp(D1))

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
		
		// Lines are colinear.
		origin_diff.Normalize();
		
		float diff_cross_dir2 = origin_diff.CrossProduct(ray2.dir);
		if (Abs(diff_cross_dir2) <= Math::ZERO_TOLERANCE)
		{
			return IT_COLINEAR;
		}
		
		// Lines are parallel, but distinct.
		return IT_EMPTY;
	}
	
	IntersectionType CheckIntersectSegmentSegment2(const Segment2& segment1, const Segment2& segment2,
												   float* out_intersect_percents, Vector2* out_intersect_pos)
	{
		// The intersection of two lines is a solution to P0+s0*D0 = P1+s1*D1.
		// Rewrite this as s0*D0 - s1*D1 = P1 - P0 = Q.  If D0.Dot(Perp(D1)) = 0,
		// the lines are parallel.  Additionally, if Q.Dot(Perp(D1)) = 0, the
		// lines are the same.  If D0.Dot(Perp(D1)) is not zero, then
		//   s0 = Q.Dot(Perp(D1))/D0.Dot(Perp(D1))
		// produces the point of intersection.  Also,
		//   s1 = Q.Dot(Perp(D0))/D0.Dot(Perp(D1))

		Vector2 origin_diff = segment2.center - segment1.center;
		
		float dir1_cross_dir2 = segment1.dir.CrossProduct(segment2.dir);
		if (Abs(dir1_cross_dir2) > Math::ZERO_TOLERANCE)
		{
			// Lines intersect in a single point.
			float inv_cross = 1.0f / dir1_cross_dir2;
			float diff_cross_dir1 = origin_diff.CrossProduct(segment1.dir);
			float diff_cross_dir2 = origin_diff.CrossProduct(segment2.dir);
			
			float parameter[2];
			parameter[0] = diff_cross_dir2 * inv_cross;
			parameter[1] = diff_cross_dir1 * inv_cross;

			// Test whether the line-line intersection is on the segments.
			if (Abs(parameter[0]) <= segment1.extent &&
				Abs(parameter[1]) <= segment2.extent)
			{
				if (out_intersect_percents)
				{
					out_intersect_percents[0] = (parameter[0] + segment1.extent) / (segment1.extent * 2);
					out_intersect_percents[1] = (parameter[1] + segment2.extent) / (segment2.extent * 2);
				}
				
				if (out_intersect_pos)
				{
					(*out_intersect_pos) = segment1.center + segment1.dir * parameter[0];
				}
				
				return IT_POINT;
			}
			else
			{
				return IT_EMPTY;
			}
		}
		
		// Lines are parallel.
		origin_diff.Normalize();

		float diff_cross_dir2 = origin_diff.CrossProduct(segment2.dir);
		if (Abs(diff_cross_dir2) <= Math::ZERO_TOLERANCE)
		{
			// Lines are colinear.
			return IT_SEGMENT;
		}
		
		// Lines are parallel, but distinct.
		return IT_EMPTY;
	}

	static bool Clip(float denom, float numer, float& t0, float& t1)
	{
		// Return value is 'true' if line segment intersects the current test
		// plane.  Otherwise 'false' is returned in which case the line segment
		// is entirely clipped.

		if (denom > 0.f)
		{
			if (numer > denom * t1)
			{
				return false;
			}
			if (numer > denom * t0)
			{
				t0 = numer / denom;
			}
			return true;
		}
		else if (denom < 0.f)
		{
			if (numer > denom * t0)
			{
				return false;
			}
			if (numer > denom * t1)
			{
				t1 = numer / denom;
			}
			return true;
		}
		else
		{
			return numer <= 0.f;
		}
	}

	static IntersectionType DoClipping(float t0, float t1, const Vector2& origin, const Vector2& dir, const Box2& box, bool is_solid,
									   int* out_quantity, float* out_intersect_percents, Vector2* out_intersect_poses)
	{
		IntersectionType intersection_type = IT_EMPTY;

		if (out_quantity)
			(*out_quantity) = 0;

		// Convert linear component to box coordinates.
		Vector2 diff = origin - box.center;
		Vector2 BOrigin(diff.DotProduct(box.axis[0]),
						diff.DotProduct(box.axis[1]));
		Vector2 BDirection(dir.DotProduct(box.axis[0]),
						   dir.DotProduct(box.axis[1]));

		float saveT0 = t0, saveT1 = t1;
		bool notAllClipped =
			Clip(+BDirection.x, -BOrigin.x - box.extent[0], t0, t1) &&
			Clip(-BDirection.x, +BOrigin.x - box.extent[0], t0, t1) &&
			Clip(+BDirection.y, -BOrigin.y - box.extent[1], t0, t1) &&
			Clip(-BDirection.y, +BOrigin.y - box.extent[1], t0, t1);

		if (notAllClipped && (is_solid || t0 != saveT0 || t1 != saveT1))
		{
			if (t1 > t0)
			{
				intersection_type = IT_SEGMENT;

				if (out_quantity)
					(*out_quantity) = 2;

				if (out_intersect_percents)
				{
					out_intersect_percents[0] = t0;
					out_intersect_percents[1] = t1;
				}

				if (out_intersect_poses)
				{
					out_intersect_poses[0] = origin + dir * t0;
					out_intersect_poses[1] = origin + dir * t1;
				}
			}
			else
			{
				intersection_type = IT_POINT;

				if (out_quantity)
					(*out_quantity) = 1;

				if (out_intersect_percents)
				{
					out_intersect_percents[0] = t0;
				}

				if (out_intersect_poses)
				{
					out_intersect_poses[0] = origin + dir * t0;
				}
			}
		}

		return intersection_type;
	}

	IntersectionType CheckIntersectRayBox2(const Ray2& ray, const Box2& box, bool is_solid,
										   int* out_quantity, float* out_intersect_percents, Vector2* out_intersect_poses)
	{
		return DoClipping(0.f, Math::FLOAT_MAX,
						  ray.origin, ray.dir,
						  box, is_solid,
						  out_quantity, out_intersect_percents, out_intersect_poses);
	}

	IntersectionType CheckIntersectSegmentBox2(const Segment2& segment, const Box2& box, bool is_solid,
											   int* out_quantity, float* out_intersect_percents, Vector2* out_intersect_poses)
	{
		return DoClipping(-segment.extent, segment.extent,
						  segment.center, segment.dir,
						  box, is_solid,
						  out_quantity, out_intersect_percents, out_intersect_poses);
	}

	bool IsIntersectLineCircle2(const Line2& line, const Circle& circle,
								std::vector<float>* out_intersect_length)
	{
		// Intersection of a the line P+t*D and the circle |X-C| = R.  The line
		// direction is unit length. The t value is a root to the quadratic
		// equation:
		//   0 = |t*D+P-C|^2 - R^2
		//     = t^2 + 2*Dot(D,P-C)*t + |P-C|^2-R^2
		//     = t^2 + 2*a1*t + a0
		// If two roots are returned, the order is T[0] < T[1].
		
		int intersection_count = 0;

		Vector2 diff = line.origin - circle.center;
		float a0 = diff.LengthSquared() - circle.radius * circle.radius;
		float a1 = line.dir.DotProduct(diff);
		float discr = a1 * a1 - a0;
		if (discr > Math::ZERO_TOLERANCE)
		{
			intersection_count = 2;
			if (out_intersect_length)
			{
				discr = sqrt(discr);
				out_intersect_length->clear();
				out_intersect_length->push_back(-a1 - discr);
				out_intersect_length->push_back(-a1 + discr);
			}
		}
		else if (discr < -Math::ZERO_TOLERANCE)
		{
		}
		else  // discr == 0
		{
			intersection_count = 1;
			if (out_intersect_length)
			{
				out_intersect_length->clear();
				out_intersect_length->push_back(-a1);
			}
		}

		return intersection_count != 0;
	}
	
	bool IsIntersectRayCircle2(const Ray2& ray, const Circle& circle,
							   std::vector<Vector2>* out_intersect_pos)
	{
		std::vector<float> t;
		Line2 line;
		line.origin = ray.origin;
		line.dir = ray.dir;
		bool intersects = IsIntersectLineCircle2(line, circle, &t);

		size_t intersection_count = t.size();

		if (intersects)
		{
			// Reduce root count if line-circle intersections are not on ray.
			if (intersection_count == 1)
			{
				if (t[0] < 0.0f)
				{
					intersection_count = 0;
				}
			}
			else
			{
				if (t[1] < 0.0f)
				{
					intersection_count = 0;
				}
				else if (t[0] < 0.0f)
				{
					intersection_count = 1;
					t[0] = t[1];
				}
			}
			
			if (out_intersect_pos)
			{
				for (int i = 0; i < intersection_count; ++i)
				{
					out_intersect_pos->push_back(ray.origin + ray.dir * t[i]);
				}
			}
		}

		return intersection_count > 0;
	}
	
	bool IsIntersectSegmentCircle2(const Segment2& segment, const Circle& circle,
								   std::vector<Vector2>* out_intersect_pos)
	{
		std::vector<float> t;
		Line2 line;
		line.origin = segment.center;
		line.dir = segment.dir;
		bool intersects = IsIntersectLineCircle2(line, circle, &t);
		
		size_t intersection_count = t.size();
		
		if (intersects)
		{
			// Reduce root count if line-circle intersections are not on segment.
			if (intersection_count == 1)
			{
				if (Abs(t[0]) > segment.extent)
				{
					intersection_count = 0;
				}
			}
			else
			{
				if (t[1] < -segment.extent || t[0] > segment.extent)
				{
					intersection_count = 0;
				}
				else
				{
					if (t[1] <= segment.extent)
					{
						if (t[0] < -segment.extent)
						{
							intersection_count = 1;
							t[0] = t[1];
						}
					}
					else
					{
						intersection_count = (t[0] >= -segment.extent ? 1 : 0);
					}
				}
			}
			
			if (out_intersect_pos)
			{
				for (int i = 0; i < intersection_count; ++i)
				{
					out_intersect_pos->push_back(segment.center + segment.dir * t[i]);
				}
			}
		}
		
		return intersection_count > 0;
	}

	bool IsIntersectSegmentBox2(const Segment2& segment, const Box2& box)
	{
		Vector2 diff = segment.center - box.center;

		float AWdU[2], ADdU[2], RHS;
		AWdU[0] = Abs(segment.dir.DotProduct(box.axis[0]));
		ADdU[0] = Abs(diff.DotProduct(box.axis[0]));
		RHS = box.extent[0] + segment.extent * AWdU[0];
		if (ADdU[0] > RHS)
		{
			return false;
		}

		AWdU[1] = Abs(segment.dir.DotProduct(box.axis[1]));
		ADdU[1] = Abs(diff.DotProduct(box.axis[1]));
		RHS = box.extent[1] + segment.extent * AWdU[1];
		if (ADdU[1] > RHS)
		{
			return false;
		}

		Vector2 perp(segment.dir.y, -segment.dir.x);
		float LHS = Abs(perp.DotProduct(diff));
		float part0 = Abs(perp.DotProduct(box.axis[0]));
		float part1 = Abs(perp.DotProduct(box.axis[1]));
		RHS = box.extent[0] * part0 + box.extent[1] * part1;
		return LHS <= RHS;
	}
	
	bool IsIntersectCircleCircle2(const Circle& circle1, const Circle& circle2)
	{
		float intersect_distance = circle1.radius + circle2.radius;
		return (circle1.center - circle2.center).LengthSquared() <= (intersect_distance * intersect_distance);
	}

	bool IsIntersectBoxCircle2(const Box2& box, const Circle& circle)
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
	
	bool IsIntersectAABoxCircle2(const AABox2& box, const Circle& circle)
	{
		float diff;
		float squared_distance = 0.0f;
		
		// process X
		if (circle.center.x < box.min.x)
		{
			diff = circle.center.x - box.min.x;
			squared_distance += diff * diff;
		}
		else if (circle.center.x > box.max.x)
		{
			float diff = circle.center.x - box.max.x;
			squared_distance += diff * diff;
		}

		// process Y
		if (circle.center.y < box.min.y)
		{
			diff = circle.center.y - box.min.y;
			squared_distance += diff * diff;
		}
		else if (circle.center.y > box.max.y)
		{
			float diff = circle.center.y - box.max.y;
			squared_distance += diff * diff;
		}
		
		return squared_distance <= (circle.radius * circle.radius);
	}
	
	bool IsIntersectAABoxAABox2(const AABox2& box1, const AABox2& box2)
	{
		// process X
		float min_min_diff = box1.min.x - box2.min.x;
		float min_max_diff = box1.min.x - box2.max.x;
		float max_min_diff = box1.max.x - box2.min.x;
		float max_max_diff = box1.max.x - box2.max.x;
		if (min_min_diff * min_max_diff > 0.0f &&
			max_min_diff * max_max_diff > 0.0f &&
			min_min_diff * max_min_diff > 0.0f)
			return false;
		
		// process Y
		min_min_diff = box1.min.y - box2.min.y;
		min_max_diff = box1.min.y - box2.max.y;
		max_min_diff = box1.max.y - box2.min.y;
		max_max_diff = box1.max.y - box2.max.y;
		if (min_min_diff * min_max_diff > 0.0f &&
			max_min_diff * max_max_diff > 0.0f &&
			min_min_diff * max_min_diff > 0.0f)
			return false;

		return true;
	}
	
	bool IsIntersectRayBox3(const Ray3& ray, const Box3& box)
	{
		float WdU[3], AWdU[3], DdU[3], ADdU[3], AWxDdU[3], RHS;
		
		Vector3 diff = ray.origin - box.center;
		
		WdU[0] = ray.dir.DotProduct(box.axis[0]);
		AWdU[0] = Abs(WdU[0]);
		DdU[0] = diff.DotProduct(box.axis[0]);
		ADdU[0] = Abs(DdU[0]);
		if (ADdU[0] > box.extent[0] && DdU[0] * WdU[0] >= 0.0f)
		{
			return false;
		}
		
		WdU[1] = ray.dir.DotProduct(box.axis[1]);
		AWdU[1] = Abs(WdU[1]);
		DdU[1] = diff.DotProduct(box.axis[1]);
		ADdU[1] = Abs(DdU[1]);
		if (ADdU[1] > box.extent[1] && DdU[1] * WdU[1] >= 0.0f)
		{
			return false;
		}
		
		WdU[2] = ray.dir.DotProduct(box.axis[2]);
		AWdU[2] = Abs(WdU[2]);
		DdU[2] = diff.DotProduct(box.axis[2]);
		ADdU[2] = Abs(DdU[2]);
		if (ADdU[2] > box.extent[2] && DdU[2] * WdU[2] >= 0.0f)
		{
			return false;
		}
		
		Vector3 WxD = ray.dir.CrossProduct(diff);
		
		AWxDdU[0] = Abs(WxD.DotProduct(box.axis[0]));
		RHS = box.extent[1] * AWdU[2] + box.extent[2] * AWdU[1];
		if (AWxDdU[0] > RHS)
		{
			return false;
		}
		
		AWxDdU[1] = Abs(WxD.DotProduct(box.axis[1]));
		RHS = box.extent[0] * AWdU[2] + box.extent[2] * AWdU[0];
		if (AWxDdU[1] > RHS)
		{
			return false;
		}
		
		AWxDdU[2] = Abs(WxD.DotProduct(box.axis[2]));
		RHS = box.extent[0] * AWdU[1] + box.extent[1] * AWdU[0];
		if (AWxDdU[2] > RHS)
		{
			return false;
		}
		
		return true;
	}
	
	float SphereInFrustum(const Sphere& sphere, const Plane* frustum)
	{
		float d;
		
		for (int p = 0; p < 6; ++p)
		{
			d = frustum[p].normal.DotProduct(sphere.center) + frustum[p].d;
			if (d <= -sphere.radius)
				return 0.0f;
		}
		
		return d + sphere.radius;
	}

	bool BoxInFrustum(const Box3& box, const Plane* frustum)
	{
		Vector3 vertices[8];
		box.GetVertices(vertices);
		
		for (int p = 0; p < 6; ++p)
		{
			if (frustum[p].normal.DotProduct(vertices[0]) + frustum[p].d > 0)
				continue;
			if (frustum[p].normal.DotProduct(vertices[1]) + frustum[p].d > 0)
				continue;
			if (frustum[p].normal.DotProduct(vertices[2]) + frustum[p].d > 0)
				continue;
			if (frustum[p].normal.DotProduct(vertices[3]) + frustum[p].d > 0)
				continue;
			if (frustum[p].normal.DotProduct(vertices[4]) + frustum[p].d > 0)
				continue;
			if (frustum[p].normal.DotProduct(vertices[5]) + frustum[p].d > 0)
				continue;
			if (frustum[p].normal.DotProduct(vertices[6]) + frustum[p].d > 0)
				continue;
			if (frustum[p].normal.DotProduct(vertices[7]) + frustum[p].d > 0)
				continue;
			
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
	
#pragma mark CatmullRomSpline
	
	/*
	 q(t) = 0.5 * ((2 * P1) +
	 (-P0 + P2) * t +
	 (2 * P0 - 5 * P1 + 4 * P2 - P3) * t^2 +
	 (-P0 + 3 * P1- 3 * P2 + P3) * t^3)
	 */
	
	void CatmullRomSpline::SetControlPoints(const Vector2& p0,
											const Vector2& p1,
											const Vector2& p2,
											const Vector2& p3)
	{
		f1 = p1 * 2;
		f2 = p2 - p0;
		f3 = p0 * 2 - p1 * 5 + p2 * 4 - p3;
		f4 = p1 * 3 - p0 - p2 * 3 + p3;
	}
	
	Vector2 CatmullRomSpline::GetPoint(float t)
	{
		ASSERT(t >= 0 && t <= 1.0f);
		
		return (f1 + f2 * t + f3 * (t * t) + f4 * (t * t * t)) * 0.5f;
	}
	
#pragma mark CubicBezierSpline
	
	/* usually won't pass througn p1, p2
	 q(t) = (1 - t)^3 * p0 + 3 * (1 - t)^2 * t * p1 + 3 * (1 - t) * t^2 * p2 + t^3 * p3
	 */
	
	void CubicBezierSpline::SetControlPoints(const Vector2& p0,
											 const Vector2& p1,
											 const Vector2& p2,
											 const Vector2& p3)
	{
		f1 = p0;
		f2 = p1 * 3;
		f3 = p2 * 3;
		f4 = p3;
	}
	
	Vector2 CubicBezierSpline::GetPoint(float t)
	{
		float ct = 1 - t;
		return f1 * ct * ct * ct + f2 * ct * ct * t + f3 * ct * t * t + f4 * t * t * t;
	}

#pragma mark Color
	
	Color Color::FromHSL(float h, float s, float l)
	{
		if (s == 0.f) return Color(l, l, l);
		
		float q = (l < 0.5f) ? (l * (1 + s)) : (l + s - (l * s));
		float p = 2 * l - q;
		float t[3];
		
		float inv_3 = 1.f / 3;
		float inv_6 = 1.f / 6;
		float div_2_3 = 2.f / 3;
		
		t[0] = h + inv_3;
		t[1] = h;
		t[2] = h - inv_3;
		for (int i = 0; i < 3; ++i)
		{
			if (t[i] < 0) t[i] += 1.f;
			if (t[i] > 1.f) t[i] -= 1.f;
			
			if (t[i] < inv_6) t[i] = p + ((q - p) * 6 * t[i]);
			else if (t[i] < 0.5f) t[i] = q;
			else if (t[i] < div_2_3) t[i] = p + ((q - p) * 6 * (div_2_3 - t[i]));
			else t[i] = p;
		}
		
		return Color(t[0], t[1], t[2]);
	}

	const Color Color::WHITE;
	const Color Color::BLACK = Color(0.f, 0.f, 0.f);
	const Color Color::RED = Color(1.f, 0.f, 0.f);
	const Color Color::GREEN = Color(0.f, 1.f, 0.f);
	const Color Color::BLUE = Color(0.f, 0.f, 1.f);
	const Color Color::ORANGE = Color(1.f, 0.5f, 0.f);
	const Color Color::YELLOW = Color(1.f, 1.f, 0.f);
	const Color Color::CYAN = Color(0.f, 1.f, 1.f);
	const Color Color::PURPLE = Color(1.f, 0.f, 1.f);

}
