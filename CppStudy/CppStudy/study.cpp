#include<iostream>

using namespace std;

int main()
{
	int c = -1;
	char d = 'a';
	char e = 'b';
	cin >> c >> d;
	if (cin.fail()) {
		cout << "�Է� ����! ��Ʈ�� �ʱ�ȭ ��...\n";
		cin.clear(); // failbit ����
		cin.ignore(numeric_limits<streamsize>::max(), '\n'); // �߸��� �Է� ���� ����
	}
	cout << c << "\n";
	cout << d << "\n";
	cin >> e;
	cout << e << "\n";
}