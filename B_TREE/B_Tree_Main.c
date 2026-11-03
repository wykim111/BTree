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

1 삭제할 key 탐색
     - 현재 노드 key[] 배열에서 key와 비교하며 위치(idx) 찾기
     - key[idx] == val → 현재 노드에서 삭제 가능
     - key[idx] > val  → 자식[idx]로 이동하여 재귀 호출

------------------------------------------------------------------
2. Case 구분

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
3. Balance (형제에게 빌리기)

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
4. Merge (형제와 병합)

- btree_mergeNode(parent, left_index, right_index)
    1) parent->key[left_index]를 왼쪽 노드에 추가
    2) 오른쪽 노드의 key들과 child들을 왼쪽으로 이동
    3) parent에서 key[left_index]와 child[right_index] 제거
    4) cnt_key--, 오른쪽 노드는 free 처리

------------------------------------------------------------------
5. Predecessor / Successor 찾기

- predecessor = 왼쪽 자식으로 내려가면서 계속 오른쪽 끝 child로 탐색
- successor   = 오른쪽 자식으로 내려가면서 계속 왼쪽 끝 child로 탐색

------------------------------------------------------------------
6. 루트 처리

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
           node->key[idx] = 0;
           
           for(int i=0;i<node->cnt_key;i++)
           {
                node->key[i] = node->key[i + 1];
           }

        }
        else// case B : key가 현재 노드에 있고, 현재 노드가 내부 노드인 경우
        {
			//왼쪽에 빌릴 수있는 상황인지
			if (node->child[idx]->cnt_key > MIN_KEY_SIZE)
			{
				int pred = btree_findPredecessor(node, idx);				
				node->key[idx] = pred;
				btree_deleteFromNode(node->child[idx], pred);

			}
			else if (node->child[idx + 1]->cnt_key > MIN_KEY_SIZE)
			{
				int succ = btree_findSuccessor(node, idx);
				node->key[idx] = succ;
				btree_deleteFromNode(node->child[idx + 1], succ);

			}
			else//왼쪽, 오른쪽 모두 빌릴 수  없으면 자식들 합치기
			{
				btree_mergeNode(node, idx, idx + 1);
				btree_deleteFromNode(node->child[idx], key);
			}
        }
    }
    else// case C or D : key가 현재 노드에 없는 경우
    {
        if(node->leaf == true)// case C : key가 현재 노드에 없고, 현재 노드가 리프 노드인 경우
        {
            printf("Key not found\n");
            return;
        }
		else //case D : 삭제할 key가 현재 노드에 없고, 내부 노드
		{
			bool is_last_child = false;

			if (idx == node->cnt_key)
			{
				is_last_child = true;
			}
			else
			{
				is_last_child = false;
			}

			//balance 수행
			if (node->child[idx]->cnt_key == MIN_KEY_SIZE)
			{
				btree_balanceNode(node, idx);
			}

			/*
				오직 merge 발생 직후 에만 true가 되며,
				병합으로 사라진 자식의 오른쪽 key를 삭제해야 할 때
				왼쪽으로 인덱스를 한 칸 이동시키기 위한 예외 처리
			*/

			if ((is_last_child == true) && (idx > node->cnt_key))
			{
				btree_deleteFromNode(node->child[idx-1],key);
			}
			else
			{
				btree_deleteFromNode(node->child[idx], key);
			}

		}


    }
}


void btree_borrowFromLeft(Node* node, int child_pos)
{
	Node* right_node = node->child[child_pos];
	Node* sibling_node = node->child[child_pos - 1];

	//right_node의 key와 child를 한 칸 뒤로 밀기
	for (int i = right_node->cnt_key - 1; i >= 0; i--)
	{
		right_node->key[i + 1] = right_node->key[i];
	}

	if (right_node->leaf == true)
	{
		for (int i = right_node->cnt_key; i >= 0; i--)
			right_node->child[i + 1] = right_node->child[i];
	}

	//부모의 key를 right_node에 저장
	right_node->key[0] = node->key[child_pos - 1];

	if (right_node->leaf == false)
	{
		right_node->child[0] = sibling_node ->child[sibling_node->cnt_key];
	}

	node->key[child_pos - 1] = sibling_node->key[sibling_node->cnt_key - 1];

	sibling_node->cnt_key--;
	right_node->cnt_key++;

}

