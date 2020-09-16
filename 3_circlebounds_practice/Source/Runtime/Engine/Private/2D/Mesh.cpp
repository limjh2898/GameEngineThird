
#include "Precompiled.h"
using namespace CK::DD;

void Mesh::CalculateBounds()
{
	_CircleBound = Circle(_Vertices);
	_Rectangle = Rectangle(_Vertices);
}