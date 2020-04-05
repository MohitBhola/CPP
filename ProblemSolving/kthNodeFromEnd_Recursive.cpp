struct Node
{
	int val{};
	struct Node* next{nullptr};
};

Node* kthNodeFromEndCore(Node* pHead, int k, int& i)
{
	if (!pHead)
	{
		return nullptr;
	}
	
	Node* ptr = kthNodeFromEndCore(pHead->next, k, i);
	
	i += 1;
	if (i == k)
	{
		return pHead;
	}
	
	return ptr;
}

Node* kthNodeFromEnd(Node* pHead, int k)
{
	int i{};
	return kthNodeFromEndCore(pHead, k, i);
}

int main()
{
  return 0;
}
