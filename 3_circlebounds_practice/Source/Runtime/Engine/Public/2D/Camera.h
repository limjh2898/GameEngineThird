#pragma once

namespace CK
{
namespace DD
{

class Camera
{
public:
	Camera() = default;

public:
	Transform& GetTransform() { return _Transform; }
	Matrix3x3 GetViewMatrix() const;
	void SetCameraViewSize(const ScreenPoint& InScreenSize);
	const Circle& GetCircleBound() const { return _CircleBound; }
	const Rectangle& GetRectangleBound() const { return _RectangleBound; }

protected:
	Transform _Transform;
	Circle _CircleBound;
	Rectangle _RectangleBound;
};

}
}