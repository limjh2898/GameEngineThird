
#include "Precompiled.h"

Circle::Circle(const std::vector<Vector2>& InVertices)
{
	for (const auto& it : InVertices)
	{
		Center += it;
	}
	Center = Center / InVertices.size(); // 중심 구하기

	// 반지름 구하기
	Radius = (Center -(*std::max_element(InVertices.begin(), InVertices.end(),
		[&](Vector2 const& v1, Vector2 const& v2)
		{ 	
			return (Center - v1).SizeSquared() > (Center - v2).SizeSquared();
		}))).Size();
}