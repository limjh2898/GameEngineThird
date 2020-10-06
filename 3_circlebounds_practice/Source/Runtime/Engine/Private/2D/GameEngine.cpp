
#include "Precompiled.h"
#include <random>
using namespace CK::DD;

const std::string GameEngine::QuadMeshKey("SM_Quad");
const std::string GameEngine::PlayerKey("Player");

bool GameEngine::InsertGameObject(std::unique_ptr<GameObject> InGameObject)
{
	auto it = std::lower_bound(_GameObjects.begin(), _GameObjects.end(), InGameObject->GetName(), OrderByHash());
	if (it == _GameObjects.end())
	{
		_GameObjects.push_back(std::move(InGameObject));
		return true;
	}

	std::size_t inHash = InGameObject->GetHash();
	std::size_t targetHash = (*it->get()).GetHash();

	if (targetHash == inHash)
	{
		// Duplicated. Same Key.
		return false;
	}
	else if (targetHash < inHash)
	{
		_GameObjects.insert(it + 1, std::move(InGameObject));
	}
	else
	{
		_GameObjects.insert(it, std::move(InGameObject));
	}

	return true;
}

GameObject& GameEngine::FindGameObject(std::string InName)
{
	return *std::lower_bound(_GameObjects.begin(), _GameObjects.end(), InName, DD::OrderByHash())->get();
}

bool GameEngine::Init(const ScreenPoint& InViewportSize)
{
	_ViewportSize = InViewportSize;

	if (!_InputManager.GetXAxis || !_InputManager.GetYAxis || !_InputManager.SpacePressed)
	{
		return false;
	}

	if (!LoadResources())
	{
		return false;
	}

	if (!LoadScene())
	{
		return false;
	}

	return true;
}

bool GameEngine::LoadResources()
{
	auto quadMesh = std::make_unique<DD::Mesh>();

	float squareHalfSize = 0.5f;
	int vertexCount = 4;
	int triangleCount = 2;
	int indexCount = triangleCount * 3;

	quadMesh->_Vertices = {
		Vector2(-squareHalfSize, -squareHalfSize),
		Vector2(-squareHalfSize, squareHalfSize),
		Vector2(squareHalfSize, squareHalfSize),
		Vector2(squareHalfSize, -squareHalfSize)
	};

	quadMesh->_Indices = {
		0, 2, 1, 0, 3, 2
	};

	quadMesh->CalculateBounds();

	_Meshes.insert({ GameEngine::QuadMeshKey , std::move(quadMesh) });

	return true;
}

bool GameEngine::LoadScene()
{
	static float squareScale = 10.f;
	auto size = Vector2::One * 1400;
	quadTree = std::make_unique<QuadTree>(this, Rectangle(-size, size), 8);

	auto player = std::make_unique<DD::GameObject>(GameEngine::PlayerKey);
	player->SetMesh(GameEngine::QuadMeshKey);
	player->GetTransform().SetScale(Vector2::One * squareScale);
	player->SetColor(LinearColor::Blue);

	_GameObjects.push_back(std::move(player));

	_Camera = std::make_unique<DD::Camera>();
	_Camera->SetCameraViewSize(_ViewportSize);

	auto enemy = std::make_unique<DD::GameObject>("Enemy");
	enemy->SetMesh(GameEngine::QuadMeshKey);
	enemy->GetTransform().SetScale(Vector2::One * 20);
	enemy->SetColor(LinearColor(CK::Color32(18,79,0,1)));

	_GameObjects.push_back(std::move(enemy));

	std::mt19937 generator(0);
	std::uniform_real_distribution<float> dist(-800.f, 800.f);

	// 200개의 배경 게임 오브젝트 생성
	for (int i = 0; i < 200; ++i)
	{
		char name[64];
		std::snprintf(name, sizeof(name), "GameObject%d", i);
		auto newGo = std::make_unique<DD::GameObject>(name);
		newGo->GetTransform().SetPosition(Vector2(dist(generator), dist(generator)));
		newGo->GetTransform().SetScale(Vector2::One * squareScale);
		newGo->SetMesh(GameEngine::QuadMeshKey);
		newGo->SetStatic();
		if (!InsertGameObject(std::move(newGo)))
		{
			return false;
		}
	}

	// 쿼드 트리에 넣기
	for (auto& it : _GameObjects)
	{
		quadTree->Insert(it.get());
	}

	return true;
}
