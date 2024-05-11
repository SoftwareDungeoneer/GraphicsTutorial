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
			{ "Math -- 2D Vector", UnitTests::Math::Vec2::run_all },
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
			WriteFile(outHandle, oss.str().data(), oss.str().size(), &dwWritten, nullptr);
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
		return false;
	}

	bool test_sgn_positive_double()
	{
		return false;
	}

	bool test_sgn_positive_int()
	{
		return false;
	}

	bool test_sgn_positive_uint()
	{
		return false;
	}

	bool test_sgn_negative_float()
	{
		return false;
	}

	bool test_sgn_negative_double()
	{
		return false;
	}

	bool test_sgn_negative_int()
	{
		return false;
	}

	bool test_sgn_zero_float()
	{
		return false;
	}

	bool test_sgn_zero_double()
	{
		return false;
	}

	bool test_sgn_zero_int()
	{
		return false;
	}

	bool test_sgn_zero_uint()
	{
		return false;
	}

	bool test_flt_cmp_rel_near_true()
	{
		return false;
	}

	bool test_flt_cmp_rel_near_false()
	{
		return false;
	}

	bool test_flt_cmp_rel_far_true()
	{
		return false;
	}

	bool test_flt_cmp_rel_far_false()
	{
		return false;
	}

	bool test_flt_cmp_zero()
	{
		return false;
	}

	bool test_flt_cmp_pos_neg_zero()
	{
		return false;
	}

	bool test_flt_cmp_abs_positive()
	{
		return false;
	}

	bool test_flt_cmp_abs_negative()
	{
		return false;
	}

	bool test_flt_cmp_abs_zero()
	{
		return false;
	}

	bool test_flt_cmp_abs_mixed()
	{
		return false;
	}

	bool test_flt_dist_ulp()
	{
		return false;
	}

	bool test_flt_dist_ulp_pos_neg_zero()
	{
		return false;
	}

	bool test_flt_dist_ulp_equal()
	{
		return false;
	}
}
