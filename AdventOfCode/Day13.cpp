#include <iostream>
#include <fstream>
#include <string> // getline
#include <sstream> // string -> stream
#include <stdexcept>
#include <cassert>
#include <vector>
#include <algorithm>

using namespace std;

class Packet {
private:
    enum Type { VALUE, LIST } type;

    union Value { 
        int val;
        // vector of pointers, since don't want to move/copy around a bunch of Packets if vector resizes
        // (could use Packets themselves, but then need to define either move or copy constructor/assignment)
        vector<Packet*> values;
        // need to provide a default constructor
        constexpr Value() {
            val = 0;
        }
        constexpr ~Value() {} // must instead be handled by containing class based on Type flag
    } value;

public:
    Packet* parent;

    explicit Packet(Packet* const parent, int num) : parent(parent), type(VALUE) {
        value.val = num;
    }

    explicit Packet(Packet* const parent) : parent(parent), type(LIST) {
        value.values = vector<Packet*>();
    }

    Packet* add(Packet* const p) {
        return value.values.emplace_back(p);
    }

    ~Packet() {
        if (type == LIST) {
            for (auto p : value.values) delete p;
            value.values.~vector();
        }
    }

    strong_ordering operator<=>(const Packet& other) const {
        if (type == VALUE) {
            if (other.type == VALUE) {
                return value.val <=> other.value.val;
            }
            else { // compare as if val = [val]
                switch (other.value.values.size()) {
                case 0: return strong_ordering::greater;
                case 1: {
                    return operator<=>(*other.value.values[0]);
                }
                default:
                    auto res = operator<=>(*other.value.values[0]);
                    if (res != strong_ordering::equal) return res;
                    else return strong_ordering::less;
                }
            }
        }
        else if (other.type == VALUE) {
            return 0 <=> (other <=> *this); // flip to always do val <=> [...], not [...] <=> val
        }
        else {
            // both lists
            for (int i = 0; i < value.values.size() && i < other.value.values.size(); i++) {
                auto res = *value.values[i] <=> *other.value.values[i];
                if (res != strong_ordering::equal) return res;
            }
            // hit the end of one list, compare by length
            return value.values.size() <=> other.value.values.size();
        }
    }
};

namespace day13 {
    Packet* readPacket(ifstream& input) {
        string line;
        // After last ']', current = nullptr, so remember root separately
        Packet *root = nullptr, *current = nullptr;

        while (getline(input, line)) {
            if (line.length() == 0) { // blank line, move onto next pair
                continue;
            }

            istringstream lineStream(line);
            char c;
            while ((c = lineStream.peek()) != EOF) {
                if (c == ',') lineStream.ignore(1);
                else if (c == '[') {
                    if (current == nullptr) { // top-level packets
                        root = current = new Packet(current);
                    }
                    else { // element of current packet
                        current = current->add(new Packet(current));
                    }
                    lineStream.ignore(1);
                }
                else if (c == ']') {
                    assert(current != nullptr);
                    current = current->parent;
                    lineStream.ignore(1);
                }
                else {
                    assert(current != nullptr);
                    int num;
                    if (!(lineStream >> num)) throw invalid_argument("Malformed line!");
                    current->add(new Packet(current, num));
                }
            }

            return root;
        }

        return nullptr;
    }

    void part1() {
        ifstream input{ "Day13.txt" };

        Packet* p1 = nullptr;
        Packet* p2 = nullptr;
        int idx = 1;

        int sum = 0;

        while ((p1 = readPacket(input)) != nullptr) {
            assert((p2 = readPacket(input)) != nullptr);
            if (*p1 < *p2) {
                sum += idx;
            }

            idx++;
            delete p1;
            delete p2;
        }

        cout << sum << endl; // 6235
    }

    void part2() {
        ifstream input{ "Day13.txt" };

        Packet* div1 = new Packet(nullptr, 2);
        Packet* div2 = new Packet(nullptr, 6);

        // Divider packets
        vector<Packet*> packets{ div1, div2 };


        Packet* p = nullptr;

        while ((p = readPacket(input)) != nullptr) {
            packets.push_back(p);
        }

        auto packetPtrComparator = [](Packet* const& p1, Packet* const& p2) {return *p1 < *p2; };

        std::sort(packets.begin(), packets.end(), packetPtrComparator);

        auto div1Pos = std::find(packets.begin(), packets.end(), div1);
        assert(div1Pos != packets.end());
        auto div2Pos = std::find(packets.begin(), packets.end(), div2);
        assert(div2Pos != packets.end());

        // +1 to account for 0 vs 1 indexing
        cout << (div1Pos - packets.begin() + 1) * (div2Pos - packets.begin() + 1) << endl; // 22866

        // contains div1 and div2
        for (Packet* p : packets) delete p;
    }

    int main() {
        part2();

        return 0;
    }

}