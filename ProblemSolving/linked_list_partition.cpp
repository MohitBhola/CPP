struct Node
{
	int val;
	struct Node* next{nullptr};
};

Node* partition(Node* pHead, int x)
{
	if (!pHead)
	{
		return nullptr;
	}
	
	Node* beg{pHead};
	
	while (pHead)
	{
		Node* ptr = pHead->next;
		if (ptr)
		{
			if (ptr->val < x)
			{
				pHead->next = ptr->next;
				ptr->next = beg;
				beg = ptr;
				continue;
			}
		}
		
		pHead = ptr;
	}
	
	return beg;
}

/*
Node* partition(Node* pHead, int x)
{
	Node* p1{nullptr};
	Node* p2{nullptr};
	Node* q1{nullptr};
	Node* q2{nullptr};
	
	while (pHead)
	{
		if (pHead->val < x)
		{
			// insert pHead in the *smaller* list
			if (p1 == nullptr)
			{
				p1 = pHead;
				p2 = pHead;
				pHead = pHead->next;
				p2->next = nullptr;
			}
			else
			{
				Node* ptr = pHead;
				pHead = pHead->next;
				ptr->next = p1;
				p1 = ptr;
			}
		}
		else
		{
			if (q1 == nullptr)
			{
				q1 = pHead;
				q2 = pHead;
				pHead = pHead->next;
				q2->next = nullptr;
			}
			else
			{
				Node* ptr = pHead;
				pHead = pHead->next;
				ptr->next = q1;
				q1 = ptr;
			}
		}
	}

	p2->next = q1;
	
	return p1;
}
*/

int main()
{
	return 0;
}
