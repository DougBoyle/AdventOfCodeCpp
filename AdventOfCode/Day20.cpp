#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

namespace day20 {

	// List has duplicates, so need to remember 'original' order to update in.
	// First value is index [0 to n), second is actual value.
	typedef pair<int, int64_t> IndexedInt64;

	/*
		Moves element at fromIdx to toIdx, shuffling down everything in between
	*/
	template<typename T>
	void shift(vector<T>& values, int fromIdx, int toIdx) {
		if (toIdx > fromIdx) {
			// want to rotate (original)(original+1, ..., newIndex) to move original element to end.
			rotate(values.begin() + fromIdx, values.begin() + fromIdx + 1, values.begin() + toIdx + 1);
		}
		else {
			// destination is before current position, need to roate (newIndex, newIndex + 1, ..., original-1)(original)
			rotate(values.begin() + toIdx, values.begin() + fromIdx, values.begin() + fromIdx + 1);
		}
	}

	/*
		std::rotate(start, mid, end) is only valid when start <= mid <= end.
		Our use case either mid = start + 1, or mid = end - 1, due to only 'shifting' 1 element.
		Given it's only 1 element, if start=X (near 0) and end=Y (near N),
			instead faster to swap element X with end element N, then rotate [0, X-1, X], [Y, N-1, N].
			(element near start moving to end, so element at end N will wrap round the end up at 0)
		If start=X (near N) and end=Y (near 0), do the opposite:
			Swap X with element 0, rotate [X, X+1, N] and [0, 1, Y]
	*/
	template<typename T>
	void fastShift(vector<T>& values, int fromIdx, int toIdx) {
		int directDistance = fromIdx < toIdx ? toIdx - fromIdx : fromIdx - toIdx;
		// instead 'wrap around outside', moving lower value down to 0 and upper value up to N
		int N = static_cast<int>(values.size());

		if (directDistance > N / 2) {
			if (fromIdx < toIdx) {
				// currently near start, need to end up near end, element at end displaced round to start
				swap(values[fromIdx], values[N - 1]);
				shift(values, fromIdx, 0);
				// toIdx + 1, as we ultimately wanted to end up just after the element at toIdx,
				// and have instead done this by rotating everything ELSE 1
				shift(values, N - 1, toIdx + 1);
			}
			else {
				swap(values[fromIdx], values[0]);
				shift(values, 0, toIdx - 1); // we end up before toIdx
				shift(values, fromIdx, N - 1);
			}
		}
		else {
			shift(values, fromIdx, toIdx);
		}
	}

	void mix(vector<IndexedInt64>& values) {
		const int N = static_cast<int>(values.size());

		for (int i = 0; i < N; i++) {
			const auto elementIter = find_if(values.begin(), values.end(), [i](const auto& value) { return value.first == i; });
			// copy, because we're going to be removing/inserting later, which will modify addresses and invalidate reference
			const auto [idx, value] = *elementIter;
			if (value == 0) continue; //doesn't move

			int originalIndex = static_cast<int>(elementIter - values.begin());

			// to find new index, consider that position in a list without it (i.e. 1 smaller length),
			// move around {value} hops (where -ve can be at 0, and +ve can be at N), and then re-insert
			int newIndex = (originalIndex + value) % (N - 1);
			// modulo defined to ensure a = (a/b)*b + a%b -- https://stackoverflow.com/questions/7594508/modulo-operator-with-negative-values
			if (newIndex < 0) newIndex += N - 1;
			// Shuffling down to start or up to end is identical, so pick closer one.
			// However, if wrapping around ends, can't use fastShift since it relies on elements from either end actually swapping sides.
			if (newIndex == 0 && originalIndex > N/2) newIndex = N - 1;

			// Despite newIndex being a point to insert into in reduced size array, it's still correct in new array.
			// Getting newIdx = originalIdx means swap with self i.e. nothing, same as removing at current position and re-inserting.
			// 
			// In reduced size one, inserting at both index 0 or N-1 (extend end of array) are equivalent due to circular behaviour.
			// In actual array, this just means we always pick value in range 0 to N-2, never displacing last element (idx N-1),
			// since it is equivaent to displacing everything on the way DOWN to element index 0 instead.
			// We end up selectively replacing 0 with N-1 to pick closer side instead.
			fastShift(values, originalIndex, newIndex);
		}
	}

	int64_t getCoordSum(const vector<IndexedInt64>& values) {
		// find values 1k, 2k, and 3k positions after the value 0
		const auto zeroIdx = find_if(values.begin(), values.end(), [](const auto& value) { return value.second == 0; }) - values.begin();
		int64_t v1 = values[(zeroIdx + 1000) % values.size()].second;
		int64_t v2 = values[(zeroIdx + 2000) % values.size()].second;
		int64_t v3 = values[(zeroIdx + 3000) % values.size()].second;

		return v1 + v2 + v3;
	}

	void part1() {
		ifstream input{ "Day20.txt" };

		vector<IndexedInt64> values;
		int count = 0;
		int64_t value;
		while (input >> value) values.push_back({ count++, value });

		const int N = static_cast<int>(values.size());
		
		cout << "Num values: " << N << endl;

		mix(values);

		cout << getCoordSum(values) << endl; // 8302
	}

	void part2() {
		ifstream input{ "Day20.txt" };
		const int64_t KEY = 811589153;

		vector<IndexedInt64> values;
		int count = 0;
		int64_t value;
		while (input >> value) values.push_back({ count++, value * KEY });

		const int N = static_cast<int>(values.size());


		for (int iteration = 0; iteration < 10; iteration++) {
			mix(values);
		}

		cout << getCoordSum(values) << endl; // 656575624777
	}

	/*
		Without any improvements, takes about 5s
		List has 5000 elements.
		No real change from using rotate()
			Numbers large enough to effectively be random swaps, so making nearby shift fast instead of 
			insert/removes near the end of the list doesn't significantly improve performance.
		Even 'rotating' wrapping around outside only helps marginally, only applicable half of the time,
			and speedup only significant if 1-r << r (i.e. 40% vs 60% pretty negligible)
	*/
	void main() {
		time_t start, end;
		time(&start);
		part1();
		part2();
		time(&end);

		cout << "Elapsed: " << (double)(end - start) << endl;
	}
}