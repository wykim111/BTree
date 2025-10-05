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

    int mid = left_node->cnt_key / 2;   // 실제 key 개수 기준으로 가운데 선택
    int midKey = left_node->key[mid];   // 부모로 올릴 key

    // 오른쪽 노드에 mid+1 ~ cnt_key-1 까지 복사
    int j = 0;
    for (int i = mid + 1; i < left_node->cnt_key; i++)
    {
        right_node->key[j++] = left_node->key[i];
    }
    right_node->cnt_key = j;

    // leaf가 아니면 자식도 분할
    if (!left_node->leaf)
    {
        for (int i = 0; i <= right_node->cnt_key; i++)
        {
            right_node->child[i] = left_node->child[mid + 1 + i];
        }
    }

    // 왼쪽 노드 key 개수 줄이기
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

    // 부모에 가운데 key 삽입
    parent_node->key[idx] = midKey;
    parent_node->cnt_key++;
}

/*
    <삭제 로직>

1-1. 삭제할 key 탐색
     - 현재 노드 key[] 배열에서 key와 비교하며 위치(idx) 찾기
     - key[idx] == val → 현재 노드에서 삭제 가능
     - key[idx] > val  → 자식[idx]로 이동하여 재귀 호출

------------------------------------------------------------------
[2. Case 구분]

Case A. key가 현재 노드에 있고, 현재 노드가 "리프"
    - key를 단순 삭제
    - 뒤의 key들을 앞으로 shift
    - cnt_key--

Case B. key가 현재 노드에 있고, 현재 노드가 "내부 노드"
    1) 왼쪽 자식의 key 개수가 최소(MIN_KEY_SIZE) 이상
        → predecessor(왼쪽 서브트리 최댓값) 찾아 교체
        → 왼쪽 자식에서 predecessor 재귀 삭제
    2) 오른쪽 자식의 key 개수가 최소(MIN_KEY_SIZE) 이상
        → successor(오른쪽 서브트리 최솟값) 찾아 교체
        → 오른쪽 자식에서 successor 재귀 삭제
    3) 양쪽 자식 모두 최소 키만 가지고 있다면
        → 자식 두 개를 merge
        → merge된 자식에서 재귀 삭제

Case C. key가 현재 노드에 없고, 현재 노드가 "리프"
    - 트리에 없는 key → 삭제 종료

Case D. key가 현재 노드에 없고, 현재 노드가 "내부 노드"
    - 자식[idx] 방향으로 내려가서 삭제 진행
    - 단, 내려가기 전에 자식[idx]의 key 개수가 최소라면 balance 필요
        → 왼쪽/오른쪽 형제에서 빌리기(borrow)
        → 둘 다 불가능하면 merge 실행 후 진행

------------------------------------------------------------------
[3. Balance (형제에게 빌리기)]

- btree_balanceNode(parent, child_pos)
    1) 왼쪽 형제의 key 개수가 MIN_KEY_SIZE 초과
        → borrowFromLeft()
    2) 오른쪽 형제의 key 개수가 MIN_KEY_SIZE 초과
        → borrowFromRight()
    3) 둘 다 불가능하면 merge 실행

- borrowFromLeft()
    : 부모의 key를 child에 내리고, 왼쪽 형제의 마지막 key를 부모로 올린다.

- borrowFromRight()
    : 부모의 key를 child에 내리고, 오른쪽 형제의 첫 key를 부모로 올린다.

------------------------------------------------------------------
[4. Merge (형제와 병합)]

- btree_mergeNode(parent, left_index, right_index)
    1) parent->key[left_index]를 왼쪽 노드에 추가
    2) 오른쪽 노드의 key들과 child들을 왼쪽으로 이동
    3) parent에서 key[left_index]와 child[right_index] 제거
    4) cnt_key--, 오른쪽 노드는 free 처리

------------------------------------------------------------------
[5. Predecessor / Successor 찾기]

- predecessor = 왼쪽 자식으로 내려가면서 계속 오른쪽 끝 child로 탐색
- successor   = 오른쪽 자식으로 내려가면서 계속 왼쪽 끝 child로 탐색

------------------------------------------------------------------
[6. 루트 처리]

- 삭제 후 root->cnt_key == 0 인 경우
    : root가 leaf가 아니면 root = root->child[0]
    : root가 leaf이면 트리 전체가 empty
    → root free 처리


*/


// 삭제 함수 
void btree_delete(int key)
{
    if(root == NULL)
    {
        printf("Tree is empty\n");
        return;
    }
    btree_deleteFromNode(root, key);
   
    if(root->cnt_key == 0)
    {
        Node* temp_node = root;

        if(root->leaf == false)
        {
            root = temp_node->child[0];
        }
        else//root가 단말노드인 경우
        {
            root = NULL;
        }
        free(temp_node);
    }
   
}

void btree_deleteFromNode(Node* node, int key)
{
    int idx = 0;

    //삭제할 노드의 위치 탐색
    while (idx < node->cnt_key && node->key[idx] < key)
    {
        ++idx;
    }

    //case A or B : key가 현재 노드에 있는 경우
    if(idx < node->cnt_key && node->key[idx] == key)
    {
        if(node->leaf == true)// case A : key가 현재 노드에 있고, 현재 노드가 리프 노드인 경우
        {

        }
        else// case B : key가 현재 노드에 있고, 현재 노드가 내부 노드인 경우
        {

        }
    }
    else// case C or D : key가 현재 노드에 없는 경우
    {
        if(node->leaf == true)// case C : key가 현재 노드에 없고, 현재 노드가 리프 노드인 경우
        {
            printf("Key not found\n");
            return;
        }
    }
}






int btree_findPredecessor(Node* node, int key_pos)
{
    Node* cur_node = node->child[key_pos];

    //단말 노드까지 탐색
    while (!cur_node->leaf)
    {
        cur_node = cur_node->child[cur_node->cnt_key];
    }
    //단말 노드에서 마지막 key 반환
    return cur_node->key[cur_node->cnt_key - 1];
}

int btree_findSuccessor(Node* node, int key_pos)
{
    Node* cur_node = node->child[key_pos];

    //단말 노드까지 탐색
    while (!cur_node->leaf)
    {
        cur_node = cur_node->child[0];
    }
    
    //단말 노드에서 첫번째 key 반환
    return cur_node->key[0];
}


void btree_printTree(Node* root, int level)
{
    if (root == NULL)
    {
        printf("Tree is empty\n");
        return;
    }

    // 레벨과 노드 정보 출력
    for (int i = 0; i < level; i++)
    {
        printf("    ");  // 레벨 깊이만큼 들여쓰기
    }

    printf("Level %d | Keys(%d): ", level, root->cnt_key);
    for (int i = 0; i < root->cnt_key; i++)
    {
        printf("%d ", root->key[i]);
    }
    printf("\n");

    // 자식 노드 출력
    if (!root->leaf)
    {
        for (int i = 0; i <= root->cnt_key; i++)
        {
            btree_printTree(root->child[i], level + 1);
        }
    }
}


void btree_freeTree(Node* root)
{
    if (root == NULL)
    {
        return;
    }
    for (int i = 0; i <= root->cnt_key; i++)
    {
        btree_freeTree(root->child[i]);
    }
    free(root);
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

    btree_printTree(root, 0);

    
    return 0;
}