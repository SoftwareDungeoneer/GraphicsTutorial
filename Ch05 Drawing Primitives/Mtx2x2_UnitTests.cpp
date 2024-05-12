#include "Math_UnitTests.h"

#include <numbers>

#include "Mtx2x2.h"

namespace UnitTests::Math::Mtx2x2Tests
{
	void run_all(unsigned* pSucceeded, unsigned* pFailed)
	{
		unsigned passed{ 0 };
		unsigned failed{ 0 };

		using test_function = bool(*)();
		test_function test_fns[]{
			test_scale,
			test_add,
			test_sub,
			test_normalize,
			test_transpose,
			test_determinant,
			test_inverse,
			test_rotation,
			test_accessors,
			test_vec2_op_parens,
			test_float_op_parens,
			test_oper_add_eq,
			test_oper_sub_eq,
			test_oper_scale_eq,
			test_non_member_det,
			test_non_member_scale,
			test_non_member_add,
			test_non_member_sub,
			test_non_member_mul,
			test_non_member_post_pultiply,
			test_non_member_normalize,
			test_non_member_transpose,
			test_non_member_hadamard_product,
			test_oper_add,
			test_oper_sub,
			test_oper_scale_left,
			test_oper_scale_right,
			test_oper_mul,
			test_oper_post_multiply,
			test_oper_equality,
		};

		for (test_function fn : test_fns)
		{
			if (fn())
				++passed;
			else
				++failed;
		}

		if (pSucceeded)
			*pSucceeded = passed;
		if (pFailed)
			*pFailed = failed;
	}

	bool test_scale()
	{
		Mtx2x2 mtx{
			1, 2,
			3, 4
		};
		mtx.Scale(2.0f);

		return mtx == Mtx2x2{ 2, 4, 6, 8 };
	}

	bool test_add()
	{
		Mtx2x2 A{
			1, 2,
			3, 4
		};
		Mtx2x2 B{
			-2, 3,
			4, -2
		};
		A.Add(B);

		return A == Mtx2x2{ -1, 5, 7, 2 };
	}

	bool test_sub()
	{
		Mtx2x2 A{
			1, 2,
			3, 4
		};
		Mtx2x2 B{
			-2, 3,
			4, -2
		};
		A.Sub(B);

		return A == Mtx2x2{ 3, -1, -1, 6 };
	}

	bool test_normalize()
	{
		Mtx2x2 A{
			1, 2,
			3, 4
		};

		A.Normalize();


		Mtx2x2 expected {
			1 / sqrtf(10), 2 / sqrtf(20),
			3 / sqrtf(10), 4 / sqrtf(20)
		};

		return A == expected;
	}

	bool test_transpose()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ 1, 3, 2, 4 };
		A.Transpose();
		return A == B;
	}

	bool test_determinant()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		float d = A.Det();
		return flt_cmp_rel(d, -2.0f);
	}

	bool test_inverse()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		auto Ainv = A.Inverse();
		Mtx2x2 expected = { -2, 1, 3 / 2.0f, -1 / 2.0f };
		return Ainv == expected;
	}

	bool test_rotation()
	{
		constexpr float pi = std::numbers::pi_v<float>;
		constexpr float theta = pi / 3.0f;
		auto rot = Mtx2x2::Rotation(theta);
		Mtx2x2 expected{ cos(theta), -sin(theta), sin(theta), cos(theta) };
		return rot == expected;
	}

	bool test_accessors()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		auto X = A.X(), Y = A.Y();
		return X == Vec2{ 1, 3 } && Y == Vec2{ 2, 4 };
	}

	bool test_vec2_op_parens()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		auto X = A(0), Y = A(1);
		return X == Vec2{ 1, 3 } && Y == Vec2{ 2, 4 };
	}

	bool test_float_op_parens()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		return flt_cmp_rel(A(0, 0), 1.0f)
			&& flt_cmp_rel(A(0, 1), 2.0f)
			&& flt_cmp_rel(A(1, 0), 3.0f)
			&& flt_cmp_rel(A(1, 1), 4.0f);
	}

	bool test_oper_add_eq()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ -2, 3, 4, -2 };
		A += B;
		return A == Mtx2x2{ -1, 5, 7, 2 };
	}

	bool test_oper_sub_eq()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ -2, 3, 4, -2 };
		A -= B;
		return A == Mtx2x2{ 3, -1, -1, 6 };
	}

	bool test_oper_scale_eq()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		A *= 2.5f;
		return A == Mtx2x2{ 2.5f, 5.0f, 7.5f, 10.0f };
	}

	bool test_non_member_det()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		return flt_cmp_rel(Det(A), -2.0f);
	}

	bool test_non_member_scale()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		auto S = Scale(2.5f, A);
		return S == Mtx2x2{ 2.5f, 5.0f, 7.5f, 10.0f };;
	}

	bool test_non_member_add()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ -2, 3, 4, -2 };
		Mtx2x2 S = Add(A, B);
		return S == Mtx2x2{ -1, 5, 7, 2 };
	}

	bool test_non_member_sub()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ -2, 3, 4, -2 };
		auto S = Sub(A, B);
		return S == Mtx2x2{ 3, -1, -1, 6 };
	}

	bool test_non_member_mul()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ 2, 4, 1, 1 };
		Mtx2x2 expected{ 4, 6, 10, 16 };
		auto result = Mul(A, B);
		return result == expected;
	}

	bool test_non_member_post_pultiply()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Vec2 v{ 1, 1 };
		auto vprime = Mul(A, v);
		return vprime == Vec2{ 3, 7 };
	}

	bool test_non_member_normalize()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ Normalize(A) };

		Mtx2x2 expected{
			1 / sqrtf(10), 2 / sqrtf(20),
			3 / sqrtf(10), 4 / sqrtf(20)
		};

		return B == expected;
	}

	bool test_non_member_transpose()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ Transpose(A) };
		return B == Mtx2x2{ 1, 3, 2, 4 };
	}

	bool test_non_member_hadamard_product()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ 2, 1, 2, 2 };
		return ComponentMultiply(A, B) == Mtx2x2{ 2, 2, 6, 8 };
	}

	bool test_oper_add()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ 2, 1, 2, 2 };
		return (A + B) == Mtx2x2{ 3, 3, 5, 6 };
	}

	bool test_oper_sub()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ 2, 1, 2, 2 };
		return (A - B) == Mtx2x2{ -1, 1, 1, 2 };
	}

	bool test_oper_scale_left()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B = 5 * A;
		return B == Mtx2x2{ 5, 10, 15, 20 };
	}

	bool test_oper_scale_right()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B = A * 5;
		return B == Mtx2x2{ 5, 10, 15, 20 };
	}

	bool test_oper_mul()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Mtx2x2 B{ 2, 4, 1, 1 };
		return (A * B) == Mtx2x2{ 4, 6, 10, 16 };
	}

	bool test_oper_post_multiply()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		Vec2 v{ 1, 1 };
		return (A * v) == Vec2{ 3, 7 };
	}

	bool test_oper_equality()
	{
		Mtx2x2 A{ 1, 2, 3, 4 };
		return A == A;
	}
}
