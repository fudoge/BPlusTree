#include <algorithm>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
        this->leafMin = (degree - 1) / 2 + degree % 2;  // ceil(n-1/2)
        this->internalMin = degree / 2;  // 키 수 = 최소포인터수-1 = ceil(n/2)-1
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

    bool deleteByKey(const std::string query) {
        if (root.get() == nullptr) {
            return false;
        }
        std::shared_ptr<BPTNode> p = root;
        std::stack<std::shared_ptr<BPTNode>> ancestors;
        std::stack<std::pair<std::shared_ptr<BPTNode>, char>> siblings;
        while (!p->isLeaf) {
            ancestors.emplace(p);
            int i = 0;
            while (i < p->keys.size() && query < p->keys[i].first) {
                i++;
            }
            if (i == 0) {
                siblings.emplace(p->children[1], 'R');
            } else {
                siblings.emplace(p->children[i - 1], 'L');
            }
            p = p->children[i];
        }

        bool leafDeleted = false;
        bool internalDeleted = false;
        std::unordered_set<std::string> internalDeletionTable;
        std::unordered_map<std::string, std::string> internalReplaceTable;
        while (true) {
            // base case:: 리프 삭제되고 더처리할 내부노드도 없다? 리턴 ㄱ
            if (leafDeleted && internalDeletionTable.empty() &&
                internalReplaceTable.empty())
                return true;
            // 삭제 처리
            // 1. 리프인가?
            // 2. 0번 키를 삭제했나?
            if (p->isLeaf) {
                int ii = 0;
                while (ii < p->keys.size()) {
                    if (query == p->keys[ii].first) {
                        leafDeleted = true;
                        break;
                    }
                    ii++;
                }
                if (!leafDeleted)
                    return false;
                else
                    p->keys.erase(p->keys.begin() + ii);
                std::string propagate_key;
                if (ii == 0) {
                    propagate_key = p->keys[0].first;
                    if (p != firstNode) internalDeletionTable.insert(query);
                }

                if (ancestors.empty()) {
                    if (p->keys.empty()) {
                        root = p->children[0];
                    }
                    break;
                }
                std::shared_ptr parent = ancestors.top();
                ancestors.pop();
                std::shared_ptr sibling = siblings.top().first;
                char siblingDir = siblings.top().second;
                siblings.pop();

                // underflow
                if (p->keys.size() < leafMin) {
                    if (siblingDir == 'R') {
                        // merge vs redistribute
                        if (p->keys.size() + sibling->keys.size() < degree) {
                            // merge

                            int siblingPastsize = sibling->keys.size();
                            sibling->keys.resize(p->keys.size() +
                                                 siblingPastsize);
                            std::move_backward(
                                sibling->keys.begin(),
                                sibling->keys.begin() + siblingPastsize,
                                sibling->keys.end());
                            for (int i = 0; i < p->keys.size(); i++) {
                                sibling->keys[i] = p->keys[i];
                            }
                            parent->keys.erase(parent->keys.begin());
                            internalReplaceTable[parent->keys[0].first] =
                                propagate_key;
                            parent->children.erase(parent->children.begin());
                        } else {
                            // redistribute

                            // 내부노드 내 대체테이블에 등록
                            internalReplaceTable[sibling->keys[0].first] =
                                sibling->keys[1].first;
                            p->keys.emplace_back(sibling->keys[0]);
                            sibling->keys.erase(sibling->keys.begin());
                        }
                    } else {
                        // siblingDir == 'L'
                        // merge vs redistribute
                        if (p->keys.size() + sibling->keys.size() < degree) {
                            // merge
                            internalDeletionTable.insert(p->keys[0].first);
                            for (int i = 0; i < p->keys.size(); i++) {
                                sibling->keys.emplace_back(p->keys[i]);
                            }
                            for (int i = 0; i < parent->keys.size(); i++) {
                                if (query >= parent->keys[i].first) {
                                    parent->children.erase(
                                        parent->children.begin() + i + 1);
                                    break;
                                }
                            }
                        } else {
                            // redistribute
                            // 여기부터
                        }
                    }
                }

            } else {
                // Internal Processing
            }
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
