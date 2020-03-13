#include <time.h>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
using namespace std;

const string s = "CDHS", v = "A23456789TJQK";
const int handCards = 9, drawCards = 3;

class card {
public:
	friend ostream& operator<< (ostream& os, const card& c) {
		os << v[c.val] << s[c.suit];
		return os;
	}
	bool isValid() { return val > -1; }
	void set(char s, char v) { suit = s; val = v; }
	char getRank() { return v[val]; }
	bool operator == (const char o) { return v[val] == o; }
	bool operator < (const card& a) { if (val == a.val) return suit < a.suit; return val < a.val; }
private:
	char suit, val;
};
class deck {
public:
	static deck* instance() {
		if (!inst) inst = new deck();
		return inst;
	}
	void destroy() {
		delete inst;
		inst = 0;
	}
	card draw() {
		card c;
		if (cards.size() > 0) {
			c = cards.back();
			cards.pop_back();
			return c;
		}
		c.set(-1, -1);
		return c;
	}
private:
	deck() {
		newDeck();
	}
	void newDeck() {
		card c;
		for (char s = 0; s < 4; s++) {
			for (char v = 0; v < 13; v++) {
				c.set(s, v);
				cards.push_back(c);
			}
		}
		random_shuffle(cards.begin(), cards.end());
		random_shuffle(cards.begin(), cards.end());
	}
	static deck* inst;
	vector<card> cards;
};
class player {
public:
	player(string n) : nm(n) {
		for (int x = 0; x < handCards; x++)
			hand.push_back(deck::instance()->draw());
		sort(hand.begin(), hand.end());
	}
	void outputHand() {
		for (vector<card>::iterator x = hand.begin(); x != hand.end(); x++)
			cout << (*x) << " ";
		cout << " ";
	}
	bool addCard(card c) {
		hand.push_back(c);
		return checkForBook();
	}
	string name() {
		return nm;
	}
	bool holds(char c) {
		return(hand.end() != find(hand.begin(), hand.end(), c));
	}
	card takeCard(char c) {
		vector<card>::iterator it = find(hand.begin(), hand.end(), c);
		swap((*it), hand.back());
		card d = hand.back();
		hand.pop_back();
		hasCards();
		sort(hand.begin(), hand.end());
		return d;
	}
	size_t getBooksCount() {
		return books.size();
	}
	void listBooks() {
		for (vector<char>::iterator it = books.begin(); it != books.end(); it++)
			cout << (*it) << "'s ";
		cout << " ";
	}
	bool checkForBook() {
		bool ret = false;
		map<char, int> countMap;
		for (vector<card>::iterator it = hand.begin(); it != hand.end(); it++)
			countMap[(*it).getRank()]++;
		for (map<char, int>::iterator it = countMap.begin(); it != countMap.end(); it++) {
			if ((*it).second == 4) {
				do {
					takeCard((*it).first);
				} while (holds((*it).first));
				books.push_back((*it).first);
				(*it).second = 0;
				ret = true;
			}
		}
		sort(hand.begin(), hand.end());
		return ret;
	}
	bool hasCards() {
		if (hand.size() < 1) {
			card c;
			for (int x = 0; x < drawCards; x++) {
				c = deck::instance()->draw();
				if (c.isValid()) addCard(c);
				else break;
			}
		}
		return(hand.size() > 0);
	}
protected:
	string nm;
	vector<card> hand;
	vector<char> books;
};
class aiPlayer : public player {
public:
	aiPlayer(string n) : player(n), askedIdx(-1), lastAsked(0), nextToAsk(-1) { }
	void rememberCard(char c) {
		if (asked.end() != find(asked.begin(), asked.end(), c) || !asked.size())
			asked.push_back(c);
	}
	char makeMove() {
		if (askedIdx < 0 || askedIdx >= static_cast<int>(hand.size())) {
			askedIdx = rand() % static_cast<int>(hand.size());
		}

		char c;
		if (nextToAsk > -1) {
			c = nextToAsk;
			nextToAsk = -1;
		}
		else {
			while (hand[askedIdx].getRank() == lastAsked) {
				if (++askedIdx == hand.size()) {
					askedIdx = 0;
					break;
				}
			}
			c = hand[askedIdx].getRank();
			if (rand() % 100 > 25 && asked.size()) {
				for (vector<char>::iterator it = asked.begin(); it != asked.end(); it++) {
					if (holds(*it)) {
						c = (*it);
						break;
					}
				}
			}
		}
		lastAsked = c;
		return c;
	}
	void clearMemory(char c) {
		vector<char>::iterator it = find(asked.begin(), asked.end(), c);
		if (asked.end() != it) {
			swap((*it), asked.back());
			asked.pop_back();
		}
	}
	bool addCard(card c) {
		if (!holds(c.getRank()))
			nextToAsk = c.getRank();
		return player::addCard(c);
	}
private:
	vector<char> asked;
	char nextToAsk, lastAsked;
	int askedIdx;
};
class goFish {
public:
	goFish() {
		plr = true;
		string n;
		cout << "Enter your name: "; 
		cin >> n;
		p1 = new player(n);
		p2 = new aiPlayer("COMP");
	}
	~goFish() {
		if (p1) delete p1;
		if (p2) delete p2;
		deck::instance()->destroy();
	}
	void play() {
		while (true) {
			if (process(getInput())) break;
		}
		cout << " ";
		showBooks();
		if (p1->getBooksCount() > p2->getBooksCount()) {
			cout << "CONGRATULATIONS\n";
		}
		else {
			cout << "YOU LOSE\n";
		}
	}
private:
	void showBooks() {
		if (p1->getBooksCount() > 0) {
			cout << "Your Book(s): ";
			p1->listBooks();
		}
		if (p2->getBooksCount() > 0) {
			cout << "Computer Book(s): ";
			p2->listBooks();
		}
	}
	void showPlayerCards() {
		system("cls");
		cout << p1->name() << ", your cards: ";
		p1->outputHand();
		showBooks();
	}
	char getInput() {
		char c;
		if (plr) {
			if (!p1->hasCards()) return -1;
			showPlayerCards();
			string w;
			while (true) {
				cout << "\nWhat card(A 2-9 TJKQ) do you want? "; 
				cin >> w;
				c = toupper(w[0]);
				if (p1->holds(c)) break;
				cout << p1->name() << ", you can't ask for a card you don't have! ";
				system("pause");
			}
		}
		else {
			if (!p2->hasCards()) return -1;
			c = p2->makeMove();
			showPlayerCards();
			string r;
			cout << "\nDo you have any " << c << "'s? \n[Y]-Yes\t[N]-No: ";
			do {
				getline(cin, r);
				r = toupper(r[0]);
			} while (r[0] != 'Y' && r[0] != 'N');
			bool hasIt = p1->holds(c);
			if (hasIt && r[0] == 'N')
				cout << "You do have the card";
			if (!hasIt && r[0] == 'Y')
				cout << "You don't have that card";
			
		}
		return c;
	}
	bool process(char c) {
		if (c < 0) return true;
		if (plr) p2->rememberCard(c);

		player* a, * b;
		a = plr ? p2 : p1;
		b = plr ? p1 : p2;
		bool r;
		if (a->holds(c)) {
			while (a->holds(c)) {
				r = b->addCard(a->takeCard(c));
			}
			if (plr && r)p2->clearMemory(c);
		}
		else {
			fish();
			plr = !plr;
		}
		return false;
	}
	void fish() {
		cout << "\aGO FISH!\n";
		card c = deck::instance()->draw();
		if (plr) {
			cout << "Your new card: " << c << ". Computer Turn" << string(36, '-') << " ";
			if (p1->addCard(c)) p2->clearMemory(c.getRank());
		}
		else {
			cout << "Your Turn" << string(36, '-') << " ";
			p2->addCard(c);
		}
		system("pause");
	}

