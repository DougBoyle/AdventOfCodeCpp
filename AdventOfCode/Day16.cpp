#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>

using namespace std;

namespace day16 {

	struct Valve {
		const string key;
		const int flow;
		map<string, int> tunnels; // key -> path weight (default 1)

		Valve(string key, int flow) : key(key), flow(flow) {}
	};

	void readNode(ifstream& input, map<string, Valve>& valves) {
		string line;
		while (getline(input, line)) {
			istringstream iss{ line };

			// Valve XD has flow rate=10; tunnels lead to valves AB, NQ, VT, SC, MU
			// Valve VM has flow rate=18; tunnel leads to valve HQ
			iss.ignore(strlen("Valve "));
			string key;
			if (!(iss >> key)) return;

			iss.ignore(strlen(" has flow rate="));
			int flow;
			iss >> flow;
			// emplace returns pair {iterator to element, bool if inserted or existing element updated}
			Valve& valve = valves.emplace(key, Valve(key, flow)).first->second;

			iss.ignore(strlen("; "));

			string placeholder;
			for (int i = 0; i < 4; i++) iss >> placeholder;

			string tunnel;
			iss >> ws;
			while (getline(iss, tunnel, ',')) {
				valve.tunnels[tunnel] = 1;
				iss >> ws; // skip whitespace after comma
			}
		}
	}

	void deleteIntermediates(map<string, Valve>& valves) {
		auto it = valves.begin();
		while (it != valves.end()) {
			auto& [removedKey, removedValve] = *it;
			if (removedValve.flow > 0) {
				it++;
				continue; // useful, keep
			}

			// 0 flow valve, delete and add updated paths to other nodes
			for (auto& [updatedKey, updatedValve] : valves) {
				if (updatedKey == removedKey) continue; // the valve being deleted
				map<string, int>& updatedTunnels = updatedValve.tunnels;

				if (!updatedTunnels.contains(removedKey)) continue; // no path via the removed valve
				int weightBefore = updatedTunnels[removedKey];

				// expand all paths going via the removed key
				for (const auto& [tunnel, weightAfter] : removedValve.tunnels) {
					if (tunnel == updatedKey) continue; // no point making self loops

					int newWeight = weightBefore + weightAfter;
					if (!updatedTunnels.contains(tunnel) || updatedTunnels[tunnel] > newWeight) {
						updatedTunnels[tunnel] = newWeight;
					}
				}

				// remove the original path
				updatedTunnels.erase(removedKey);
			}

			// don't delete AA from top-level list (just intermediate paths), we start there
			if (removedKey != "AA") it = valves.erase(it);
			else it++;
		}
	}

	/*
		Incrementally consider all shortest paths from any i -> j using just intermediates { 1, ..., k }.
		For k+1, if this allows a new shortest path, it must be made of segments i -> k+1 and k+1 -> j.
		Note: This implementation does not include self loops (weight 0, but useless)
	*/
	void floydWarshall(map<string, Valve>& graph, map<string, map<string, int>>& shortestDistances) {
		// initially, shortest paths are just the direct paths
		for (const auto& [key, valve] : graph) {
			for (const auto& [destination, weight] : valve.tunnels) {
				shortestDistances[key][destination] = weight;
			}
		}

		for (const auto& [key, _] : graph) {
			for (const auto& [start, _] : graph) {
				if (key == start) continue; // just an existing path
				for (const auto& [end, _] : graph) {
					if (key == end || start == end) continue; // just an existing path or self loop
					if (!shortestDistances[start].contains(key) || !shortestDistances[key].contains(end)) continue;
					int newDistance = shortestDistances[start][key] + shortestDistances[key][end];

					if (!shortestDistances[start].contains(end) || shortestDistances[start][end] > newDistance) {
						shortestDistances[start][end] = newDistance;
					}
				}
			}
		}
	}

	/*
	* Before: Currently at valve, and it is either on or has 0 flow, and everything in 'visited' is on (in order visited).
	 */
	void dfs(
		const map<string, Valve>& valves,
		const map<string, map<string, int>>& shortestPaths,
		set<string>& visited,
		int& bestScore,
		int currentScore,
		int remainingFlow,
		int remainingTime,
		const Valve& valve
	) {
		int maxDistance = remainingTime - 2; // at least 1 min to turn on + 1 min to accumulate any flow
		if (currentScore + maxDistance * remainingFlow <= bestScore) return; // even if all switched on, can't beat top score

		const string& currentKey = valve.key;
		visited.insert(currentKey);

		bool progressing = false;
		for (const auto& [next, weight] : shortestPaths.at(currentKey)) {
			if (weight > maxDistance || find(visited.begin(), visited.end(), next) != visited.end()) continue;

			progressing = true; // not a leaf

			int newTime = remainingTime - weight - 1;
			const Valve& nextValve = valves.at(next);
			int remainingFlowAfter = remainingFlow - nextValve.flow;
			dfs(valves, shortestPaths, visited, bestScore, currentScore + nextValve.flow * newTime, remainingFlowAfter, newTime, nextValve);
		}

		// in case we ended up skipping all of them (either already visited or no time left)
		if (!progressing) bestScore = max(currentScore, bestScore);

		visited.erase(currentKey);
	}

