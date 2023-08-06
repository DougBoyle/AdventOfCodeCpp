#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <optional>

using namespace std;

namespace day21 {

	enum Op { VALUE, PLUS, MINUS, MULT, DIV, EQUALS }; // Extra case for root node

	Op opFromChar(const char c) {
		switch (c) {
		case '+': return PLUS;
		case '-': return MINUS;
		case '*': return MULT;
		case '/': return DIV;
		default: throw invalid_argument("Unknown op: " + c);
		}
	}

	int64_t eval(const Op& op, const int64_t left, const int64_t right) {
		if (op == VALUE || op == EQUALS) throw invalid_argument("eval should not be called for special VALUE and EQUALS operations");
		switch (op) {
		case PLUS:
			return left + right;
		case MINUS:
			return left - right;
		case MULT:
			return left * right;
		case DIV:
			// Assert never fractional, significantly simplifies code
			if (left % right != 0) throw invalid_argument(to_string(right) + " does not divide " + to_string(left));
			return left / right;
		default:
			throw invalid_argument("Unhandled case " + to_string(op));
		}
	}

	class Monkey {
	private:
		string name;
		Op op;
		mutable optional<int64_t> value; // not known initially, cached once computed
		vector<string> dependsOn; // order matters for some operations
		mutable set<string> contributesTo; // filled in after

		// No default constructor, so have to use .at() and .emplace() for map operations.
		// Alternatively, could just store a map<string, Monkey*> and free them all at the end.
		Monkey(const string& name, const Op& op, const optional<int64_t>& value, const vector<string>& dependsOn)
			: name(name), op(op), value(value), dependsOn(dependsOn) {}

		void computeValue(const map<string, Monkey>& monkeys) const {
			if (op == VALUE) throw invalid_argument("VALUE monkey should already have a value, should not need computing");
			int64_t left = monkeys.at(dependsOn[0]).getValue(monkeys);
			int64_t right = monkeys.at(dependsOn[1]).getValue(monkeys);
			value = eval(op, left, right);
		}

	public:
		static Monkey fromValue(const string& name, const int64_t value) {
			return Monkey(name, Op::VALUE, value, {});
		}

		static Monkey fromOp(const string& name, const Op& op, const string& left, const string& right) {
			return Monkey(name, op, {}, { left, right });
		}

		int64_t getValue(const map<string, Monkey>& monkeys) const {
			if (!value) computeValue(monkeys);
			return *value;
		}

		void linkFromDependents(const map<string, Monkey>& monkeys) const {
			for (string child : dependsOn) monkeys.at(child).contributesTo.insert(name);
		}

		size_t getNumParents() const {
			return contributesTo.size();
		}

		void setOp(Op op) {
			this->op = op;
		}

		/* Is a tree, so can work way up rest of the tree to solve the missing value.

			root = A == ? or ? == A   => ? = A

			A = B op ?
				find A from parent
				get B value
				invert equation:
					A = B + ?   =>   ? = A - B
					A = B - ?   =>   ? = B - A
					A = B * ?   =>   ? = A / B
					A = B / ?   =>   ? = B / A

			A = ? op B
				find A from parent
				get B value
				invert equation:
					A = ? + B   => ? = A - B
					A = ? - B   => ? = A + B
					A = ? * B   => ? = A / B
					A = ? / B   => ? = A * B
		*/
		int64_t solveForThis(const map<string, Monkey>& monkeys) const {
			if (contributesTo.size() != 1) throw invalid_argument(name + " has " + to_string(contributesTo.size()) + " parents, can't solve for it.");
			
			const Monkey& parent = monkeys.at(*contributesTo.begin());
			const Monkey& leftChild = monkeys.at(parent.dependsOn[0]);
			const Monkey& rightChild = monkeys.at(parent.dependsOn[1]);

			if (&leftChild == this) {
				int64_t rightValue = rightChild.getValue(monkeys);
				if (parent.op == EQUALS) return rightValue;

				int64_t parentValue = parent.solveForThis(monkeys);
				switch (parent.op) {
				case PLUS:
					return parentValue - rightValue;
				case MINUS:
					return parentValue + rightValue;
				case MULT:
					// catch non-divisible divide
					return eval(DIV, parentValue, rightValue);
;				case DIV:
					return parentValue * rightValue;
				default:
					throw invalid_argument("Unhandled case " + to_string(op));
				}
			}
			else {
				int64_t leftValue = leftChild.getValue(monkeys);
				if (parent.op == EQUALS) return leftValue;

				int64_t parentValue = parent.solveForThis(monkeys);
				switch (parent.op) {
				case PLUS:
					return parentValue - leftValue;
				case MINUS:
					return leftValue - parentValue;
				case MULT:
					// catch non-divisible divide
					return eval(DIV, parentValue, leftValue);
				;case DIV:
					// catch non-divisible divide
					return eval(DIV, leftValue, parentValue);
				default:
					throw invalid_argument("Unhandled case " + to_string(op));
				}
			}
		}
	};

	map<string, Monkey> readMonkeys(ifstream& input) {
		map<string, Monkey> monkeys;

		string line;
		while (getline(input, line)) {
			istringstream iss{ line };


			string name;
			getline(iss, name, ':');
			iss >> ws;

			if (isdigit(iss.peek())) {
				int64_t value;
				iss >> value;
				monkeys.emplace(name, Monkey::fromValue(name, value));
			}
			else {
				string left, right;
				char op;
				iss >> left >> op >> right;
				monkeys.emplace(name, Monkey::fromOp(name, opFromChar(op), left, right));
			}
		}

		return monkeys;
	}

	void part1() {
		ifstream input{ "Day21.txt" };
		if (!input) throw invalid_argument("Failed to open Day21.txt");

		map<string, Monkey> monkeys = readMonkeys(input);

		cout << "Root monkey shouts: " << monkeys.at("root").getValue(monkeys) << endl; // 21208142603224
	}

	void part2() {
		ifstream input{ "Day21.txt" };
		if (!input) throw invalid_argument("Failed to open Day21.txt");

		map<string, Monkey> monkeys = readMonkeys(input);

		// link parents
		for (const auto& [_, monkey] : monkeys) monkey.linkFromDependents(monkeys);

		// identify if DAG - nodes with >1 parent
		bool tree = true;
		for (const auto& [name, monkey] : monkeys) {
			size_t parents = monkey.getNumParents();
			if (parents > 1) throw invalid_argument(name + " has " + to_string(parents) + " parent nodes. DAG not tree");
		}
		cout << "Graph is a tree" << endl;

		monkeys.at("root").setOp(EQUALS);
		const Monkey& human = monkeys.at("humn");
		int64_t humanValue = human.solveForThis(monkeys);

		cout << "Correct leaf value: " << humanValue << endl; // 3882224466191
	}

	void main() {
		time_t start, end;
		time(&start);
		part2();
		time(&end);

		cout << "Elapsed: " << (double)(end - start) << endl;
	}
}