void btree_borrowFromRight(Node* node, int child_pos)
{
	Node* left_node = node->child[child_pos];
	Node* sibling_node = node->child[child_pos + 1];

	//부모 key를 child 맨 끝에 추가
	left_node->key[left_node->cnt_key] = node->key[child_pos];

	//silbling의 첫번째 child를 child의 마지막 child로 연결(leaf가 아닌 경우)
	if (left_node->leaf == false)
	{
		left_node->child[left_node->cnt_key + 1] = sibling_node->child[0];
	}

	//sibling의 첫 key를 부모로 올림.
	node->key[child_pos] = sibling_node->key[0];

	// sibling key들과 child 포인터를 한 칸 앞으로 당김
	for (int i = 1; i < sibling_node->cnt_key; i++)
		sibling_node->key[i - 1] = sibling_node->key[i];
	if (!sibling_node->leaf)
	{
		for (int i = 1; i <= sibling_node->cnt_key; i++)
			sibling_node->child[i - 1] = sibling_node->child[i];
	}

	//갯수 조정
	sibling_node->cnt_key--;
	left_node->cnt_key++;
}

void btree_mergeNode(Node* parent, int left_index, int right_index)
{
	Node* left_node = parent->child[left_index];
	Node* right_node = parent->child[right_index];

	// 부모의 key를 왼쪽 자식으로 내려줌.
	left_node->key[left_node->cnt_key] = parent->key[left_index];

	// 오른쪽 자식의 key들을 모두 왼쪽 자식으로 복사
	for (int i = 0; i < right_node->cnt_key; i++)
	{
		left_node->key[left_node->cnt_key + 1 + i] = right_node->key[i];
	}

	// 오른쪽 자식의 child 포인터도 모두 복사(단말 노드가 아닌 경우)
	if (left_node->leaf == false)
	{
		for (int i = 0; i <= right_node->cnt_key; i++)
		{
			left_node->child[left_node->cnt_key + 1 + i] = right_node->child[i];
		}
	}

	//왼쪽 자식의 key 갯수 갱신
	left_node->cnt_key += right_node->cnt_key + 1;
	
	//부모의 key 및 child 배열 정리(빈자리 당기기)
	for (int i = left_index; i < parent->cnt_key - 1; i++)
	{
		parent->key[i] = parent->key[i + 1];
		parent->child[i + 1] = parent->child[i + 2];
	}

	parent->cnt_key--;

	//오른쪽 노드 메모리 해제
	free(right_node);

}

