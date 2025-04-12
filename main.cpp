#include <cctype>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <vector>

#define MAXDEGREE 4

class Record {
   public:
    std::string id;
    std::string name;
    int age;
    std::string department;

    Record(std::string id, std::string name, int age, std::string department)
        : id(id), name(name), age(age), department(department) {}
};

class BPTNode {
   public:
    bool isLeaf;
    std::vector<std::shared_ptr<BPTNode>> children;
    std::vector<std::pair<std::string, std::shared_ptr<Record>>> keys;
    std::shared_ptr<BPTNode> next;

    BPTNode(bool isLeaf) : isLeaf(isLeaf), next(nullptr) {}
};

class BPlusTree {
   private:
    int degree;
    int internalMin;
    int leafMin;
    std::shared_ptr<BPTNode> root;

    // 노드 정보 입력 인터렉션.
    // 리프여부, 키의 수와 각 키들을 입력받고,
    // 포인터의 수는 자동으로 key+1개로 간주
    std::shared_ptr<BPTNode> makeNodeWithInteraction(int nodeNum) {
        bool isLeaf = false;
        std::string temp;
        int numberOfKeys;
        int numberOfChildren;
        std::cout << "Node " << nodeNum << ":\n";
        std::cout << "Is it a leaf node? If leaf, input 'y' or 'Y', otherwise, "
                     "type any.\n";
        std::getline(std::cin, temp);
        isLeaf = temp == "Y" || temp == "y";

        std::cout << "How many keys it has?\n";
        std::getline(std::cin, temp);
        numberOfKeys = stoi(temp);
        if (numberOfKeys < 0 || numberOfKeys >= degree) {
            std::cout << "Invalid Input.\n Trees will be rollbacked.\n";
            return NULL;
        }

        std::shared_ptr<BPTNode> p = std::make_shared<BPTNode>(isLeaf);
        std::cout << "Please Input the keys in 0-indexed.\n";
        for (int i = 0; i < numberOfKeys; i++) {
            std::cout << "Key[" << i << "]:\n";
            std::string temp;
            std::getline(std::cin, temp);
            p->keys.emplace_back(temp, nullptr);
        }
        return p;
    }

    // 레코드 생성 인터렉션.
    std::shared_ptr<Record> iniTializeRecord(std::string key) {
        std::string name, age, department;
        std::cout << "\n[Record Initialization for key " << key << "]\n";
        std::cout << "Input the Informations line by line:\n";

        std::cout << "Name: ";
        std::getline(std::cin, name);

        std::cout << "Age: ";
        std::getline(std::cin, age);

        std::cout << "Department: ";
        std::getline(std::cin, department);
        return std::make_shared<Record>(key, name, stoi(age), department);
    }

   public:
    BPlusTree(int degree) {
        this->degree = degree;                                // maxdegree
        this->leafMin = (degree - 1) / 2 + (degree - 1) % 2;  // ceil(n-1/2)
        this->internalMin =
            degree / 2 + degree % 2 -
            1;  // 내부노드의 최소 키 수 = 최소포인터수-1 = ceil(n/2)-1
        this->root = nullptr;
    }

    // BFS기반 출력
    // 낮은레벨부터, 같은레벨인경우 왼쪽부터 읽도록 하였음
    void printTree() {
        if (this->root.get() == nullptr) {
            std::cout << "Tree is Empty\n";
            return;
        }
        std::cout << "==========[B+ Tree Print]==========\n";
        std::queue<std::pair<int, BPTNode*>> q;
        q.push({1, root.get()});

        int numberOfNodes = 1;
        while (!q.empty()) {
            auto currNode = q.front();
            int nodeNum = currNode.first;
            BPTNode* nodePtr = currNode.second;
            q.pop();
            std::cout << "================[Node " << nodeNum
                      << "]================\n";
            std::cout << "Number of keys: " << nodePtr->keys.size() << "\n";
            for (int i = 0; i < nodePtr->keys.size(); i++) {
                std::cout << "[key " << i << "]: " << nodePtr->keys[i].first
                          << "\n";
            }
            if (nodePtr->isLeaf) {
                std::cout << "This is Leaf Node\n";
            } else {
                std::cout << "Number of children: " << nodePtr->children.size()
                          << "\n";
                for (int i = 0; i < nodePtr->children.size(); i++) {
                    std::cout << "[child " << i << "]: " << ++numberOfNodes
                              << "\n";
                    q.push({numberOfNodes, nodePtr->children[i].get()});
                }
            }
        }
        std::cout << "\n";
    }

