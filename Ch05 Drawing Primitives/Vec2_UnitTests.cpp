#include "Math_UnitTests.h"

namespace UnitTests::Math::Vec2
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
		return false;
	}

	bool test_add() 
	{
		return false;
	}

	bool test_sub() 
	{
		return false;
	}

	bool test_dot() 
	{
		return false;
	}

	bool test_magnitude_squared() 
	{
		return false;
	}

	bool test_magnitude() 
	{
		return false;
	}

	bool test_normalize() 
	{
		return false;
	}

	bool test_oper_add_eq() 
	{
		return false;
	}

	bool test_oper_sub_eq() 
	{
		return false;
	}

	bool test_oper_scale_eq() 
	{
		return false;
	}

	bool test_nonmember_scale() 
	{
		return false;
	}

	bool test_non_member_add() 
	{
		return false;
	}

	bool test_non_member_sub() 
	{
		return false;
	}

	bool test_non_member_dot() 
	{
		return false;
	}

	bool test_non_member_normalize() 
	{
		return false;
	}

	bool test_non_member_nomral_vector() 
	{
		return false;
	}

	bool test_non_member_antinormal() 
	{
		return false;
	}

	bool test_non_member_hadamard_product() 
	{
		return false;
	}

	bool test_non_member_outer_product() 
	{
		return false;
	}

	bool test_non_member_oper_add() 
	{
		return false;
	}

	bool test_non_member_oper_sub() 
	{
		return false;
	}

	bool test_non_member_oper_scale_left() 
	{
		return false;
	}

	bool test_non_member_oper_scale_right() 
	{
		return false;
	}

	bool test_non_member_oper_dot() 
	{
		return false;
	}
}
