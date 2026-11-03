#ifndef _B_TREE_MAIN_H_
#define _B_TREE_MAIN_H_


/*
[1. Insert 과정]

1-1. 삽입 위치 탐색
  - 현재 노드의 key 배열에서 삽입할 위치(pos) 찾기
	1-1-1. 삽입할 값 < 현재 key 이면 해당 위치에서 멈춤
	1-1-2. 삽입할 값 > 현재 key 이면 다음 위치로 이동

1-2. 현재 노드가 내부 노드인 경우 (leaf == false)
  - 자식 노드 insertNode 함수 호출
  - 자식 노드의 key 개수가 max_keys+1 이면 split 필요

1-3. 현재 노드가 리프 노드인 경우
  - 삽입 위치(pos) 에 key값을 한 칸씩 밀어서 삽입
  - pos 위치에 key 삽입
  - 현재 노드 key 개수가 max_keys+1 이면 split 필요

1-4. 현재 노드가 루트인 경우
  - split이 발생하면 새로운 루트 노드 생성
  - 중간 key를 새로운 루트에 삽입
  - 좌/우 자식 노드를 새 루트에 연결

1-5. 현재 노드가 루트가 아닌 경우
  - 부모 노드가 존재하므로, 중간 key를 부모에 삽입
  - 오른쪽 분할 노드를 부모의 child[pos+1]에 연결

--------------------------------------------------

[2. Split 과정]

2-1. 중간 인덱스(mid) 계산
  - mid = cnt_key / 2

2-2. 오른쪽 노드 생성
  - 현재 노드의 key[mid+1 ~ cnt_key] 를 right_node로 이동
  - key 배열의 인덱스를 0부터 시작하도록 정리
  - key 개수, cnt_key 업데이트

2-3. 자식 노드들 분할 (현재 노드가 leaf가 아닌 경우)
  - 현재 노드의 child[mid+1 ~ cnt_child] 를 right_node로 이동
  - child 인덱스 정리, cnt_child 업데이트

2-4. 중간 key를 부모 노드에 삽입
  - 루트 노드인 경우: 새로운 루트 생성
  - 부모 노드가 존재하면:
	- key 배열 위치에 중간 key 삽입
	- 오른쪽 자식(right_node)을 부모의 child[pos+1]에 연결
	- key 와 child 배열 정리

2-5. 분할 후 부모 노드, right_node를 반환하여 상위로 전달

============================================================================================================
[1. Delete 과정]

1-1. 삭제할 key 탐색
  - 현재 노드의 key 배열에서 삭제할 값(val)을 탐색
	1-1-1. key[pos] == val 이면 해당 위치 저장
	1-1-2. key[pos] > val 이면 왼쪽 자식으로 이동 (자식[pos]로)

1-2. 삭제할 key가 현재 노드에 있는 경우
  - 1-2-1. 현재 노드가 리프 노드인 경우
	- key[pos] 를 삭제하고 키들을 한 칸씩 앞으로 이동하여 덮어쓰기
	- cnt_key--

  - 1-2-2. 현재 노드가 내부 노드인 경우
	- 왼쪽 자식(cnt_key >= min_keys)이 충분하면 predecessor 찾기
	- 오른쪽 자식(cnt_key >= min_keys)이 충분하면 successor 찾기
	- 둘 다 부족하면 자식 노드 병합 후 삭제 진행

1-3. 삭제할 key가 현재 노드에 **없는 경우**
  - 리프 노드인 경우: 종료 (삭제할 값 없음)
  - 내부 노드인 경우: 자식[pos]로 재귀 호출
	- 단, 자식 노드의 key 개수가 min_keys 이하이면 balance 필요

--------------------------------------------------

[2. balanceNode - 부족한 자식 노드의 key 개수를 보충하는 함수]

2-1. child_pos == 0 (왼쪽 끝)
  - 오른쪽 형제에서 키를 빌려와서 borrowFromRight()
  - 불가능하면 merge(node, child_pos+1, child_pos)

2-2. child_pos == cnt_key (오른쪽 끝)
  - 왼쪽 형제에서 빌려와서 borrowFromLeft()
  - 불가능하면 merge(node, child_pos, child_pos-1)

2-3. 중간 위치
  - 왼쪽 형제에서 빌려와서 borrowFromLeft()
  - 아니면 오른쪽 형제에서 borrowFromRight()
  - 둘 다 불가능하면 merge(node, child_pos, child_pos-1)

--------------------------------------------------

[3. mergeNode - 자식 노드 두 개를 병합하여 하나로 만드는 함수]

3-1. 왼쪽 형제를 기준으로 병합
  - 왼쪽 형제에 부모 노드의 key를 삽입
  - 오른쪽 형제의 모든 key를 왼쪽 형제에 추가
  - 오른쪽 형제의 자식 노드들도 모두 왼쪽 형제로 이동
  - 오른쪽 형제 노드 free 처리

3-2. 부모 노드의 key, child 배열에서 한 칸씩 앞으로 shift
  - cnt_key--, cnt_child-- 처리

--------------------------------------------------

[4. deleteInnerNode - 내부 노드에서 key 삭제 시 처리]

4-1. 왼쪽 자식에서 key 를 가져오는 경우
  - 왼쪽 자식이 min_keys 이상이면 predecessor 찾기
	→ 해당 key를 predecessor로 교체, 그 자식에서 재귀 삭제
  - 불가능하면 병합(mergeChildNode), 재귀 삭제 진행

4-2. 오른쪽 자식에서 key 를 가져오는 경우
  - 오른쪽 자식이 min_keys 이상이면 successor 찾기
  - 불가능하면 병합(mergeChildNode), 재귀 삭제 진행

--------------------------------------------------

[5. mergeChildNode - 내부 노드에서 자식 노드 병합]

5-1. 왼쪽 자식 기준으로 병합
  - 왼쪽 자식에 부모 key 삽입
  - 오른쪽 자식의 key, 자식 노드들을 가져오기
  - 부모 key 배열과 child 배열 정리
  - 오른쪽 자식의 key를 가져온 자식에서 다시 deleteValFromNode() 함수 호출

--------------------------------------------------

[6. delete() - 전체 호출을 담당하는 함수]

6-1. deleteValFromNode(val, root) 호출
6-2. 삭제 후 루트 노드의 key 개수가 0이면
  - 루트 노드를 root->child[0]으로 변경 (루트 변경)



*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


#define M					5 //5차 BTREE
#define MAX_KEY_SIZE		M-1	//한 노드에 저장 가능한 최대 Key size
#define MIN_KEY_SIZE		((M + 1) / 2 - 1)  // ⌈M/2⌉-1 = 2

typedef struct _Node
{
    int key[MAX_KEY_SIZE + 1];
    struct _Node* child[MAX_KEY_SIZE + 2];
    int cnt_key;
    bool leaf;
} Node;

// 함수 선언
Node* btree_createNode(bool leaf);
Node* btree_search(Node* root, int key);
void btree_insert(Node* cur_node, int key);
void btree_insertNonFull(Node* node, int key);
void btree_splitChild(Node* parent, int index);
void btree_delete(int key);
void btree_deleteFromNode(Node* node, int key);
void btree_balanceNode(Node* node, int child_pos);
void btree_mergeNode(Node* parent, int left_index, int right_index);
void btree_borrowFromLeft(Node* node, int child_pos);
void btree_borrowFromRight(Node* node, int child_pos);
int btree_findPredecessor(Node* node, int key_pos);
int btree_findSuccessor(Node* node, int key_pos);
void btree_printTree(Node* root, int level);
void btree_freeTree(Node* root);

// 테스트 함수 선언
void test_insert();
void test_search();
void test_delete();
void test_edge_cases();
void test_integration();
void run_all_tests();

#endif