    // 인터렉션 기반 트리 생성
    // 유효한 B+트리를 입력받아야 함
    void initializeWithInteraction() {
        std::cout << "\n[Initialize the B+Tree]\n";
        std::cout << "Input node data in full binary-tree order, starting from "
                     "(root:1)\n";

        std::queue<
            std::tuple<int, std::shared_ptr<BPTNode>, std::shared_ptr<BPTNode>>>
            q;
        std::shared_ptr<BPTNode> newRoot = makeNodeWithInteraction(1);

        if (newRoot == nullptr) return;
        q.emplace(1, nullptr, newRoot);

        int maxNodeNum = 1;
        while (!q.empty()) {
            int nodeNum;
            std::shared_ptr<BPTNode> parent, current;
            tie(nodeNum, parent, current) = q.front();
            q.pop();

            // 부모와 연결 설정
            if (parent) parent->children.emplace_back(current);

            if (current->isLeaf) {
                current->next = !q.empty() ? std::get<2>(q.front()) : nullptr;
                for (int i = 0; i < current->keys.size(); i++) {
                    current->keys[i].second =
                        iniTializeRecord(current->keys[i].first);
                }
                continue;
            }
            // 내부노드면 자식생성 인터렉션
            // key+1개 의 자식을 만듬
            for (int i = 0; i <= current->keys.size(); i++) {
                std::shared_ptr<BPTNode> p =
                    makeNodeWithInteraction(++maxNodeNum);
                if (p == nullptr) return;
                q.emplace(maxNodeNum, current, p);
            }
        }

        root = newRoot;
    }

    // 검색
    std::shared_ptr<Record> search(const std::string query) {
        if (this->root == nullptr) return nullptr;
        std::shared_ptr<Record> res;
        BPTNode* p = this->root.get();
        while (!p->isLeaf) {
            int i = 0;
            while (i < p->keys.size() && query >= p->keys[i].first) {
                i++;
            }
            p = p->children[i].get();
        }

        for (int i = 0; i < p->keys.size(); i++) {
            if (query == p->keys[i].first) {
                return p->keys[i].second;
            }
        }

        return nullptr;
    }

