#include "Math_UnitTests.h"

#include "MathUtil.h"
#include "Vec2.h"
#include "Mtx2x2.h"

namespace UnitTests::Math::Vec2Tests
{
	void run_all(unsigned* pSucceeded, unsigned* pFailed)
	{
		unsigned passed{ 0 };
		unsigned failed{ 0 };

		using test_function = bool(*)();
		test_function test_fns[] {
			test_scale,
			test_add,
			test_sub,
			test_dot,
			test_magnitude_squared,
			test_magnitude,
			test_normalize,
			test_oper_add_eq,
			test_oper_sub_eq,
			test_oper_scale_eq,
			test_nonmember_scale,
			test_non_member_add,
			test_non_member_sub,
			test_non_member_dot,
			test_non_member_normalize,
			test_non_member_nomral_vector,
			test_non_member_antinormal,
			test_non_member_hadamard_product,
			test_non_member_outer_product,
			test_non_member_oper_add,
			test_non_member_oper_sub,
			test_non_member_oper_scale_left,
			test_non_member_oper_scale_right,
			test_non_member_oper_dot,
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
		::Vec2 v{ 1, 2 };
		v.Scale(3.5f);
		return v == Vec2{ 3.5f, 7.f };
	}

	bool test_add() 
	{
		Vec2 u{ 1, 2 }, v{ 2, 4 };
		u.Add(v);
		return u == Vec2{ 3, 6 };
	}

	bool test_sub() 
	{
		Vec2 u{ 1, 2 }, v{ 2, 4 };
		u.Sub(v);
		return u == Vec2{ -1, -2 };
	}

	bool test_dot() 
	{
		Vec2 u{ 1.4f, 2.3f }, v{ -1.2f, 3.1f };
		return flt_cmp_rel(u.Dot(v), 5.45f);
	}

	bool test_magnitude_squared() 
	{
		Vec2 u{ 4, 3 };
		return flt_cmp_rel(u.MagnitudeSquared(), 25.0f);
	}

	bool test_magnitude() 
	{
		Vec2 u{ 4, 3 };
		return flt_cmp_rel(u.Magnitude(), 5.0f);
	}

	bool test_normalize() 
	{
		Vec2 u{ 4, 3 };
		u.Normalize();
		return u == Vec2(4 / 5.0f, 3 / 5.0f);
	}

	bool test_oper_add_eq() 
	{
		Vec2 u{ 1, 2 }, v{ 3, 2 };
		u += v;
		return u == Vec2(4, 4);
	}

	bool test_oper_sub_eq() 
	{
		Vec2 u{ 4, 2 }, v{ 3, 1 };
		u -= v;
		return u == Vec2(1, 1);
	}

	bool test_oper_scale_eq() 
	{
		Vec2 u{ 3.2f, 1.4f };
		u *= 5.0f;
		return u == Vec2(16.0f, 7.0f);
	}

	bool test_nonmember_scale() 
	{
		Vec2 u{ 3.2f, 1.4f };
		Vec2 v = Scale(5.0f, u);
		return v == Vec2(16.0f, 7.0f);
	}

	bool test_non_member_add() 
	{
		Vec2 u{ 1, 2 }, v{ 2, 1 };
		Vec2 r = Add(u, v);
		return r == Vec2(3, 3);
	}

	bool test_non_member_sub() 
	{
		Vec2 u{ 1, 2 }, v{ 2, 1 };
		Vec2 r = Sub(u, v);
		return r == Vec2(-1, 1);
	}

	bool test_non_member_dot() 
	{
		Vec2 u{ 1.4f, 2.3f }, v{ -1.2f, 3.1f };
		return flt_cmp_rel(Dot(u, v), 5.45f);
	}

	bool test_non_member_normalize() 
	{
		Vec2 u{ 4, 3 };
		return Normalize(u) == Vec2(4 / 5.0f, 3 / 5.0f);
	}

	bool test_non_member_nomral_vector() 
	{
		Vec2 u{ 4, 3 };
		auto norm = NormalVector(u);
		return norm == Vec2(-3, 4);
	}

	bool test_non_member_antinormal() 
	{
		Vec2 u{ 4, 3 };
		auto norm = Antinormal(u);
		return norm == Vec2(3, -4);
	}

	bool test_non_member_hadamard_product() 
	{
		Vec2 u{ 1.4f, 3.4f };
		Vec2 v{ 5.0f, 3.0f };
		return ComponentMultiply(u, v) == Vec2(7.0f, 10.2f);
	}

	bool test_non_member_outer_product() 
	{
		Vec2 u{ 3.0f, -2.0f };
		Mtx2x2 product = OuterProduct(u, u);
		return product == Mtx2x2{ 9, -6, -6, 4 };
	}

	bool test_non_member_oper_add() 
	{
		Vec2 u{ 1.4f, 3.4f };
		Vec2 v{ 5.0f, 3.0f };
		return (u + v) == Vec2(6.4f, 6.4f);
	}

	bool test_non_member_oper_sub() 
	{
		Vec2 u{ 3.0f, 3.0f };
		Vec2 v{ 1.0f, 2.0f };
		return (u - v) == Vec2(2.0f, 1.0f);
	}

	bool test_non_member_oper_scale_left() 
	{
		Vec2 u{ 2.f, 3.f };
		auto v = 5.f * u;
		return v == Vec2(10.f, 15.f);
	}

	bool test_non_member_oper_scale_right() 
	{
		Vec2 u{ 2.f, 3.f };
		auto v = u * 5.f;
		return v == Vec2(10.f, 15.f);
	}

	bool test_non_member_oper_dot() 
	{
		Vec2 u{ 1.4f, 2.3f }, v{ -1.2f, 3.1f };
		return flt_cmp_rel(u * v, 5.45f);
	}
}
