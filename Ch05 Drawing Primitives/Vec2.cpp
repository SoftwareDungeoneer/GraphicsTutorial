#include "Vec2.h"

#include "mtx2x2.h"

const Vec2 Vec2::X_AXIS{ 1, 0 };
const Vec2 Vec2::Y_AXIS{ 0, 1 };

Mtx2x2 OuterProduct(const Vec2& u, const Vec2& v)
{
	return { 0, 0, 0, 0 };
}
