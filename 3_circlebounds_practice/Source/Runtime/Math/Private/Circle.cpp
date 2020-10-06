
#include "Precompiled.h"

Circle::Circle(const std::vector<Vector2>& InVertices)
{
	for (const auto& it : InVertices)
	{
		Center += it;
	}
	Center = Center / InVertices.size(); // �߽� ���ϱ�

	// ������ ���ϱ�
	Radius = (Center -(*std::max_element(InVertices.begin(), InVertices.end(),
		[&](Vector2 const& v1, Vector2 const& v2)
		{ 	
			return (Center - v1).SizeSquared() > (Center - v2).SizeSquared();
		}))).Size();
}