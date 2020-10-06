#include "Precompiled.h"
#include <chrono>

QuadTree::QuadTree(GameEnginePtr ownerEngine, CK::Rectangle rootBound, int maxNodeDepth)
	: mOwnerEngine(ownerEngine), mMaxNodeLevel(maxNodeDepth)
{
	mRootNode = new Node();
	mRootNode->bound = rootBound;
	mRootNode->nodeLevel = 1;

	nodes.push_back(mRootNode);
}

QuadTree::~QuadTree()
{
	Clear();
	delete mRootNode;
}

void QuadTree::Insert(GameObjectPtr go)
{
	CK::Rectangle bound = mOwnerEngine->GetMesh(go->GetMeshKey()).GetRectangleBound();
	auto transform = go->GetTransform();
	auto modelMat = transform.GetModelingMatrix();
	bound.Max = modelMat * bound.Max;
	bound.Min = modelMat * bound.Min;

	Compress(mRootNode, go, bound);
}

void QuadTree::DynamicObjectTreeUpdate()
{
	static std::chrono::steady_clock::time_point startPoint, endPoint;

	startPoint = std::chrono::high_resolution_clock::now();

	for (auto& set : mDynamicObjects)
	{
		CK::Rectangle bound = mOwnerEngine->GetMesh(set.object->GetMeshKey()).GetRectangleBound();
		auto transform = set.object->GetTransform();
		auto modelMat = transform.GetModelingMatrix();
		bound.Max = modelMat * bound.Max;
		bound.Min = modelMat * bound.Min;

		if (set.node->bound.IsInside(bound))
		{
			auto originNode = set.node;
			set.node->gameObjects.remove(set.object);
			Compress(set.node, set.object, bound);
			RefreshNodeArea(originNode, set.node);
		}
		else// if (set.node->bound.Intersect(bound))
		{
			auto originNode = set.node;
			auto pNode = set.node->parentNode;
			while (true)
			{
				if (!pNode)
					return;

				if (pNode->bound.IsInside(bound))
				{
					break;
				}

				pNode = pNode->parentNode;
			}

			originNode->gameObjects.remove(set.object);
			pNode->gameObjects.push_back(set.object);
			set.node = pNode;

			RefreshNodeArea(originNode, set.node);
		}
	}

	endPoint = std::chrono::high_resolution_clock::now();


	if (curCount >= 100)
		curCount = 0;

	dynamicRefreshTime[curCount] = std::chrono::duration_cast<std::chrono::nanoseconds>(endPoint - startPoint).count();
	curCount++;
}

void QuadTree::Clear()
{
	nodes.clear();
	DeleteChildNode(mRootNode);
	mRootNode->isLeaf = true;
}

bool QuadTree::GetVerificationObjects(const CK::Rectangle& bound, std::vector<GameObjectPtr>& verOb)
{
	if (!mRootNode)
		return false;

	TreeTraversal(mRootNode, bound, verOb);
	return true;
}

float QuadTree::GetDynamicRefreshTime()
{
	float sum = 0.0f;
	for (int i = 0; i < 100; i++)
	{
		sum += dynamicRefreshTime[i];
	}

	return sum / 100.0f;
}

bool QuadTree::Compress(Node* node, GameObjectPtr go, CK::Rectangle& goBound)
{	
	// 영역 안에 완전히 포함되는지, 노드의 깊이를 검사
	if (node->bound.IsInside(goBound) 
		&& node->nodeLevel < mMaxNodeLevel 
		&& nodes.size()+4 <= MaxNodeCount)
	{
		int count = 0;
		// 자식 노드들을 생성
		if (CreateChildNode(node, go, goBound)) 
		{
			for (int i = 0; i < 4; i++)
			{
				if (!node->nodes[i]->bound.IsInside(goBound))
				{
					count++;
					continue;
				}
				// 자식 노드로 이관
				Compress(node->nodes[i], go, goBound);
			}
			if (count != 4)
			{
				return true;
			}
		}
	}
	
	// 추가
	node->gameObjects.push_back(go);

	if (!go->IsStatic())
	{
		auto it = std::find_if(mDynamicObjects.begin(), mDynamicObjects.end(),
			[&](ObjectNodeSet& set)
			{
				return set.object == go;
			});

		if (it == mDynamicObjects.end())
		{
			mDynamicObjects.push_back(ObjectNodeSet{ go, node });
		}
		else
		{
			(*it).node = node;
		}
	}
	return true;
}

bool QuadTree::CreateChildNode(Node* node, GameObjectPtr go, CK::Rectangle& goBound)
{
	if (!node->isLeaf)
		return true;

	//int count = 0;
	for (int i = 0; i < 4; i++)
	{
		if (node->nodes[i] != nullptr)
		{
			//count++;
			continue;
		}

		node->nodes[i] = new Node();
		node->nodes[i]->nodeLevel = node->nodeLevel + 1;
		node->nodes[i]->parentNode = node;
		nodes.push_back(node->nodes[i]);
	}

	//if (count == 4)
		//return true;

	Vector2 center;
	Vector2 extent;
	node->bound.GetCenterAndExtent(center, extent);

	node->nodes[0]->bound = CK::Rectangle(Vector2(center.X - extent.X, center.Y), Vector2(center.X, center.Y + extent.Y));
	node->nodes[1]->bound = CK::Rectangle(center, center + extent);
	node->nodes[2]->bound = CK::Rectangle(center - extent, center);
	node->nodes[3]->bound = CK::Rectangle(Vector2(center.X, center.Y - extent.Y), Vector2(center.X + extent.X, center.Y));
	
	node->isLeaf = false;

	return true;
}

void QuadTree::TreeTraversal(Node* node, const CK::Rectangle& bound, std::vector<GameObjectPtr>& verOb)
{
	if (!node->bound.Intersect(bound))
		return;

	for (auto& ob : node->gameObjects)
	{
		verOb.push_back(ob);
	}
	for (int i = 0; i < 4; i++)
	{
		if (node->nodes[i] != nullptr)
		{
			TreeTraversal(node->nodes[i], bound, verOb);
		}
	}
}

void QuadTree::RefreshNodeArea(Node* node, Node* newNode)
{
	if (node == newNode)
		return;

	size_t obCount = GetChildsNodeObjectCount(node);

	if (obCount == 0)
	{
		for (int i = 0; i < 4; i++)
		{
			if (node->nodes[i])
			{
				auto del = node->nodes[i];
				nodes.remove(node->nodes[i]);
				node->nodes[i] = nullptr;
				delete del;
			}
		}

		node->isLeaf = true;

		if (node->parentNode)
		{
			RefreshNodeArea(node->parentNode, newNode);
		}
	}
}

int QuadTree::GetChildsNodeObjectCount(Node* node)
{
	int count = node->gameObjects.size();

	if (node->isLeaf)
		return count;

	for (int i = 0; i < 4; i++)
	{
		if (!node->nodes[i])
			continue;

		count += GetChildsNodeObjectCount(node->nodes[i]);
	}

	return count;
}

void QuadTree::DeleteChildNode(Node* node)
{
	node->gameObjects.clear();
	for (int i = 0; i < 4; i++)
	{
		if (node->nodes[i] != nullptr)
		{
			auto del = node->nodes[i];
			if (del != nullptr)
			{
				DeleteChildNode(del);
			}
			delete del;
			node->nodes[i] = nullptr;
		}
	}
}