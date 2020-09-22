
#include "Precompiled.h"
#include "SoftRenderer.h"
using namespace CK::DD;

// 그리드 그리기
void SoftRenderer::DrawGrid2D()
{
	// 그리드 색상
	LinearColor gridColor(LinearColor(0.8f, 0.8f, 0.8f, 0.3f));

	// 뷰의 영역 계산
	Vector2 viewPos = _GameEngine.GetCamera().GetTransform().GetPosition();
	Vector2 extent = Vector2(_ScreenSize.X * 0.5f, _ScreenSize.Y * 0.5f);

	// 좌측 하단에서부터 격자 그리기
	int xGridCount = _ScreenSize.X / _Grid2DUnit;
	int yGridCount = _ScreenSize.Y / _Grid2DUnit;

	// 그리드가 시작되는 좌하단 좌표 값 계산
	Vector2 minPos = viewPos - extent;
	Vector2 minGridPos = Vector2(ceilf(minPos.X / (float)_Grid2DUnit), ceilf(minPos.Y / (float)_Grid2DUnit)) * (float)_Grid2DUnit;
	ScreenPoint gridBottomLeft = ScreenPoint::ToScreenCoordinate(_ScreenSize, minGridPos - viewPos);

	for (int ix = 0; ix < xGridCount; ++ix)
	{
		_RSI->DrawFullVerticalLine(gridBottomLeft.X + ix * _Grid2DUnit, gridColor);
	}

	for (int iy = 0; iy < yGridCount; ++iy)
	{
		_RSI->DrawFullHorizontalLine(gridBottomLeft.Y - iy * _Grid2DUnit, gridColor);
	}

	// 월드의 원점
	ScreenPoint worldOrigin = ScreenPoint::ToScreenCoordinate(_ScreenSize, -viewPos);
	_RSI->DrawFullHorizontalLine(worldOrigin.Y, LinearColor::Red);
	_RSI->DrawFullVerticalLine(worldOrigin.X, LinearColor::Green);
}


// 게임 로직
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	static float moveSpeed = 100.f;

	InputManager input = _GameEngine.GetInputManager();

	// 플레이어 게임 오브젝트의 트랜스폼
	Transform& playerTransform = _GameEngine.FindGameObject(GameEngine::PlayerKey).GetTransform();
	playerTransform.AddPosition(Vector2(input.GetXAxis(), input.GetYAxis()) * moveSpeed * InDeltaSeconds);

	// 플레이어를 따라다니는 카메라의 트랜스폼
	static float thresholdDistance = 1.f;
	Transform& cameraTransform = _GameEngine.GetCamera().GetTransform();
	Vector2 playerPosition = playerTransform.GetPosition();
	Vector2 prevCameraPosition = cameraTransform.GetPosition();
	if ((playerPosition - prevCameraPosition).SizeSquared() < thresholdDistance * thresholdDistance)
	{
		cameraTransform.SetPosition(playerPosition);
	}
	else
	{
		static float lerpSpeed = 2.f;
		float ratio = lerpSpeed * InDeltaSeconds;
		ratio = Math::Clamp(ratio, 0.f, 1.f);
		Vector2 newCameraPosition = prevCameraPosition + (playerPosition - prevCameraPosition) * ratio;
		cameraTransform.SetPosition(newCameraPosition);
	}
}

// 렌더링 로직
void SoftRenderer::Render2D()
{
	// 격자 그리기
	DrawGrid2D();

	// 카메라의 뷰 행렬
	Matrix3x3 viewMat = _GameEngine.GetCamera().GetViewMatrix();

	// 전체 그릴 물체의 수
	size_t totalObjectCount = _GameEngine.GetGameObject().size();
	size_t culledObjectCount = 0;
	size_t culledObjectCountRect = 0;
	size_t renderingObjectCount = 0;

	// 카메라의 현재 원 바운딩
	Circle cameraCircleBound(_GameEngine.GetCamera().GetCircleBound());
	CK::Rectangle cameraRectangleBound(_GameEngine.GetCamera().GetRectangleBound());

	// 랜덤하게 생성된 모든 게임 오브젝트들
	for (auto it = _GameEngine.GoBegin(); it != _GameEngine.GoEnd(); ++it)
	{
		GameObject& gameObject = *it->get();
		const Mesh& mesh = _GameEngine.GetMesh(gameObject.GetMeshKey());
		Transform& transform = gameObject.GetTransform();
		Matrix3x3 finalMat = viewMat * transform.GetModelingMatrix();

		size_t vertexCount = mesh._Vertices.size();
		size_t indexCount = mesh._Indices.size();
		size_t triangleCount = indexCount / 3;

		// 오브젝트의 원 바운딩 볼륨
		Circle goCircleBound(mesh.GetCircleBound());
		CK::Rectangle goRectangleBound(mesh.GetRectangleBound());

		// 메시의 바운딩 볼륨 정보를 가져와서 뷰 좌표계로 변환해 비교하기 ( 스케일도 고려해 직접 구현할 것. )
		goCircleBound.Center = viewMat * goCircleBound.Center + transform.GetPosition();
		goCircleBound.Radius *= transform.GetScale().Max();

		//goRectangleBound.Min += transform.GetPosition() - transform.GetScale();
		//goRectangleBound.Max += transform.GetPosition() + transform.GetScale();

		goRectangleBound.Min += viewMat * goRectangleBound.Min + transform.GetPosition() - transform.GetScale();
		goRectangleBound.Max += viewMat * goRectangleBound.Max + transform.GetPosition() + transform.GetScale();
		
		// 두 바운딩 볼륨이 겹치지 않으면 그리기에서 제외
		if (!cameraCircleBound.Intersect(goCircleBound))
		{
			culledObjectCount++;
			continue;
		}
		// 사각 영역 
		if (!cameraRectangleBound.Intersect(goRectangleBound))
		{
			culledObjectCountRect++;
			continue;
		}

		renderingObjectCount++;

		// 렌더러가 사용할 정점 버퍼와 인덱스 버퍼 생성
		Vector2* vertices = new Vector2[vertexCount];
		std::memcpy(vertices, &mesh._Vertices[0], sizeof(Vector2) * vertexCount);
		int* indices = new int[indexCount];
		std::memcpy(indices, &mesh._Indices[0], sizeof(int) * indexCount);

		// 각 정점에 행렬을 적용
		for (int vi = 0; vi < vertexCount; ++vi)
		{
			vertices[vi] = finalMat * vertices[vi];
		}

		// 변환된 정점을 잇는 선 그리기
		for (int ti = 0; ti < triangleCount; ++ti)
		{
			int bi = ti * 3;
			_RSI->DrawLine(vertices[indices[bi]], vertices[indices[bi + 1]], gameObject.GetColor());
			_RSI->DrawLine(vertices[indices[bi]], vertices[indices[bi + 2]], gameObject.GetColor());
			_RSI->DrawLine(vertices[indices[bi + 1]], vertices[indices[bi + 2]], gameObject.GetColor());
		}

		delete[] vertices;
		delete[] indices;
	}

	_RSI->PushStatisticText("Total Objects : " + std::to_string(totalObjectCount));
	_RSI->PushStatisticText("Culled by Circle : " + std::to_string(culledObjectCount));
	_RSI->PushStatisticText("Culled by Rectangle : " + std::to_string(culledObjectCountRect));
	_RSI->PushStatisticText("Rendering Objects : " + std::to_string(renderingObjectCount));
}