    // 삽입
    bool insert(const std::string key, std::string name, int age,
                std::string department) {
        // 트리가 비어있다면, 트리를 생성
        // 처음만들어진 루트는 리프임.
        if (root.get() == nullptr) {
            root = std::make_shared<BPTNode>(true);
            root->keys.emplace_back(
                key, std::make_shared<Record>(key, name, age, department));
            return true;
        }

        // 삽입
        // 스택을 타고가면서, 역대 조상들을 저장해둠.
        // 이를 이용해서 추루 오버플로우 처리를 재귀적으로 돌아오면서 처리가능
        std::shared_ptr<BPTNode> p = root;
        std::stack<std::shared_ptr<BPTNode>> ancestors;
        while (!p->isLeaf) {
            int i = 0;
            ancestors.push(p);
            while (i < p->keys.size() && key > p->keys[i].first) {
                i++;
            }
            if (i < p->keys.size() && key == p->keys[i].first) return false;
            p = p->children[i];
        }
        bool inserted = false;
        for (int i = 0; i < p->keys.size(); i++) {
            if (key < p->keys[i].first) {
                p->keys.insert(p->keys.begin() + i,
                               {key, std::make_shared<Record>(key, name, age,
                                                              department)});
                inserted = true;
                break;
            } else if (key == p->keys[i].first)
                return false;  // 이미 값이 있으면 삽입 실패
        }
        if (!inserted)
            p->keys.emplace_back(
                key, std::make_shared<Record>(key, name, age, department));

        // 현재 노드가 오버플로우인 동안, 분리 및 부모노드에 전파
        while (p->keys.size() >= degree) {
            std::shared_ptr<BPTNode> newNode =
                std::make_shared<BPTNode>(p->isLeaf);
            std::string propagate_key;  // 오버플로우 시, 부모 노드로 올릴 키
            int mid = p->keys.size() / 2;
            propagate_key = p->keys[mid].first;
            if (p->isLeaf) {
                // 리프면 키가 복사되어 올라가야 함
                // ->새로 추가된 리프노드를 부모노드에서 분기시켜줄 엔트리가 됨.
                for (int i = mid; i < p->keys.size(); i++) {
                    newNode->keys.emplace_back(p->keys[i]);
                }
                p->keys.resize(mid);

                newNode->next = p->next;
                p->next = newNode;
            } else {
                // 내부 노드에서는 복사해서 올리지 않음.
                for (int i = mid + 1; i < p->keys.size(); i++) {
                    newNode->keys.emplace_back(p->keys[i]);
                }
                for (int i = mid + 1; i < p->children.size(); i++) {
                    newNode->children.emplace_back(p->children[i]);
                }

                p->keys.resize(mid);
                p->children.resize(mid + 1);
            }

            // 루트가 오버플로우라면, 부모노드를 생성해서 분기 엔트리만듬
            if (ancestors.empty()) {
                std::shared_ptr<BPTNode> newRoot =
                    std::make_shared<BPTNode>(false);
                newRoot->keys.emplace_back(propagate_key, nullptr);
                newRoot->children.emplace_back(p);
                newRoot->children.emplace_back(newNode);
                root = newRoot;
                break;
            }

            // 부모노드에 새 노드 연결시켜주기
            std::shared_ptr<BPTNode> parent = ancestors.top();
            ancestors.pop();
            bool inserted = false;
            for (int i = 0; i < parent->keys.size(); i++) {
                if (newNode->keys[0].first < parent->keys[i].first) {
                    parent->keys.insert(parent->keys.begin() + i,
                                        {propagate_key, nullptr});
                    parent->children.insert(parent->children.begin() + i + 1,
                                            newNode);
                    inserted = true;
                    break;
                }
            }
            if (!inserted) {
                parent->keys.emplace_back(newNode->keys[0].first, nullptr);
                parent->children.emplace_back(newNode);
            }

            // 계층 끌어올리기(이제 상위 노드 오버플로우 처리)
            p = parent;
        }

        return true;
    }

    void remove(const std::string query) {
        // 루트가 NULL이거나 삭제에 실패 시 따로 알림
        if (!root || !deleteRecursively(query, root, nullptr, nullptr, -1,
                                        nullptr, -1)) {
            std::cout << "Entry " << query << " not found!\n";
        }
        while (root && root->children.size() == 1) root = root->children[0];
    }