void btree_balanceNode(Node* node, int child_pos)
{
	if (child_pos > 0 && node->child[child_pos - 1]->cnt_key > MIN_KEY_SIZE)
	{
		btree_borrowFromLeft(node, child_pos);
	}
	else if (child_pos < node->cnt_key && node->child[child_pos+1]->cnt_key > MIN_KEY_SIZE)
	{
		btree_borrowFromRight(node, child_pos);
	}
	else	//둘다 여유가 없는 경우 merge 수행
	{
		if (child_pos < node->cnt_key)
		{
			btree_mergeNode(node, child_pos, child_pos + 1);
		}
		else
		{
			btree_mergeNode(node, child_pos-1, child_pos);
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


Node* btree_search(Node* root, int key)
{
	if (root == NULL)
	{
		return NULL;
	}

	int idx = 0;

	while (idx < root->cnt_key && key > root->key[idx])
	{
		++idx;
	}

	if (idx < root->cnt_key && root->key[idx] == key)
	{
		return root;
	}

	if (root->leaf == true)
	{
		return NULL;
	}



	return btree_search(root->child[idx], key);
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

// 테스트 함수들
void test_insert()
{
    printf("=== Insert Test ===\n");
    
    // 기본 삽입 테스트
    btree_insert(root, 10);
    btree_insert(root, 20);
    btree_insert(root, 5);
    btree_insert(root, 6);
    btree_insert(root, 12);
    
    printf("After inserting 10, 20, 5, 6, 12:\n");
    btree_printTree(root, 0);
    
    // Split 테스트 (5차 B-Tree에서 5개 이상 삽입)
    btree_insert(root, 30);
    btree_insert(root, 7);
    btree_insert(root, 17);
    btree_insert(root, 15);
    btree_insert(root, 25);
    
    printf("\nAfter inserting more keys (split test):\n");
    btree_printTree(root, 0);
}

void test_search()
{
    printf("\n=== Search Test ===\n");
    
    int test_keys[] = {10, 20, 5, 6, 12, 30, 7, 17, 15, 25, 99, -1};
    int test_size = sizeof(test_keys) / sizeof(test_keys[0]);
    
    for(int i = 0; i < test_size; i++)
    {
        Node* result = btree_search(root, test_keys[i]);
        if(result != NULL)
        {
            printf("Key %d found\n", test_keys[i]);
        }
        else
        {
            printf("Key %d not found\n", test_keys[i]);
        }
    }
}

void test_delete()
{
    printf("\n=== Delete Test ===\n");
    
    // 리프 노드에서 삭제
    printf("Deleting key 6 (leaf node):\n");
    btree_delete(6);
    btree_printTree(root, 0);
    
    // 내부 노드에서 삭제
    printf("\nDeleting key 10 (internal node):\n");
    btree_delete(10);
    btree_printTree(root, 0);
    
    // 존재하지 않는 키 삭제
    printf("\nDeleting non-existent key 99:\n");
    btree_delete(99);
}

void test_edge_cases()
{
    printf("\n=== Edge Cases Test ===\n");
    
    // 빈 트리에서 삭제
    printf("Deleting from empty tree:\n");
    Node* empty_root = NULL;
    root = empty_root;
    btree_delete(10);
    
    // 빈 트리에서 검색
    printf("Searching in empty tree:\n");
    Node* result = btree_search(root, 10);
    if(result == NULL)
    {
        printf("Search in empty tree works correctly\n");
    }
    
    // 단일 노드 삭제
    printf("\nSingle node operations:\n");
    btree_insert(root, 100);
    printf("After inserting single key 100:\n");
    btree_printTree(root, 0);
    
    btree_delete(100);
    printf("After deleting single key 100:\n");
    btree_printTree(root, 0);
}

void test_integration()
{
    printf("\n=== Integration Test ===\n");
    
    // 대량 데이터 삽입
    int keys[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45, 55, 65, 75, 85};
    int size = sizeof(keys) / sizeof(keys[0]);
    
    printf("Inserting multiple keys: ");
    for(int i = 0; i < size; i++)
    {
        printf("%d ", keys[i]);
        btree_insert(root, keys[i]);
    }
    printf("\n");
    
    printf("Final tree structure:\n");
    btree_printTree(root, 0);
    
    // 일부 키 삭제
    printf("\nDeleting some keys: 20, 40, 60, 80\n");
    btree_delete(20);
    btree_delete(40);
    btree_delete(60);
    btree_delete(80);
    
    printf("After deletions:\n");
    btree_printTree(root, 0);
}

void run_all_tests()
{
    printf("=== B-Tree Comprehensive Test Suite ===\n\n");
    
    test_insert();
    test_search();
    test_delete();
    test_edge_cases();
    test_integration();
    
    printf("\n=== All Tests Completed ===\n");
}

int main()
{
    int choice;
    printf("=== B-Tree Program ===\n");
    printf("1. Run original test\n");
    printf("2. Run comprehensive test suite\n");
    printf("Enter your choice (1 or 2): ");
    scanf("%d", &choice);
    
    if(choice == 1)
    {
        // 기존 테스트 코드
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

		// === 확장된 삽입 테스트 ===
		btree_insert(root, 2);
		btree_insert(root, 3);
		btree_insert(root, 4);
		btree_insert(root, 11);
		btree_insert(root, 13);
		btree_insert(root, 18);
		btree_insert(root, 19);
		btree_insert(root, 21);
		btree_insert(root, 54);
		btree_insert(root, 55);
		btree_insert(root, 68);
		btree_insert(root, 69);

        btree_printTree(root, 0);

        btree_freeTree(root);
    }
    else if(choice == 2)
    {
        // 새로운 테스트 스위트 실행
        run_all_tests();
        btree_freeTree(root);
    }
    else
    {
        printf("Invalid choice. Running original test...\n");
        
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

        btree_freeTree(root);
    }
    
    return 0;
}