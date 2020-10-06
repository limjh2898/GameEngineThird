#pragma once

#include <stack>

namespace CK
{
	namespace DD
	{
		class GameEngine;
	}
}

template<class Areana>
class FixedMemoryManager;
struct FixedArenaController;

typedef FixedMemoryManager<FixedArenaController> FixedMemoryBlock;

template<class Object, int capacity = 100>
class FixedMemoryPool
{
public:
	static void* operator new(size_t size)
	{
		if (!arena)
		{
			size_t arenaSize = sizeof(Object) * (capacity + 1);
			arena = std::move(std::make_unique<char[]>(arenaSize));
			fmmgr = std::move(std::make_unique<FixedMemoryBlock>((arena.get()), arenaSize));
		}
		return fmmgr->allocate(size);
	}

	static void operator delete(void* object)
	{
		if (!fmmgr)
		{
			return;
		}
		fmmgr->deallocate(object);
	}

private:
	static std::unique_ptr<char[]> arena;
	static std::unique_ptr<FixedMemoryBlock> fmmgr;
};

template<typename Object, int capacity>
std::unique_ptr<char[]> FixedMemoryPool<Object, capacity>::arena;

template<typename Object, int capacity>
std::unique_ptr<FixedMemoryBlock>  FixedMemoryPool<Object, capacity>::fmmgr;


#define MaxNodeCount 5000


class QuadTree
{
	struct Node;
	typedef CK::DD::GameObject* GameObjectPtr;
	typedef CK::DD::GameEngine* GameEnginePtr;

	// �������� �ִ� ������Ʈ ������ ���� �淮 �ڷ�
	struct ObjectNodeSet
	{
		GameObjectPtr object;
		Node* node;
	};

private:
	/* ���
	   nw(0)  ne(1)

	   sw(2)  se(3)  */
	struct Node : FixedMemoryPool<Node, MaxNodeCount>
	{
		int nodeLevel = 0;
		bool isLeaf = true;
		Node* parentNode = nullptr;
		Node* nodes[4] = { nullptr, };
		CK::Rectangle bound;
		std::list<GameObjectPtr> gameObjects;
	};

public:
	QuadTree(GameEnginePtr ownerEngine, CK::Rectangle rootBound, int maxNodeDepth = 3);
	~QuadTree();

	// ���� ������Ʈ ����
	void Insert(GameObjectPtr go);

	// �����̴� ������Ʈ ������Ʈ�� ���� �Լ�
	void DynamicObjectTreeUpdate();

	// ��Ʈ ��带 ������ ��� ��� ����
	void Clear();

	// Ư�� ������ ������Ʈ�� ��������
	bool GetVerificationObjects(const CK::Rectangle& bound, std::vector<GameObjectPtr>& verOb);

	float GetDynamicRefreshTime();

	// �׸��� ���� �ӽ÷� �ۺ����� ����
	std::list<Node*> nodes;
	std::list<ObjectNodeSet> mDynamicObjects;

private:
	bool Compress(Node* node, GameObjectPtr go, CK::Rectangle& goBound);

	// �ڽ� ��� ����
	bool CreateChildNode(Node* node, GameObjectPtr go, CK::Rectangle& goBound);

	// Ʈ�� ��ȸ
	void TreeTraversal(Node* node, const CK::Rectangle& bound, std::vector<GameObjectPtr>& verOb);

	// �����̴� ������Ʈ�� ��� ���� ����
	void RefreshNodeArea(Node* node, Node* newNode);

	// �ڽ� ��� ����
	void DeleteChildNode(Node* node);

	int GetChildsNodeObjectCount(Node* node);

private:

	Node* mRootNode;

	GameEnginePtr mOwnerEngine;

	int mMaxNodeLevel;

	int curCount = 0;
	float dynamicRefreshTime[100] = { 0.0f , };
};


template<class Areana>
class FixedMemoryManager
{
public:
	template<size_t N>
	FixedMemoryManager(char(&a)[N]);
	FixedMemoryManager(char* block, size_t size);
	FixedMemoryManager(FixedMemoryManager&) = delete;

	~FixedMemoryManager() = default;

	void operator=(FixedMemoryManager&) = delete;

	void* allocate(size_t size);
	size_t block_size() const { return mArena.block_size(); }
	size_t capacity() const { return mArena.capacity(); }
	void clear();
	void deallocate(void* value);
	bool empty() const;

private:
	struct free_block
	{
		free_block* next;
	};
	free_block* mFreePtr;
	size_t mBlockSize;
	Areana mArena;
};

struct FixedArenaController
{
	template<int N>
	FixedArenaController(char(&a)[N]);
	FixedArenaController(char* block, size_t size);
	FixedArenaController(FixedArenaController&) = delete;
	~FixedArenaController() = default;

	void operator=(FixedArenaController&) = delete;

	inline void* allocate(size_t size);
	size_t block_size() const { return mBlockSize; }
	size_t capacity() const { return mBlockSize ? (mArenaSize / mBlockSize) : 0; }
	void clear() { mBlockSize = 0; }
	bool empty() const { return mBlockSize == 0; }

private:
	void* mArena;
	size_t mArenaSize;
	size_t mBlockSize;
};

template<class Areana>
template<size_t N>
inline FixedMemoryManager<Areana>::FixedMemoryManager(char(&a)[N])
	: mArena(a), mFreePtr(nullptr), mBlockSize(0)
{
}

template<class Areana>
inline FixedMemoryManager<Areana>::FixedMemoryManager(char* block, size_t size)
	: mArena(block, size), mFreePtr(nullptr), mBlockSize(0)
{

}

template<class Areana>
inline void* FixedMemoryManager<Areana>::allocate(size_t size)
{
	if (empty())
	{
		mFreePtr = reinterpret_cast<free_block*>(mArena.allocate(size));
		mBlockSize = size;

		if (empty())
			throw std::bad_alloc();
	}
	if (size != mBlockSize)
		throw std::bad_alloc();

	auto p = mFreePtr;
	mFreePtr = mFreePtr->next;
	return p;
}

template<class Areana>
inline void FixedMemoryManager<Areana>::clear()
{
	mFreePtr = nullptr;
	mArena.clear();
}

template<class Areana>
inline void FixedMemoryManager<Areana>::deallocate(void* value)
{
	if (value == nullptr)
		return;
	auto fp = reinterpret_cast<free_block*>(value);
	fp->next = mFreePtr;
	mFreePtr = fp;
}

template<class Areana>
inline bool FixedMemoryManager<Areana>::empty() const
{
	return mArena.empty();
}


/// FixedArenaController

template<int N>
inline FixedArenaController::FixedArenaController(char(&a)[N])
	: mArena(a), mArenaSize(N), mBlockSize(0)
{
}

inline FixedArenaController::FixedArenaController(char* block, size_t size)
	: mArena(block), mArenaSize(size), mBlockSize(0)
{
}

inline void* FixedArenaController::allocate(size_t size)
{
	if (!empty())
		return nullptr; // �̹� �Ʒ����� �Ҵ� �Ǿ� ����

	mBlockSize = Math::Max(size, sizeof(void*));
	size_t count = capacity();

	if (count == 0)
		return nullptr; // �Ʒ����� �뷮�� �׸� �ϳ��� ũ�⺸�� ������

	char* p;
	for (p = (char*)mArena; count > 1; --count, p += size)
	{
		*reinterpret_cast<char**>(p) = p + size;
	}
	*reinterpret_cast<char**>(p) = nullptr;
	return mArena;
}