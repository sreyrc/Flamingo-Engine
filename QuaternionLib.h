// Custom Quaternion library

#pragma once

#include <glm/glm.hpp>

namespace QuaternionLib {

	class Quaternion {
	
	public:
		float w, x, y, z;

		Quaternion() : w(0.0f), x(0.0f), y(0.0f), z(0.0f) {}

		Quaternion(float _w, float _x, float _y, float _z)
			: w(_w), x(_x), y(_y), z(_z) {}

		Quaternion(float _w, glm::vec3 _v)
			: w(_w), x(_v.x), y(_v.y), z(_v.z) {}

		Quaternion(glm::quat _q)
			: w(_q.w), x(_q.x), y(_q.y), z(_q.z) {}

		// Add two quaternions
		Quaternion operator+(Quaternion& q) {
			return Quaternion(w + q.w, x + q.x, y + q.y, z + q.z);
		}

		// Multiply two quaternions
		Quaternion operator*(const Quaternion& q) {
			glm::vec3 q1V(x, y, z);
			glm::vec3 q2V(q.x, q.y, q.z);
			float wRes = w * q.w - glm::dot(q1V, q2V);
			glm::vec3 vRes = w * q2V + q.w * q1V + glm::cross(q1V, q2V);

			return Quaternion(wRes, vRes.x, vRes.y, vRes.z);
		}
		
		// Dot product of two quaternions
		float Dot(const Quaternion& q) {
			return w * q.w + x * q.x + y * q.y + z * q.z;
		}

		float Length() {
			return sqrtf(Dot(*this));
		}

		void Normalize() {
			float d = Length();
			x = x / d;
			y = y / d;
			z = z / d;
			w = w / d;
		}

		bool IsUnit() {
			return ((Length() - 1.0f) <= FLT_EPSILON);
		}
		
		// Create a quaternion matrix out of the current quat	
		glm::mat4 CreateMatrix() {

			glm::mat3 Result(1.0f);
			float xx = x * x;
			float yy = y * y;
			float zz = z * z;
			float xz = x * z;
			float xy = x * y;
			float yz = y * z;
			float wx = w * x;
			float wy = w * y;
			float wz = w * z;

			Result[0][0] = 1 - 2 * (yy + zz);
			Result[0][1] = 2 * (xy + wz);
			Result[0][2] = 2 * (xz - wy);

			Result[1][0] = 2 * (xy - wz);
			Result[1][1] = 1 - 2 * (xx + zz);
			Result[1][2] = 2 * (yz + wx);

			Result[2][0] = 2 * (xz + wy);
			Result[2][1] = 2 * (yz - wx);
			Result[2][2] = 1 - 2 * (xx + yy);
			return Result;
		}
	};
};
