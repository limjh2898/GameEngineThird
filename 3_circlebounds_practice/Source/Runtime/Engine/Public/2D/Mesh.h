#pragma once

namespace CK
{
namespace DD
{

class Mesh
{
public:
	Mesh() {};
	~Mesh() {};

	void CalculateBounds();
	const Circle& GetCircleBound() const { return _CircleBound; }
	const Rectangle& GetRectangleBound() const { return _Rectangle; }

public:
	std::vector<Vector2> _Vertices;
	std::vector<int> _Indices;

private:
	Circle _CircleBound;
	Rectangle _Rectangle;
};

}
}

