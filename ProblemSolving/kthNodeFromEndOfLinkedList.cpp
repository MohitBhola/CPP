#include <iostream>
#include <sstream>
#include <string>
using namespace std;

/*

WAP (recursive) to print the kth node from the end of a LinkedList

*/

struct Node
{
    int val{};
    Node* next{nullptr};
    
    Node() = default;
    Node(int v) : val(v) {}
};

Node* getKthNodeFromEndCore(Node* p, int k, int& count)
{
    if (!p) return nullptr;
    
    Node* result = getKthNodeFromEndCore(p->next, k, count);
    
    if (++count == k)
    {
        result = p;
    }
    
    return result;
}

Node* getKthNodeFromEnd(Node* p, int k)
{
    int count{};
    return getKthNodeFromEndCore(p, k, count);
}

Node* makeLinkedList()
{
    Node* head = new Node(1);
    
    Node* pCurr = head; 
    for (int i = 2; i <= 10; ++i)
    {
        Node* p = new Node(i);
        pCurr->next = p;
        pCurr = p;
    }
    
    return head;
}

string printNode(Node* p)
{
    ostringstream oss{};
    
    if (p)
    {
        oss << p->val;
    }
    else
    {
        oss << "NULL";
    }
    
    return oss.str();
}

void printLinkedList(Node* p)
{
    while (p)
    {
        cout << p->val << "->";
        p = p->next;
    }
    
    cout << "NULL" << '\n';
}

int main()
{
    Node* l = makeLinkedList();
    int k = 20;
    
    printLinkedList(l);
    cout << k << "th Node from the end: " << printNode(getKthNodeFromEnd(l, k)) << '\n';
    
    return 0;
}

/*

1->2->3->4->5->6->7->8->9->10->NULL
20th Node from the end: NULL

1->2->3->4->5->6->7->8->9->10->NULL
5th Node from the end: 6

*/

