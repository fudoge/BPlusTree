#include <cctype>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <vector>

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
    std::shared_ptr<BPTNode> firstNode;

   public:
    BPlusTree(int degree) {
        this->degree = degree;
        this->leafMin = (degree - 1) / 2 + (degree - 1) % 2;  // ceil(n-1/2)
        this->internalMin = degree / 2 + degree % 2 -
                            1;  // 키 수 = 최소포인터수-1 = ceil(n/2)-1
        this->root = nullptr;
    }

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

    bool insert(const std::string key, std::string name, int age,
                std::string department) {
        if (root.get() == nullptr) {
            root = std::make_shared<BPTNode>(true);
            root->keys.emplace_back(
                key, std::make_shared<Record>(key, name, age, department));
            firstNode = root;
            return true;
        }

        // 삽입
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
                return false;
        }
        if (!inserted)
            p->keys.emplace_back(
                key, std::make_shared<Record>(key, name, age, department));

        // 분리 및 전파
        while (p->keys.size() >= degree) {
            std::shared_ptr<BPTNode> newNode =
                std::make_shared<BPTNode>(p->isLeaf);
            std::string propagate_key;
            int mid = p->keys.size() / 2;
            propagate_key = p->keys[mid].first;
            if (p->isLeaf) {
                for (int i = mid; i < p->keys.size(); i++) {
                    newNode->keys.emplace_back(p->keys[i]);
                }
                p->keys.resize(mid);

                newNode->next = p->next;
                p->next = newNode;
            } else {
                for (int i = mid + 1; i < p->keys.size(); i++) {
                    newNode->keys.emplace_back(p->keys[i]);
                }
                for (int i = mid + 1; i < p->children.size(); i++) {
                    newNode->children.emplace_back(p->children[i]);
                }

                p->keys.resize(mid);
                p->children.resize(mid + 1);
            }

            if (ancestors.empty()) {
                std::shared_ptr<BPTNode> newRoot =
                    std::make_shared<BPTNode>(false);
                newRoot->keys.emplace_back(propagate_key, nullptr);
                newRoot->children.emplace_back(p);
                newRoot->children.emplace_back(newNode);
                root = newRoot;
                break;
            }
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
    }

    // p는 현재 노드
    // parent는 부모 노드
    // leftSibling은 왼쪽 형제,
    // leftAnchor는 왼쪽 형제와 p의 분기가 되는 부모 노드의 key의 인덱스
    // rightSibling은 왼쪽 형제,
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
            if (p->keys.size() + leftSibling->keys.size() < degree) {
                // 왼쪽과 병합 시도
                for (int i = 0; i < p->keys.size(); i++) {
                    leftSibling->keys.emplace_back(p->keys[i]);
                }
                // leftAnchor와 p삭제
                leftSibling->next = p->next;
                parent->children.erase(parent->children.begin() + leftAnchor);
                parent->keys.erase(parent->keys.begin() + leftAnchor);
            } else if (p->keys.size() + rightSibling->keys.size() < degree) {
                // 왼쪽과 병합 시도
                for (int i = 0; i < rightSibling->keys.size(); i++) {
                    p->keys.emplace_back(rightSibling->keys[i]);
                }
                // rightAnchor와 p삭제
                p->next = rightSibling->next;
                parent->children.erase(parent->children.begin() + rightAnchor);
                parent->keys.erase(parent->keys.begin() + rightAnchor);
            } else {
                // 양쪽 다 병합
                int sumOfKeys = leftSibling->keys.size() + p->keys.size() +
                                rightSibling->keys.size();

                // p가 left에게 줌..
                int idx = 0;
                while (leftSibling->keys.size() < sumOfKeys / 2) {
                    leftSibling->keys.emplace_back(p->keys[idx]);
                    idx++;
                }
                // p에서 left에 준만큼 자르기
                p->keys.erase(p->keys.begin(), p->keys.begin() + idx);

                // right가 p에 줌..
                idx = 0;
                while (leftSibling->keys.size() < sumOfKeys / 2) {
                    leftSibling->keys.emplace_back(rightSibling->keys[idx]);
                    idx++;
                }

                // right[idx:]부터는 다 p로 밀어버리기
                for (int i = idx; i < rightSibling->keys.size(); i++) {
                    p->keys.emplace_back(rightSibling->keys[i]);
                }
                // 부모 노드에서 정리..
                parent->keys.erase(parent->keys.begin() + rightAnchor);
                parent->keys.erase(parent->keys.begin() + leftAnchor);
                parent->children.erase(parent->children.begin() + rightAnchor);
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
                            res = deleteRecursively(
                                entry, p->children.back(), p,
                                p->children[p->keys.size() - 1], p->keys.size(),
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
        }

        return true;
    }
};

int main(int argc, char* argv[]) {
    std::cout << "Hello Database!\n";
    std::shared_ptr<BPlusTree> mybpt = std::make_shared<BPlusTree>(4);

    mybpt->printTree();
    mybpt->insert("A102", "Jisoo Kim", 24, "Computer Science");
    mybpt->insert("B840", "Minho Lee", 21, "Mechanical Engineering");
    mybpt->insert("C203", "Hyejin Park", 22, "Business Administration");
    mybpt->insert("D394", "Taeyang Choi", 25, "Electrical Engineering");
    mybpt->insert("E583", "Ara Yoon", 20, "Psychology");
    mybpt->insert("F238", "Jihoon Han", 23, "Mathematics");
    mybpt->insert("G492", "Seoyun Lim", 22, "Architecture");
    mybpt->insert("H183", "Nayeon Jung", 24, "Biology");
    mybpt->insert("I823", "Hyunwoo Seo", 26, "Chemical Engineering");
    mybpt->insert("J094", "Yuna Kang", 21, "Philosophy");
    mybpt->printTree();
    return 0;
}
