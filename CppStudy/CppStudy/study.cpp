#include<iostream>

using namespace std;

int main()
{
	int c = -1;
	char d = 'a';
	char e = 'b';
	cin >> c >> d;
	if (cin.fail()) {
		cout << "입력 실패! 스트림 초기화 중...\n";
		cin.clear(); // failbit 해제
		cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 잘못된 입력 버퍼 비우기
	}
	cout << c << "\n";
	cout << d << "\n";
	cin >> e;
	cout << e << "\n";
}