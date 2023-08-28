#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

using namespace std;

namespace day25 {
	void part1() {
		ifstream input{ "Day25.txt" };
		if (!input) throw invalid_argument("Failed to open Day25.txt");

		int64_t total{ 0 };

		string line;
		while (getline(input, line) && line.length() > 0) {
			int64_t value{ 0 };
			for (char c : line) {
				value *= 5;
				switch (c) {
				case '0': break;
				case '1': {
					value++;
					break;
				}
				case '2': {
					value += 2;
					break;
				}
				case '-': {
					value--;
					break;
				}
				case '=':
					value -= 2;
					break;
				default: throw invalid_argument("Unexpected digit " + c);
				}
			}
			total += value;
		}

		// Now print that total in base 5.
		// special case where value is 0
		if (total == 0) {
			cout << 0 << endl;
			return;
		}
		 
		// We work out digits right -> left, so have to build up a vector and pop in reverse.
		// If we really cared about efficiency, could use (modified) powers of 5 to get size of array up front.
		vector<char> encodedValue{};
		while (total != 0) {
			int rem = total % 5;
			switch (rem) {
			case 0: {
				encodedValue.push_back('0');
				break;
			}
			case 1: {
				encodedValue.push_back('1');
				break;
			}
			case 2: {
				encodedValue.push_back('2');
				break;
			}
			case 3:
				encodedValue.push_back('=');
				total += 5;
				break;

			case 4:
				encodedValue.push_back('-');
				total += 5;
				break;
			}
		
			total /= 5;
		}

		while (!encodedValue.empty()) {
			cout << encodedValue.back(); encodedValue.pop_back();
		}
		cout << endl; // 2-0-01==0-1=2212=100
	}

	void part2() {
		ifstream input{ "Day25.txt" };
		if (!input) throw invalid_argument("Failed to open Day25.txt");
	}

	void main() {
		time_t start, end;
		time(&start);
		part1();
		part2();
		time(&end);

		cout << "Elapsed: " << (double)(end - start) << endl;
	}
}