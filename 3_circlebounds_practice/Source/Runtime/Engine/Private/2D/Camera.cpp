
#include "Precompiled.h"
using namespace CK::DD;

Matrix3x3 Camera::GetViewMatrix() const
{
	return Matrix3x3(Vector3::UnitX, Vector3::UnitY, Vector3(-_Transform.GetPosition()));
}

void Camera::SetCameraViewSize(const ScreenPoint& InScreenSize)
{
	Vector2 halfSize = Vector2(InScreenSize.X, InScreenSize.Y) * 0.5f;

	_CircleBound.Center = Vector2::Zero;
	_CircleBound.Radius = halfSize.Size();

	_RectangleBound = Rectangle(-halfSize, halfSize);
}

