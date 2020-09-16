
#include "Precompiled.h"

Circle::Circle(const std::vector<Vector2>& InVertices)
{
	// 위치 정보를 받아 중심과 반지름의 값을 구하는 로직 ( 직접 구현할 것 )
	for (const auto& it : InVertices)
	{
		Center += it;
	}
	Center = Center / InVertices.size();

	Radius = (*std::max_element(InVertices.begin(), InVertices.end(),
		[&](Vector2 const& v1, Vector2 const& v2)
		{ 	
			return (Center - v1).SizeSquared() > (Center - v2).SizeSquared();
		})).Size();
}