#include "Math_UnitTests.h"

#include <Windows.h>

#include "MathUtil.h"

#include <sstream>
#include <map>
#include <string>
#include <vector>

struct TestReport 
{
	std::string desc;
	unsigned passed;
	unsigned failed;
	double time_seconds;
};

namespace UnitTests::Math
{
	using run_function = void(*)(unsigned*, unsigned*);

	void run_all(unsigned* pSucceeded, unsigned* pFailed)
	{
		std::ostringstream oss;

		const std::map<std::string, run_function> run_fns = {
			{ "Math utility functions", UnitTests::Math::Utility::run_all },
			{ "Math -- 2D Vector", UnitTests::Math::Vec2Tests::run_all },
			{ "Math -- 2x2 Matrix", UnitTests::Math::Mtx2x2Tests::run_all },
		};

		unsigned total_passed{ 0 };
		unsigned total_failed{ 0 };

		oss << "Running Math unit tests...\n";
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		double frequency = double(li.QuadPart);
		QueryPerformanceCounter(&li);
		double start = double(li.QuadPart);

		std::vector<TestReport> results;
		unsigned pass, fail;
		for (auto [desc, fn] : run_fns)
		{
			LARGE_INTEGER begin_time, end_time;
			QueryPerformanceCounter(&begin_time);

			fn(&pass, &fail);

			QueryPerformanceCounter(&end_time);
			end_time.QuadPart -= begin_time.QuadPart;
			double elapsed = end_time.QuadPart / frequency;

			results.emplace_back(desc, pass, fail, elapsed);
			total_passed += pass;
			total_failed += fail;
		}

		QueryPerformanceCounter(&li);
		double tend = double(li.QuadPart);
		double elapsed = (tend - start) / frequency;

		if (pSucceeded)
			*pSucceeded = total_passed;
		if (pFailed)
			*pFailed = total_failed;

		for (auto report : results)
		{
			oss << report.desc << "--" << "Passed: " << report.passed
				<< " Failed: " << report.failed
				<< " (" << report.time_seconds << "sec)\n";
		}
		oss << "Total time: " << elapsed << " sec\n";

		auto outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (outHandle && outHandle != INVALID_HANDLE_VALUE)
		{
			DWORD dwWritten{ 0 };
			WriteFile(
				outHandle, 
				oss.str().data(),
				static_cast<unsigned>(oss.str().size()),
				&dwWritten, 
				nullptr
			);
		}
	}
}

namespace UnitTests::Math::Utility
{
	void run_all(unsigned* pSucceeded, unsigned* pFailed)
	{
		unsigned passed{ 0 };
		unsigned failed{ 0 };

		using test_function = bool(*)();
		test_function test_fns[]{
			test_sgn_positive_float,
			test_sgn_positive_double,
			test_sgn_positive_int,
			test_sgn_positive_uint,
			test_sgn_negative_float,
			test_sgn_negative_double,
			test_sgn_negative_int,
			test_sgn_zero_float,
			test_sgn_zero_double,
			test_sgn_zero_int,
			test_sgn_zero_uint,
			test_flt_cmp_rel_near_true,
			test_flt_cmp_rel_near_false,
			test_flt_cmp_rel_far_true,
			test_flt_cmp_rel_far_false,
			test_flt_cmp_zero,
			test_flt_cmp_pos_neg_zero,
			test_flt_cmp_abs_positive,
			test_flt_cmp_abs_negative,
			test_flt_cmp_abs_zero,
			test_flt_cmp_abs_mixed,
			test_flt_dist_ulp,
			test_flt_dist_ulp_pos_neg_zero,
			test_flt_dist_ulp_equal,
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

	bool test_sgn_positive_float()
	{
		float f = +15.3f;
		return sgn(f) == 1.0f;
	}

	bool test_sgn_positive_double()
	{
		double d = 47.99;
		return sgn(d) == 1.0;
	}

	bool test_sgn_positive_int()
	{
		int i = 52;
		return sgn(i) == 1;
	}

	bool test_sgn_positive_uint()
	{
		unsigned n = 42;
		return sgn(n) == 1;
	}

	bool test_sgn_negative_float()
	{
		float f = -24.25;
		return sgn(f) == -1.f;
	}

	bool test_sgn_negative_double()
	{
		double d = -42.99;
		return sgn(d) == -1.0;
	}

	bool test_sgn_negative_int()
	{
		int i = -42;
		return sgn(i) == -1;
	}

	bool test_sgn_zero_float()
	{
		float f = 0;
		return sgn(f) == 0.f;
	}

	bool test_sgn_zero_double()
	{
		float d = 0;
		return sgn(d) == 0.0;
	}

	bool test_sgn_zero_int()
	{
		int i = 0;
		return sgn(i) == 0;
	}

	bool test_sgn_zero_uint()
	{
		unsigned n = 0;
		return sgn(n) == 0;
	}

	bool test_flt_cmp_rel_near_true()
	{
		float a = 1.0f;
		float b = 0;
		for (int i = 0; i < 10; ++i)
			b += 0.1f;
		return flt_cmp_rel(a, b);
	}

	bool test_flt_cmp_rel_near_false()
	{
		float a = 1.0f;
		float b = a + 4 * std::numeric_limits<float>::epsilon();
		return flt_cmp_rel(a, b) == false;
	}

	bool test_flt_cmp_rel_far_true()
	{
		float a = 4.0;
		float b = 4.005f;
		return flt_cmp_rel(a, b, 0.002f);
	}

	bool test_flt_cmp_rel_far_false()
	{
		float a   = 15.2f;
		float b   = 15.2155f;
		float rel =  0.001f;
		return flt_cmp_rel(a, b, rel) == false;
	}

	bool test_flt_cmp_zero()
	{
		return flt_cmp_rel(0.f, 0.f);
	}

	bool test_flt_cmp_pos_neg_zero()
	{
		return flt_cmp_rel(+0.0f, -0.0f);
	}

	bool test_flt_cmp_abs_positive()
	{
		float a = 4.0f;
		float b = 4.001f;
		return flt_cmp_abs(a, b, 0.001f);
	}

	bool test_flt_cmp_abs_negative()
	{
		float a = -16.2f;
		float b = -16.2001f;
		return flt_cmp_abs(a, b, 0.001f);
	}

	bool test_flt_cmp_abs_zero()
	{
		return flt_cmp_abs(0.0f, 0.0f);
	}

	bool test_flt_cmp_abs_mixed()
	{
		return flt_cmp_abs(+0.0f, -0.0f);
	}

	bool test_flt_dist_ulp()
	{
		unsigned ulp = flt_dist_ulp(4.0f, 4.0f + FLT_EPSILON * 4);
		return ulp == 1;
	}

	bool test_flt_dist_ulp_pos_neg_zero()
	{
		unsigned ulp = flt_dist_ulp(+0.0f, -0.0f);
		return ulp == 0;
	}

	bool test_flt_dist_ulp_equal()
	{
		float a = 42.99024f;
		unsigned ulp = flt_dist_ulp(a, a);
		return ulp == 0;
	}
}
