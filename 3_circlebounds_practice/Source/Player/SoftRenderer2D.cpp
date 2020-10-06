
#include "Precompiled.h"
#include "SoftRenderer.h"

#include <sstream>
#include <iomanip>

using namespace CK::DD;

// �׸��� �׸���
void SoftRenderer::DrawGrid2D()
{
	// �׸��� ����
	LinearColor gridColor(LinearColor(0.8f, 0.8f, 0.8f, 0.3f));

	// ���� ���� ���
	Vector2 viewPos = _GameEngine.GetCamera().GetTransform().GetPosition();
	Vector2 extent = Vector2(_ScreenSize.X * 0.5f, _ScreenSize.Y * 0.5f);

	// ���� �ϴܿ������� ���� �׸���
	int xGridCount = _ScreenSize.X / _Grid2DUnit;
	int yGridCount = _ScreenSize.Y / _Grid2DUnit;

	// �׸��尡 ���۵Ǵ� ���ϴ� ��ǥ �� ���
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

	// ������ ����
	ScreenPoint worldOrigin = ScreenPoint::ToScreenCoordinate(_ScreenSize, -viewPos);
	_RSI->DrawFullHorizontalLine(worldOrigin.Y, LinearColor::Red);
	_RSI->DrawFullVerticalLine(worldOrigin.X, LinearColor::Green);
}

void SoftRenderer::DrawRectangle(CK::Rectangle& rect, CK::LinearColor color)
{
	_RSI->DrawLine(rect.Min, Vector2(rect.Min.X, rect.Max.Y), color);
	_RSI->DrawLine(Vector2(rect.Min.X, rect.Max.Y), rect.Max, color);
	_RSI->DrawLine(rect.Max, Vector2(rect.Max.X, rect.Min.Y), color);
	_RSI->DrawLine(Vector2(rect.Max.X, rect.Min.Y), rect.Min, color);
}
#include <chrono>

// ���� ����
void SoftRenderer::Update2D(float InDeltaSeconds)
{

	_GameEngine.GetTree().DynamicObjectTreeUpdate();
	//startPoint = std::chrono::high_resolution_clock::now();

	//_GameEngine.GetTree().Clear();

	//for (auto it = _GameEngine.GoBegin(); it != _GameEngine.GoEnd(); ++it)
	//{
	//	_GameEngine.GetTree().Insert(it->get());
	//}

	//if (curCount >= 100)
	//	curCount = 0;

	//endPoint = std::chrono::high_resolution_clock::now();

	//dynamicRefreshTime[curCount] = std::chrono::duration_cast<std::chrono::nanoseconds>(endPoint - startPoint).count();
	//curCount++;

	static float moveSpeed = 100.f;

 	InputManager input = _GameEngine.GetInputManager();

	// �÷��̾� ���� ������Ʈ�� Ʈ������
	Transform& playerTransform = _GameEngine.FindGameObject(GameEngine::PlayerKey).GetTransform();
	playerTransform.AddPosition(Vector2(input.GetXAxis(), input.GetYAxis()) * moveSpeed * InDeltaSeconds);

	// �÷��̾ ����ٴϴ� ī�޶��� Ʈ������
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

	Transform& enemyTransform = _GameEngine.FindGameObject("Enemy").GetTransform();
	enemyTransform.SetPosition(Vector2::MoveTowards(enemyTransform.GetPosition(), playerTransform.GetPosition(), 50.0f * InDeltaSeconds));
}

// ������ ����
void SoftRenderer::Render2D()
{
	// ���� �׸���
	//DrawGrid2D();

	// ī�޶��� �� ���
	Matrix3x3 viewMat = _GameEngine.GetCamera().GetViewMatrix();
	// ��ü �׸� ��ü�� ��
	size_t totalObjectCount = _GameEngine.GetGameObject().size();
	size_t culledObjectCount = 0;
	size_t culledObjectCountRect = 0;
	size_t renderingObjectCount = 0;

	// ī�޶��� ���� �� �ٿ��
	Circle cameraCircleBound(_GameEngine.GetCamera().GetCircleBound());
	//CK::Rectangle cameraRectangleBound(_GameEngine.GetCamera().GetRectangleBound());
	Vector2 temp(Vector2::One * 200.0f);
	CK::Rectangle cameraRectangleBound(-temp, temp);
	DrawRectangle(cameraRectangleBound, CK::LinearColor::Magenta);


	auto qts = _GameEngine.GetTree().nodes;
	Transform& playerTransform = _GameEngine.FindGameObject(GameEngine::PlayerKey).GetTransform();

	for (auto& it : qts)
	{
		CK::Rectangle rect = it->bound;
		rect.Max = viewMat * rect.Max;
		rect.Min = viewMat * rect.Min;
		DrawRectangle(rect, CK::LinearColor(CK::Color32(190, 190, 190,1)));
	}
	auto dd = _GameEngine.GetTree().mDynamicObjects;
	for (auto& it : dd)
	{
		CK::Rectangle rect = it.node->bound;
		rect.Max = viewMat * rect.Max;
		rect.Min = viewMat * rect.Min;
		DrawRectangle(rect, CK::LinearColor::Red);
	}


	std::vector<GameObject*> renderObjects;
	auto camPosition = _GameEngine.GetCamera().GetTransform().GetPosition();
	cameraRectangleBound.Min = (cameraRectangleBound.Min) + camPosition;
	cameraRectangleBound.Max = (cameraRectangleBound.Max) + camPosition;
	_GameEngine.GetTree().GetVerificationObjects(cameraRectangleBound, renderObjects);

	// �����ϰ� ������ ��� ���� ������Ʈ��
	for (auto it = renderObjects.begin(); it != renderObjects.end(); ++it)
	{
		GameObject& gameObject = *(*it);
		const Mesh& mesh = _GameEngine.GetMesh(gameObject.GetMeshKey());
		Transform& transform = gameObject.GetTransform();
		Matrix3x3 finalMat = viewMat * transform.GetModelingMatrix();

		size_t vertexCount = mesh._Vertices.size();
		size_t indexCount = mesh._Indices.size();
		size_t triangleCount = indexCount / 3;

		renderingObjectCount++;

		// �������� ����� ���� ���ۿ� �ε��� ���� ����
		Vector2* vertices = new Vector2[vertexCount];
		std::memcpy(vertices, &mesh._Vertices[0], sizeof(Vector2) * vertexCount);
		int* indices = new int[indexCount];
		std::memcpy(indices, &mesh._Indices[0], sizeof(int) * indexCount);

		// �� ������ ����� ����
		for (int vi = 0; vi < vertexCount; ++vi)
		{
			vertices[vi] = finalMat * vertices[vi];
		}

		// ��ȯ�� ������ �մ� �� �׸���
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
	_RSI->PushStatisticText("Culled by Quad Tree : " + std::to_string(totalObjectCount - renderObjects.size()));
	_RSI->PushStatisticText("Rendering Objects : " + std::to_string(renderObjects.size()));
	_RSI->PushStatisticText("Total Tree Node Count : " + std::to_string(qts.size()));


	float sum = 0.0f;
	for (int i = 0; i < 100; i++)
	{
		sum += dynamicRefreshTime[i];
	}

	std::ostringstream out;
	//out << std::setprecision(0) << sum / 100.0f;//_GameEngine.GetTree().GetDynamicRefreshTime();
	out << std::setprecision(0) << _GameEngine.GetTree().GetDynamicRefreshTime();
	_RSI->PushStatisticText("Dynamic Refresh Time : " + out.str() + "ns");
}

