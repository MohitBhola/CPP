#include <iostream>
#include <map>
#include <deque>

using namespace std;

struct Node
{
	int data;
	struct Node* left{nullptr};
	struct Node* right{nullptr};
	
	Node(int i) : data(i) {}
};

map<int, deque<int>> sMaps{};
int sLevel; 

void foo(Node* pRoot)
{
	if (!pRoot)
	{
		return;
	}
	
	sMaps[sLevel].push_front(pRoot->data);
	
	--sLevel;
	foo(pRoot->left);
	++sLevel;
	
	++sLevel;
	foo(pRoot->right);
	--sLevel;
}

int main()
{
	Node* pRoot = new Node(6);
	pRoot->left = new Node(9);
	pRoot->left->left = new Node(5);
	pRoot->left->left->right = new Node(0);
	pRoot->left->left->right->left = new Node(2);
	pRoot->left->left->right->left->right = new Node(3);
	
	pRoot->right = new Node(10);
	pRoot->right->left = new Node(11);
	pRoot->right->right = new Node(12);
	
	foo(pRoot);
	
	for (auto const& pair : sMaps)
	{
		auto const& deque = pair.second;
		
		auto it = deque.crbegin();
		auto end = deque.crend();
		
		while (it != end)
		{
			cout << *it << '\n';
			++it;
		}
	}
	
	return 0;
}

/*
5
2
9
0
3
6
11
10
12
*/
