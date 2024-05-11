#ifndef GRAPHICS_TUTORIAL_MATH_H
#define GRAPHICS_TUTORIAL_MATH_H
#pragma once

namespace UnitTests::Math
{
	void run_all(unsigned* pSucceeded, unsigned* pFailed);

	// Math util tests, in Math_UnitTests.cpp
	namespace Utility
	{
		void run_all(unsigned* pSucceeded, unsigned* pFailed);

		bool test_sgn_positive_float();
		bool test_sgn_positive_double();
		bool test_sgn_positive_int();
		bool test_sgn_positive_uint();
		bool test_sgn_negative_float();
		bool test_sgn_negative_double();
		bool test_sgn_negative_int();
		bool test_sgn_zero_float();
		bool test_sgn_zero_double();
		bool test_sgn_zero_int();
		bool test_sgn_zero_uint();
		bool test_flt_cmp_rel_near_true();
		bool test_flt_cmp_rel_near_false();
		bool test_flt_cmp_rel_far_true();
		bool test_flt_cmp_rel_far_false();
		bool test_flt_cmp_zero();
		bool test_flt_cmp_pos_neg_zero();
		bool test_flt_cmp_abs_positive();
		bool test_flt_cmp_abs_negative();
		bool test_flt_cmp_abs_zero();
		bool test_flt_cmp_abs_mixed();
		bool test_flt_dist_ulp();
		bool test_flt_dist_ulp_pos_neg_zero();
		bool test_flt_dist_ulp_equal();
	}

	// Vec 2 unit tests
	namespace Vec2
	{
		void run_all(unsigned* pSucceeded, unsigned* pFailed);

		bool test_scale();
		bool test_add();
		bool test_sub();
		bool test_dot();
		bool test_magnitude_squared();
		bool test_magnitude();
		bool test_normalize();
		bool test_oper_add_eq();
		bool test_oper_sub_eq();
		bool test_oper_scale_eq();
		bool test_nonmember_scale();
		bool test_non_member_add();
		bool test_non_member_sub();
		bool test_non_member_dot();
		bool test_non_member_normalize();
		bool test_non_member_nomral_vector();
		bool test_non_member_antinormal();
		bool test_non_member_hadamard_product();
		bool test_non_member_outer_product();
		bool test_non_member_oper_add();
		bool test_non_member_oper_sub();
		bool test_non_member_oper_scale_left();
		bool test_non_member_oper_scale_right();
		bool test_non_member_oper_dot();
	}

}


#endif // GRAPHICS_TUTORIAL_MATH_H
