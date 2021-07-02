#include <iostream>
using namespace std;

/*

You have two numbers represented by a LinkedList where each node contains a single digit.
The digits are stored in forward order.

WAP to add the two numbers and return the sum as a LinkedList

*/

struct Node
{
    int val{};
    Node* next{};
    
    Node() = default;
    Node(int v) : val(v) {}
};

Node* addListsCore(Node* l1, Node* l2, int& carry)
{
    if (!l1 && !l2)
    {
        return nullptr;
    }
    
    Node* l = addListsCore(l1->next, l2->next, carry);
    
    int v{};
    if (carry)
    {
        v += carry;
    }
    v += l1->val;
    v += l2->val;
    
    carry = v / 10;
    v = v % 10;
    
    Node* head = new Node(v);
    head->next = l;
    
    return head;
}

Node* padList(Node* l1, int n)
{
    Node* head = l1;
    
    for (int i = n; i >= 1; --i)
    {
        Node* p = new Node(0);
        p->next = head;
        head = p;
    }
    
    return head;
}

int length(Node* p)
{
    int len{};
    while (p)
    {
        ++len;
        p = p->next;
    }
    
    return len;
}

Node* addLists(Node* l1, Node* l2)
{
    if (!l1) return l2;
    if (!l2) return l1;
    
    int sz1 = length(l1);
    int sz2 = length(l2);
    
    if (sz1 < sz2)
    {
        l1 = padList(l1, sz2-sz1);
    }
    else if (sz2 < sz1)
    {
        l2 = padList(l2, sz1-sz2);
    }
    
    int carry{};
    Node* head = addListsCore(l1, l2, carry);
    Node* result = head;
    
    if (carry)
    {
        result = new Node(carry);
        result->next = head;
    }
    
    return result;
}

Node* makeList1()
{
    Node* head = new Node(1);
    
    Node* pCurr = head;
    for (int i = 7; i < 10; ++i)
    {
        Node* p = new Node(i);
        pCurr->next = p;
        pCurr = p;
    }
    
    return head;
}

Node* makeList2()
{
    Node* head = new Node(5);
    
    Node* pCurr = head;
    for (int i = 3; i >= 1; --i)
    {
        Node* p = new Node(i);
        pCurr->next = p;
        pCurr = p;
    }
    
    return head;
}

void printList(Node* p)
{
    while (p)
    {
        cout << p->val << "->";
        p = p->next;
    }
    
    cout << "NULL";
}

int main()
{
    Node* l1 = makeList1();
    Node* l2 = makeList2();
    
    printList(l1);
    cout << '\n';
    printList(l2);
    cout << "\n\n";
    
    printList(addLists(l1,l2));
    cout << '\n';
    
    return 0;
}

/*
1->7->8->9->NULL
5->3->2->1->NULL

7->1->1->0->NULL
*/