	player* p1;
	aiPlayer* p2;
	bool plr;
};
deck* deck::inst = 0;
int main(int argc, char* argv[]) {
	char again;
	bool playAgain = true;

	cout << "RULES:\n";
	cout << "-One of the players (player A, YOU) begins the game by asking the other player (player B, COMPUTER) for all of his/her cards of a specific rank (e.g. \"Please give me all of your 7's\"). To ask for a given rank, a player must have at least one card of that rank in his/her hand. ";
	cout << "\n\n-If player B has cards of the requested rank, he/she must give all of his/her cards of that rank to player A, and player A gets to take another turn. ";
	cout << "\n\n-If player B does not have any cards of the requested rank, he/she says \"go fish\", and player A must select one card from the stock. If that card has the rank player A originally requested, then player A gets to take another turn. Otherwise, it becomes player B's turn. ";
	cout << "\n\n-If at any point a player runs out of cards, then, when it is that player's turn to play, they may draw a card from the stock and ask for cards of that rank. If a player runs out of other cards when it is the other player's turn to ask for a rank, the other player may continue to ask for a rank and draw from the stock until they draw a rank other than the one they asked for. ";
	cout << "\n\n-If a player collects all four cards of the same rank, this is called a \"book\", and the player lays down his/her book on the table. Both players may see the cards in every book that's laid down.";
	cout << "\n\n-The game continues with the players alternating turns until all of the books are laid down. At the end of the game, the player who laid down the most books wins. ";
	cout << "\n\n Whenever you understand the rules, ";
	system("pause");
	while (playAgain == true) {
		system("cls");
		srand(static_cast<unsigned>(time(NULL)));
		goFish f; f.play();
		cout << "\nWould you like to play again? \n[Y]-YES\t[N]-No\n";
		cin >> again;
		again = toupper(again);
		if (again == 'Y') {
			playAgain = true;
		}
		else {
			playAgain = false;
			cout << "\nGoodbye";
		}
	}
	return 0;
}