	/*
	* valve1 = node we've arrived at, need to make a decision
	* valve2 = node other player is going towards, currently distance2 >= 0 away (distance2 include switching it on)
	* visited = by either player. one player cannot pick a node as soon as the other is heading towards it,
	*	so also allow a 'do nothing' state, falls through to regular dfs for the other person.
	* distance2 < remainingTime, else wouldn't have picked it
	*/ 
	// alternative - rather than switching between, do entire search for 1st person then another whole search, with updated 'visited' list
	// also - list itself not useful. doing many linear scans, so replace with a set?
	void dfs2(
		const map<string, Valve>& valves,
		const map<string, map<string, int>>& shortestPaths,
		set<string>& visited,
		int& bestScore,
		int currentScore,
		int remainingFlow,
		int remainingTime,
		const Valve& valve1,
		const Valve& valve2,
		int distance2
	) {
		// effectively a prefix of the search space
		if (remainingTime > 22) {
			cout << "T=" << remainingTime << ": v1=" << valve1.key << ", v2=" << valve2.key 
				<< ", score=" << currentScore << ", best=" << bestScore << endl;
		}
		int maxDistance = remainingTime - 2; // at least 1 min to turn on + 1 min to accumulate any flow
		if (currentScore + maxDistance * remainingFlow <= bestScore) return; // even if all switched on, can't beat top score

		const string& currentKey = valve1.key;
		visited.insert(currentKey);

		
		bool progressing = false;

		for (const auto& [next, weight] : shortestPaths.at(currentKey)) {
			// can't also go to the one the other has chosen
			if (weight > maxDistance || next == valve2.key || find(visited.begin(), visited.end(), next) != visited.end()) continue;

			// At start, enforce second player picks the alphabetically higher first hop, to halve symmetric search space
			if (currentKey == "AA" && next < valve2.key) continue;

			progressing = true; // still able to pick nodes we can reach, don't fall through to 1-person case yet

			const Valve& nextValve = valves.at(next);
			int valueAdded = nextValve.flow * (remainingTime - weight - 1);
			int remainingFlowAfter = remainingFlow - nextValve.flow;

			// work out who will reach their destination first (and turn on the valve)
			if (weight < distance2) {
				// same person again
				int timeDelta = weight + 1; // 1 to turn on
				int newTime = remainingTime - timeDelta;
				dfs2(valves, shortestPaths, visited, bestScore, currentScore + valueAdded, remainingFlowAfter, newTime, nextValve, valve2, distance2 - timeDelta);
			}
			else {
				// switch to the other person
				int timeDelta = distance2;
				int newTime = remainingTime - timeDelta;
				dfs2(valves, shortestPaths, visited, bestScore, currentScore + valueAdded, remainingFlowAfter, newTime, valve2, nextValve, weight + 1 - timeDelta);
			}
		}

		// or we've turned on our last one, let them do the rest
		if (!progressing) dfs(valves, shortestPaths, visited, bestScore, currentScore, remainingFlow, remainingTime - distance2, valve2);

		visited.erase(currentKey);
	}

	void part1() {
		ifstream input{ "Day16.txt" };

		map<string, Valve> valves;
		readNode(input, valves);

		cout << "Original number of nodes: " << valves.size() << endl;

		deleteIntermediates(valves);

		cout << "Reduced number of nodes: " << valves.size() << endl;

		map<string, map<string, int>> shortestPaths;
		floydWarshall(valves, shortestPaths);

		int totalFlow = 0;
		for (const auto& [_, valve] : valves) totalFlow += valve.flow;

		int bestScore = -1;
		set<string> visited;
		dfs(valves, shortestPaths, visited, bestScore, 0, totalFlow, 30, valves.at("AA"));

		cout << bestScore << endl; // 2124
	}

	void part2() {
		ifstream input{ "Day16.txt" };

		map<string, Valve> valves;
		readNode(input, valves);

		cout << "Original number of nodes: " << valves.size() << endl;

		deleteIntermediates(valves);

		cout << "Reduced number of nodes: " << valves.size() << endl;

		map<string, map<string, int>> shortestPaths;
		floydWarshall(valves, shortestPaths);

		int bestScore = -1;
		set<string> visited;
		const Valve& startValve = valves.at("AA");

		int totalFlow = 0;
		for (const auto& [_, valve] : valves) totalFlow += valve.flow;

		// Possible optimisations:
		// 1. make visited a set - log(n) lookup + insert + remove, rather than O(n) lookup for each neighbour + constant insert/remove
		//		time taken = 1191 = 20mnis
		// 2. track upper bound on remaining value, cutoff early if less than maximum?
		//		only takes 15s with upper bound!
		// 3. do a full search with 1 person, then just repeat for the other with a reduced visited set
		time_t start, end;
		time(&start);
		dfs2(valves, shortestPaths, visited, bestScore, 0, totalFlow, 26, startValve, startValve, 0);
		time(&end);

		cout << "Elapsed: " << (double)(end - start) << endl;

		cout << bestScore << endl; // 2775
	}


	void main() {
		part2();
	}
}