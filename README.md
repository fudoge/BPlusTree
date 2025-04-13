# B+ Tree
B+트리의 C++구현입니다.  
인터랙티브하게 트리를 구성할 수도 있고, 입력 파일을 통해서 구조를 불러올 수 있습니다.  

보통의 B+트리에서, 리프노드의 포인터들은 각 레코드를 가리키고, 마지막 하나가 다음 노드를 가리키지만,  
여기서는 그러한 포인터들이 오직 자식노드만을 위해 연결됩니다.  
대신, keys의 요소로,  `<SearchKey, Record에 대한 포인터>`를 저장하여 키가 직접 레코드를 가리킵니다.  
내부 노드에서는 keys의 요소들이 `<SearchKey, nullptr>`의 꼴로 존재합니다.  
리프 노드는 `next` 포인터를 통해서 리프노드들간의 연결리스트를 구성하도록 하였습니다.  
`shared_ptr`을 쓰는 구현의 편리함을 위한 선택이였습니다.  

트리가 생성되면, 명령어를 통해서 트리에 연산을 적용할 수 있습니다. 

> 해당 소스는 학습을 위한 예시일 뿐입니다. 

## 셋업 예시
```bash
g++ -std=c++17 -o main main.cpp
```

## 실행하기
```bash
## 빈 트리를 생성하고, 노드의 최대 차수는 4로 합니다.
./main 0, 4
```

```bash
## 인터랙티브 모드로 트리를 입력하고, 노드의 최대 차수는 4로 합니다.
./main 1, 4
```

입력파일 기반 트리 생성 예시
```bash
## fig1409.txt의내용을 트리로 입력합니다. 
## 노드의 최대 차수는 4로 합니다.
./main 2 4 fig1409.txt

```

> [!CAUTION]
> 인터랙티브 모드의 입력으로는 유효한 B+트리를 낮은 레벨부터,  
> 같은 레벨에서는 왼쪽에서 오른쪽순서대로 입력하셔야 합니다.  
> 포화 이진 트리에서의 배열 표현을 생각하시면 됩니다.  
> 루트부터 순서번호 1로 시작합니다.  

## 입력 파일 만들기 예시
1. 노드 번호를 표준출력으로부터 전달받아서, 노드를 식별
2. 리프 여부 입력(y 또는 Y를 눌러 리프 노드로 마킹)
3. 각 키 값 입력(0-indexed)

이후, 레코드 채우기 부분에서:
1. 레코드의 SearchKey를 통해 식별
2. 이름
3. 나이
4. 부서 | 학과 입력

예를 들어, 아래와 같은 인터렉션을 원한다면, 
```plaintext
Node 1:
Is it a leaf node? If leaf, input 'y' or 'Y', otherwise, type any.
y
How many keys it has?
2
Please Input the keys in 0-indexed.
Key[0]:
15
Key[1]:
25

[Record Initialization for key 15]
Input the Informations line by line:
Name: Alice
Age: 22
Department: CS

[Record Initialization for key 25]
Input the Informations line by line:
Name: Bob
Age: 24
Department: EE

```

텍스트 파일을 아래와 같이 만들 수 있습니다:
```plaintext
y
2
15
25
Alice
22
CS
Bob
24
EE
```

## 커맨드
- `HELP`: 프로그램의 사용법을 얻습니다.
- `FIND`: 해당 레코드를 검색합니다. 검색할 레코드의 서치 키를 입력해야 합니다.
- `INSERT`: 새 레코드를 삽입합니다. 이 명령어 이후에는 필드를 입력해야 합니다.
- `DELETE:` 해당 키를 가진 레코드를 삭제합니다. 이 명령어 이후에는 삭제할 서치 키를 입력해야 합니다.
- `PRINT`: 트리를 레벨 순서대로, 같은 레벨에서는 왼쪽부터 오른쪽 순서로 출력합니다.
- `EXIT` 또는 `QUIT`: 프로그램을 종료합니다.

## To Do 
- [ ] 서치 키 탬플릿으로 변경
- [ ] 레코드 탬플릿으로 변경
- [ ] 구간 쿼리 및 조건부 쿼리
- [ ] 인터랙티브 개선
- [ ] 프로젝트 구조화