    // p는 현재 노드
    // parent는 부모 노드
    // leftSibling은 왼쪽 형제,
    // leftAnchor는 왼쪽 형제와 p의 분기가 되는 부모 노드의 key의 인덱스
    // rightSibling은 오른쪽 형제,
    // rightAnchor는 오른쪽 형제와 p의 분기가 되는 부모 노드의 key의인덱스
    bool deleteRecursively(const std::string entry, std::shared_ptr<BPTNode> p,
                           std::shared_ptr<BPTNode> parent,
                           std::shared_ptr<BPTNode> leftSibling, int leftAnchor,
                           std::shared_ptr<BPTNode> rightSibling,
                           int rightAnchor) {
        // 0. 리프인가?
        if (p->isLeaf) {
            // 1. 리프에서 해당 값 제거
            bool deleted = false;
            for (int i = 0; i < p->keys.size(); i++) {
                if (entry == p->keys[i].first) {
                    p->keys.erase(p->keys.begin() + i);
                    deleted = true;
                    break;
                }
            }
            if (!deleted) return false;

            // 2. 언더플로우인가?
            // 언더플로우 아니면, 빠른 리턴
            if (p->keys.size() >= leafMin) return true;
            // 만약 리프이자 루트인데 언더플로우? 그러면 그냥 유지..
            if (p->keys.empty() && p == root) return true;

            // 3. 왼쪽과 병합 시도..
            // 4. 오른쪽과 병합 시도..
            // 5. 둘 다 안된다? 양쪽 병합 후 redistribution..
            if ((leftSibling &&
                 p->keys.size() + leftSibling->keys.size() < degree) ||
                !rightSibling) {
                // 왼쪽과 병합 시도
                for (int i = 0; i < p->keys.size(); i++) {
                    leftSibling->keys.emplace_back(p->keys[i]);
                }
                if (leftSibling->keys.size() < degree) {
                    // redistribution 필요 없음..
                    // leftAnchor와 p삭제
                    leftSibling->next = p->next;
                    parent->children.erase(parent->children.begin() +
                                           leftAnchor + 1);
                    parent->keys.erase(parent->keys.begin() + leftAnchor);
                } else {
                    // redistribution
                    int mid = leftSibling->keys.size() / 2;
                    p->keys.resize(leftSibling->keys.size() - mid);
                    for (int i = 0; i + mid < leftSibling->keys.size(); i++) {
                        p->keys[i] = leftSibling->keys[mid + i];
                    }
                    leftSibling->keys.resize(mid);
                    parent->keys[leftAnchor].first = p->keys[0].first;
                }

            } else if ((rightSibling &&
                        p->keys.size() + rightSibling->keys.size() < degree) ||
                       !leftSibling) {
                // 오른쪽과 병합 시도
                for (int i = 0; i < rightSibling->keys.size(); i++) {
                    p->keys.emplace_back(rightSibling->keys[i]);
                }
                if (rightSibling->keys.size() < degree) {
                    // redistribution 필요 없음..
                    // rightAnchor와 p삭제
                    p->next = rightSibling->next;
                    parent->children.erase(parent->children.begin() +
                                           rightAnchor + 1);
                    parent->keys.erase(parent->keys.begin() + rightAnchor);
                } else {
                    // redistribution
                    int mid = rightSibling->keys.size() / 2;
                    rightSibling->keys.resize(p->keys.size() - mid);
                    for (int i = 0; i + mid < p->keys.size(); i++) {
                        rightSibling->keys[i] = p->keys[mid + i];
                    }
                    p->keys.resize(mid);
                    parent->keys[rightAnchor].first =
                        rightSibling->keys[0].first;
                }
            } else {
                // 양쪽 다 병합
                std::vector<std::pair<std::string, std::shared_ptr<Record>>>
                    fullKeys(leftSibling->keys.begin(),
                             leftSibling->keys.end());
                fullKeys.insert(fullKeys.end(), p->keys.begin(), p->keys.end());
                fullKeys.insert(fullKeys.end(), rightSibling->keys.begin(),
                                rightSibling->keys.end());
                int sumOfKeys = leftSibling->keys.size() + p->keys.size() +
                                rightSibling->keys.size();

                if (sumOfKeys <= (degree - 1) * 2) {
                    // 2개로 재분배
                    int mid = sumOfKeys / 2;
                    leftSibling->keys.resize(mid);
                    for (int i = 0; i < mid; i++) {
                        leftSibling->keys[i] = fullKeys[i];
                    }

                    for (int i = 0; i + mid < sumOfKeys; i++) {
                        p->keys[i] = fullKeys[i + mid];
                    }
                    // Anchor 수정 및 삭제, 자식포인터 하나 삭제
                    parent->keys[leftAnchor].first = p->keys[0].first;
                    parent->keys.erase(parent->keys.begin() + rightAnchor);
                    parent->children.erase(parent->children.begin() +
                                           rightAnchor + 1);
                    // 오른쪽형제의 포인터 끌어오기
                    p->next = rightSibling->next;
                } else {
                    // 3개로 재분배
                    int seg = sumOfKeys / 3;
                    int remainder = sumOfKeys % 3;

                    int lsz = seg + (--remainder >= 0 ? 1 : 0);
                    for (int i = 0; i < lsz; i++) {
                        leftSibling->keys[i] = fullKeys[i];
                    }

                    int psz = seg + (--remainder >= 0 ? 1 : 0);
                    for (int i = 0; i < psz; i++) {
                        p->keys[i] = fullKeys[lsz + i];
                    }

                    for (int i = 0; i + seg; i++) {
                        rightSibling->keys[i] = fullKeys[lsz + psz + i];
                    }
                    // Anchor 수정
                    parent->keys[leftAnchor].first = p->keys[0].first;
                    parent->keys[rightAnchor].first =
                        rightSibling->keys[0].first;
                }
            }

            return true;
        } else {
            // 1. 리프에 재귀호출해서 리프 삭제 먼저 시키기
            bool res;  // 재귀 호출의 응답
            if (entry < p->keys[0].first) {
                res = deleteRecursively(entry, p->children[0], p, nullptr, -1,
                                        p->children[1], 0);
            } else {
                for (int i = 0; i < p->keys.size(); i++) {
                    if (entry >= p->keys[i].first) {
                        if (i == p->keys.size() - 1)
                            res = deleteRecursively(entry, p->children[i + 1],
                                                    p, p->children[i], i,
                                                    nullptr, -1);
                        else
                            res = deleteRecursively(entry, p->children[i + 1],
                                                    p, p->children[i], i,
                                                    p->children[i + 2], i + 1);
                        break;
                    }
                }
            }

            // 만약 리프로부터 삭제 실패소식이 전파되어 올라오면, 연쇄하여 전파
            if (!res) return false;

            // 만약 언더플로우가 안나면, 조기 종료
            if (p->keys.size() >= internalMin) return true;

            // 2. 만약 루트인 경우
            if (!parent) {
                // 루트인데 포인터가 1개밖에 없다? 루트를 자식에게 물려주고 종료
                if (p->keys.empty() || p->children.size() == 1)
                    root = p->children[0];
                return true;
            }

            // 3. 왼쪽과 병합 시도..
            // 4. 오른쪽과 병합 시도..
            // 5. 둘 다 안된다? 양쪽 병합 후 redistribution..
            if ((leftSibling &&
                 leftSibling->keys.size() + p->keys.size() < degree) ||
                !rightSibling) {
                // 우선, 키 및 자식 포인터들 직렬화
                std::vector<std::pair<std::string, std::shared_ptr<Record>>>
                    fullKeys(leftSibling->keys.begin(),
                             leftSibling->keys.end());
                fullKeys.emplace_back(parent->keys[leftAnchor]);
                fullKeys.insert(fullKeys.end(), p->keys.begin(), p->keys.end());

                std::vector<std::shared_ptr<BPTNode>> fullPtrs(
                    leftSibling->children.begin(), leftSibling->children.end());
                fullPtrs.insert(fullPtrs.end(), p->children.begin(),
                                p->children.end());

                // 좌+앵커+현재가 degree 미만이라면, 병합으로
                leftSibling->keys = fullKeys;
                leftSibling->children = fullPtrs;
                if (leftSibling->keys.size() < degree) {
                    parent->children.erase(parent->children.begin() +
                                           leftAnchor + 1);
                } else {
                    // 아니면, 재분배(redistribution)
                    int mid = fullKeys.size() / 2;
                    leftSibling->keys.resize(mid);
                    leftSibling->children.resize(mid + 1);
                    parent->keys[leftAnchor] = fullKeys[mid];
                    p->keys.resize(fullKeys.size() - mid - 1);
                    p->children.resize(0);
                    for (int i = 2; i + mid <= fullKeys.size(); i++) {
                        p->keys[i - 2] = fullKeys[i + mid - 1];
                        p->children.emplace_back(fullPtrs[i + mid - 1]);
                    }
                    p->children.emplace_back(fullPtrs.back());
                }

            } else if ((rightSibling &&
                        rightSibling->keys.size() + p->keys.size() < degree) ||
                       !leftSibling) {
                // 우선, 키 및 자식 포인터들 직렬화
                std::vector<std::pair<std::string, std::shared_ptr<Record>>>
                    fullKeys(p->keys.begin(), p->keys.end());
                fullKeys.emplace_back(parent->keys[rightAnchor]);
                fullKeys.insert(fullKeys.end(), p->keys.begin(), p->keys.end());

                std::vector<std::shared_ptr<BPTNode>> fullPtrs(
                    p->children.begin(), p->children.end());
                fullPtrs.insert(fullPtrs.end(), rightSibling->children.begin(),
                                rightSibling->children.end());

                // 현재+앵커+우형제의 key가 degree 미만이라면, 병합으로
                p->keys = fullKeys;
                p->children = fullPtrs;
                if (p->keys.size() < degree) {
                    parent->children.erase(parent->children.begin() +
                                           rightAnchor + 1);
                } else {
                    // 아니면, 재분배(redistribution)
                    int mid = fullKeys.size() / 2;
                    p->keys.resize(mid);
                    p->children.resize(mid + 1);
                    parent->keys[rightAnchor] = fullKeys[mid];
                    rightSibling->keys.resize(fullKeys.size() - mid - 1);
                    rightSibling->children.resize(0);
                    for (int i = 2; i + mid <= fullKeys.size(); i++) {
                        rightSibling->keys[i - 2] = fullKeys[i + mid - 1];
                        rightSibling->children.emplace_back(
                            fullPtrs[i + mid - 1]);
                    }
                    rightSibling->children.emplace_back(fullPtrs.back());
                }

            } else {
                // 양쪽과 재분배 ㄱㄱ
                // 좌형제 + 좌앵커 + 현재 + 우앵커 + 우형제의 정보들을 모두
                // 일렬로 세우기
                std::vector<std::pair<std::string, std::shared_ptr<Record>>>
                    fullKeys(leftSibling->keys.begin(),
                             leftSibling->keys.end());
                fullKeys.emplace_back(parent->keys[leftAnchor]);
                fullKeys.insert(fullKeys.end(), p->keys.begin(), p->keys.end());
                fullKeys.emplace_back(parent->keys[rightAnchor]);
                fullKeys.insert(fullKeys.end(), rightSibling->keys.begin(),
                                rightSibling->keys.end());
                std::vector<std::shared_ptr<BPTNode>> fullPtrs(
                    leftSibling->children.begin(), leftSibling->children.end());
                fullPtrs.insert(fullPtrs.end(), p->children.begin(),
                                p->children.end());
                fullPtrs.insert(fullPtrs.end(), rightSibling->children.begin(),
                                rightSibling->children.end());

                // 3개 병합이 1개가 되는 것은, 불가능
                // 이웃들은 이미 절반이상 있기에,
                // 3개병합이 1개될순없음
                // 3개에서 2개로 될 수 있다면, 2개로 만들기
                if (fullKeys.size() <= 2 * (degree - 1) + 1) {
                    int mid = fullKeys.size() / 2;
                    leftSibling->keys = fullKeys;
                    leftSibling->keys.resize(mid);
                    parent->keys[leftAnchor] = fullKeys[mid + 1];
                    p->keys.resize(fullKeys.size() - mid - 1);
                    p->children.resize(0);
                    for (int i = 2; i + mid < fullKeys.size(); i++) {
                        p->keys[i - 2] = fullKeys[i + mid];
                        p->children.emplace_back(fullPtrs[i + mid - 1]);
                    }
                    p->children.emplace_back(fullPtrs.back());

                    parent->keys.erase(parent->keys.begin() + rightAnchor);
                    parent->children.erase(parent->children.begin() +
                                           rightAnchor + 1);
                } else {
                    // 아니면 재분배
                    int seg = (fullKeys.size() - 2) / 3;
                    int remainder = ((fullKeys.size() - 2) % 3) / 2;
                    int j = 0;

                    int lsz = seg + (--remainder >= 0 ? 1 : 0);
                    leftSibling->keys.resize(lsz);
                    leftSibling->children.resize(0);
                    for (int i = 0; i < lsz; i++) {
                        leftSibling->keys[i] = fullKeys[j];
                        leftSibling->children.emplace_back(fullPtrs[j]);
                        j++;
                    }
                    leftSibling->children.emplace_back(fullPtrs[j + 1]);

                    parent->keys[leftAnchor] = fullKeys[j++];
                    int psz = seg + (--remainder >= 0 ? 1 : 0);
                    p->keys.resize(psz);
                    p->children.resize(0);
                    for (int i = 0; i < psz; i++) {
                        p->keys[lsz + 1 + i] = fullKeys[j];
                        p->children.emplace_back(fullPtrs[j + 1]);
                        j++;
                    }
                    p->children.emplace_back(fullPtrs[j + 2]);

                    parent->keys[rightAnchor] = fullKeys[j++];
                    rightSibling->keys.resize(seg);
                    rightSibling->children.resize(0);
                    for (int i = 0; i < seg; i++) {
                        rightSibling->keys[lsz + psz + i] = fullKeys[j];
                        rightSibling->children.emplace_back(fullPtrs[j + 2]);
                        j++;
                    }
                }
            }
            return true;
        }
    }
};

int main(int argc, char* argv[]) {
    std::shared_ptr<BPlusTree> mybpt = std::make_shared<BPlusTree>(MAXDEGREE);

    mybpt->initializeWithInteraction();
    mybpt->printTree();

    // Deletion from Figure 14.14
    mybpt->remove("Srinivasan");
    mybpt->remove("Singh");
    mybpt->remove("Wu");
    mybpt->remove("Gold");
    mybpt->printTree();
    return 0;
}
