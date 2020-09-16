#include "Precompiled.h"

Rectangle::Rectangle(const std::vector<Vector2>& InVertices)
{
	Min = Vector2(INFINITY, INFINITY);
	Max = Vector2(-INFINITY, -INFINITY);

	for (const auto& it : InVertices)
	{
		(*this) += it;
	}
}