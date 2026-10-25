/*
 * Created by v1tr10l7 on 12.10.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
using System;
using System.Runtime.InteropServices;

namespace Vortex
{
	[StructLayout(LayoutKind.Sequential)]
    public struct Vector4
    {
		public static Vector4 Zero = new Vector4(0, 0, 0, 0);
		public static Vector4 One = new Vector4(1, 1, 1, 1);

		public static Vector4 Infiniity = new Vector4(float.PositiveInfinity);

        public float X;
        public float Y;
        public float Z;
        public float W;

        public Vector4(float scalar) => X = Y = Z = W = scalar;

        public Vector4(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

		public Vector4(Vector3 xyz, float w)
		{
			X = xyz.X;
			Y = xyz.Y;
			Z = xyz.Z;
			W = w;
		}

			public void Clamp(Vector4 min, Vector4 max)
		{
			X = Mathf.Clamp(X, min.X, max.X);
			Y = Mathf.Clamp(Y, min.Y, max.Y);
			Z = Mathf.Clamp(Z, min.Z, max.Z);
			W = Mathf.Clamp(W, min.W, max.W);
		}

		public float Length() => (float)Math.Sqrt(X * X + Y * Y + Z * Z + W * W);

		public Vector4 Normalized()
		{
			float length = Length();
			return length > 0.0f ? New(v => v / length) : new Vector4(X, Y, Z, W);
		}

		public void Normalize()
		{
			float length = Length();
			Apply(v => v / length);
		}
        
        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
        {
	        if (t < 0.0f) t = 0.0f;
	        if (t > 1.0f) t = 1.0f;
	        return (1.0f - t) * a + t * b;
        }

        public void Apply(Func<double, double> func)
        {
	        X = (float)func(X);
	        Y = (float)func(Y);
	        Z = (float)func(Z);
        }
        
        public Vector4 New(Func<double, double> func) => new Vector4((float)func(X), (float)func(Y), (float)func(Z), W);

        public static Vector4 operator+(Vector4 left, Vector4 right) => new Vector4(left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W);
        public static Vector4 operator-(Vector4 left, Vector4 right) => new Vector4(left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W);
        public static Vector4 operator*(Vector4 left, Vector4 right) => new Vector4(left.X * right.X, left.Y * right.Y, left.Z * right.Z, left.W * right.W);
        public static Vector4 operator *(Vector4 left, float scalar) => new Vector4(left.X * scalar, left.Y * scalar, left.Z * scalar, left.W * scalar);
        public static Vector4 operator *(float scalar, Vector4 right) => new Vector4(scalar * right.X, scalar * right.Y, scalar * right.Z, scalar * right.W);
        public static Vector4 operator/(Vector4 left, Vector4 right) => new Vector4(left.X / right.X, left.Y / right.Y, left.Z / right.Z, left.W / right.W);
        public static Vector4 operator /(Vector4 left, float scalar) => new Vector4(left.X / scalar, left.Y / scalar, left.Z / scalar, left.W / scalar);

		public override bool Equals(object? obj) => obj is Vector4 other && Equals(other);
		public bool Equals(Vector4 right) => X == right.X && Y == right.Y && Z == right.Z && W == right.W;

		public override int GetHashCode() => (X, Y, Z, W).GetHashCode();

		public static bool operator ==(Vector4 left, Vector4 right) => left.Equals(right);
		public static bool operator !=(Vector4 left, Vector4 right) => !(left == right);

		public static Vector4 Cos(Vector4 vector) => vector.New(Math.Cos);
		public static Vector4 Sin(Vector4 vector) => vector.New(Math.Sin);

		public override string ToString() => $"Vector4[{X}, {Y}, {Z}, {W}]";

		public Vector3 XYZ
		{
			get => new Vector3(X, Y, Z);
			set { X = value.X; Y = value.Y; Z = value.Z; }
		}
	}
}
