#include "B_Tree_Main.h"



/*
    <insert>
    
// 5차 B-Tree 삽입 알고리즘 (개념 정리)
--------------------------------------------------
// 탐색 단계
// - 루트에서 시작한다.
// - 삽입할 key 값과 현재 노드의 key들을 비교하면서 
//   올바른 위치를 찾아 자식 노드로 내려간다.
// - 각 노드의 key 배열은 항상 오름차순 정렬 상태를 유지해야 한다.

// Leaf 도착 후 삽입
// - leaf 노드에 도착하면 key를 삽입한다.
// - key는 정렬 순서에 맞게 삽입해야 하므로
//   기존 key들을 오른쪽으로 shift해서 공간을 만든 후 삽입한다.

// Overflow 검사
// - 삽입 후 노드에 있는 key 개수가 차수 m 이상인지 확인한다.
// - 5차 B-Tree에서는 key 최대 개수가 4이므로,
//   key가 5개가 되는 순간 overflow가 발생한다.

// Split 수행
// - overflow가 발생한 노드를 가운데 key를 기준으로 분할한다.
// - 가운데 key(3번째 key)는 부모 노드로 올라간다.
// - 왼쪽 key 2개는 왼쪽 자식 노드,
//   오른쪽 key 2개는 오른쪽 자식 노드로 나눈다.
// - 만약 부모도 overflow 상태라면 부모도 다시 split을 수행한다.
// - 이 과정은 재귀적으로 반복된다.

// Root 분할 시
// - 루트 노드에서 split이 발생하면 새로운 루트를 생성한다.
// - 가운데 key가 새로운 루트의 key가 되고,
//   분할된 두 노드가 루트의 자식으로 연결된다.
// - 이 경우 트리의 높이가 1 증가한다.

// 요약
// - 항상 leaf에 삽입
// - overflow 검사
// - split으로 균형 유지
// - 필요 시 root까지 split 발생
// - root split 시 트리 높이 증가


*/

Node* root = NULL;

Node* btree_createNode(bool leaf)
{
    Node* newNode = (Node*)malloc(sizeof(Node));

    if(newNode == NULL)
    {
        printf("[btree_createNode] Memory allocation failed\n");
        return NULL;
    }

    newNode->leaf = leaf;
    newNode->cnt_key = 0;

    for(int i = 0; i < MAX_KEY_SIZE; i++)
    {
        newNode->key[i] = 0;
        newNode->child[i] = NULL;
    }

    newNode->child[MAX_KEY_SIZE] = NULL;
    return newNode;
}


void btree_insert(Node* cur_node, int key)
{
    // 트리가 비어있는 경우
    if(root == NULL)
    {
        root = btree_createNode(true);
        root->key[0] = key;
        root->cnt_key = 1;
        return;
    }

    // 루트가 포화 상태인 경우
    if(root->cnt_key == MAX_KEY_SIZE)
    {
        Node* newRoot = btree_createNode(false);
        newRoot->child[0] = root;
        btree_splitChild(newRoot, 0);
        root = newRoot;
    }

    btree_insertNonFull(root, key);
}


// 비포화 노드에 키 삽입
void btree_insertNonFull(Node* node, int key)
{
    int i = node->cnt_key - 1;

    if (node->leaf)
    {
        // 리프 노드인 경우: key를 올바른 위치에 삽입
        while (i >= 0 && node->key[i] > key)
        {
            node->key[i + 1] = node->key[i];
            i--;
        }
        node->key[i + 1] = key;
        node->cnt_key++;
    }
    else
    {
        // 내부 노드인 경우: 적절한 자식 노드 찾기
        while (i >= 0 && node->key[i] > key)
        {
            i--;
        }
        i++;

        // 자식 노드가 포화 상태인지 확인
        if (node->child[i]->cnt_key == MAX_KEY_SIZE)
        {
            btree_splitChild(node, i);
            // 분할 후 key가 어느 쪽에 들어가야 하는지 결정
            if (key > node->key[i])
            {
                i++;
            }
        }
        btree_insertNonFull(node->child[i], key);
    }
}


/*
SplitChild 절차:
1. mid 위치 계산
2. 오른쪽 노드 생성 → mid+1 ~ 끝까지 key, child 복사
3. 왼쪽 노드 크기 줄이기
4. 부모에 mid key 삽입
5. 부모 child[idx+1]에 오른쪽 노드 연결
*/
void btree_splitChild(Node* parent_node, int idx)
{
    Node* left_node = parent_node->child[idx];
    Node* right_node = btree_createNode(left_node->leaf);

    int mid = MAX_KEY_SIZE / 2;          // 가운데 key 위치
    int midKey = left_node->key[mid];    // 부모에 올릴 key를 미리 저장

    // right_node에 mid+1 ~ 끝까지 key 복사
    right_node->cnt_key = MAX_KEY_SIZE - mid - 1;
    for (int i = 0; i < right_node->cnt_key; i++)
    {
        right_node->key[i] = left_node->key[mid + 1 + i];
    }

    // leaf가 아니면 자식 포인터도 분할
    if (!left_node->leaf)
    {
        for (int i = 0; i <= right_node->cnt_key; i++)
        {
            right_node->child[i] = left_node->child[mid + 1 + i];
        }
    }

    // left_node의 key 개수 줄이기
    left_node->cnt_key = mid;

    // 부모 child 자리 뒤로 밀기
    for (int i = parent_node->cnt_key; i > idx; i--)
    {
        parent_node->child[i + 1] = parent_node->child[i];
    }
    parent_node->child[idx + 1] = right_node;

    // 부모 key 자리 뒤로 밀기
    for (int i = parent_node->cnt_key; i > idx; i--)
    {
        parent_node->key[i] = parent_node->key[i - 1];
    }

    // 가운데 key를 부모에 삽입
    parent_node->key[idx] = midKey;
    parent_node->cnt_key++;
}


void btree_printTree(Node* root, int level)
{
    if(root == NULL)
    {
        printf("Tree is empty\n");
        return;
    }

    for(int i = 0; i < level; i++)
    {
        printf("  ");
    }
    printf("Node: ");
    for(int i = 0; i < root->cnt_key; i++)
    {
        printf("%d ", root->key[i]);
    }
    printf("\n");

    if(!root->leaf)
    {
        for(int i = 0; i <= root->cnt_key; i++)
        {
            btree_printTree(root->child[i], level + 1);
        }
    }
}

int main()
{
    printf("=== B-Tree Test ===\n");

    btree_insert(root, 10);
    btree_insert(root, 20);
    btree_insert(root, 5);
    btree_insert(root, 6);
    btree_insert(root, 12);
    btree_insert(root, 30);
    btree_insert(root, 7);
    btree_insert(root, 17);
    btree_insert(root, 15);
    btree_insert(root, 25);
    btree_insert(root, 40);
    btree_insert(root, 50);
    btree_insert(root, 60);
    btree_insert(root, 70);
    
    return 0;
}