#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <numeric>
#include <algorithm>

using namespace std;

namespace day19 {
	enum Resource { ORE, CLAY, OBSIDIAN, GEODE };
	// for easy iteration
	// TODO: Determines search order, get baseline faster by prioritising geode?
	const vector<Resource> RESOURCES{ ORE, CLAY, OBSIDIAN, GEODE };

	constexpr int divRoundUp(int numerator, int denominator) {
		// efficient, since divide on most architectures returns the quotient anyway
		return numerator / denominator + (numerator % denominator != 0);
	}

	/*
	Note: Sum of the k terms: n, n+1, n+2, ..., n+k-1 = k(n + (n+k-1))/2 = k(2n + k - 1)/2
	*/
	constexpr int sumUp(int from, int count) {
		return count <= 0 ? 0 : (count * (2 * from + count - 1)) / 2;
	}

	class Blueprint {
	public:
		int id;
		
		int oreRobotOreCost;
		
		int clayRobotOreCost;
		
		int obsidianRobotOreCost;
		int obsidianRobotClayCost;

		int geodeRobotOreCost;
		int geodeRobotObsidianCost;

		// Derived, maximum of each resource needed to build any miner (except geode).
		// Once this many obtained, already guaranteed to have enough resources each minute.
		map<Resource, int> maxMiners;

		Blueprint(
			int id,
			int oreRobotOreCost,
			int clayRobotOreCost,
			int obsidianRobotOreCost,
			int obsidianRobotClayCost,
			int geodeRobotOreCost,
			int geodeRobotObsidianCost
		) : id(id),
			oreRobotOreCost(oreRobotOreCost),
			clayRobotOreCost(clayRobotOreCost),
			obsidianRobotOreCost(obsidianRobotOreCost),
			obsidianRobotClayCost(obsidianRobotClayCost),
			geodeRobotOreCost(geodeRobotOreCost),
			geodeRobotObsidianCost(geodeRobotObsidianCost) {
			maxMiners[ORE] = max({geodeRobotOreCost, clayRobotOreCost, obsidianRobotOreCost, geodeRobotOreCost});
			maxMiners[CLAY] = obsidianRobotClayCost;
			maxMiners[OBSIDIAN] = geodeRobotObsidianCost;
		}

		void advance(int t, const map<Resource, int>& miners, map<Resource, int>& resources) const {
			for (const auto& [resource, minerCount] : miners) {
				resources[resource] += minerCount * t;
			}
		}

		void build(Resource miner, map<Resource, int>& miners, map<Resource, int>& resources) const {
			switch (miner) {
			case ORE:
				resources[ORE] -= oreRobotOreCost;
				miners[ORE]++;
				return;
			case CLAY:
				resources[ORE] -= clayRobotOreCost;
				miners[CLAY]++;
				return;
			case OBSIDIAN:
				resources[ORE] -= obsidianRobotOreCost;
				resources[CLAY] -= obsidianRobotClayCost;
				miners[OBSIDIAN]++;
				return;
			case GEODE:
				resources[ORE] -= geodeRobotOreCost;
				resources[OBSIDIAN] -= geodeRobotObsidianCost;
				miners[GEODE]++;
				return;
			}
		}

		// allows using the same maps the whole time, by reversing actions, to avoid creating/copying memory all the time
		void destory(Resource miner, map<Resource, int>& miners, map<Resource, int>& resources) const {
			switch (miner) {
			case ORE:
				resources[ORE] += oreRobotOreCost;
				miners[ORE]--;
				return;
			case CLAY:
				resources[ORE] += clayRobotOreCost;
				miners[CLAY]--;
				return;
			case OBSIDIAN:
				resources[ORE] += obsidianRobotOreCost;
				resources[CLAY] += obsidianRobotClayCost;
				miners[OBSIDIAN]--;
				return;
			case GEODE:
				resources[ORE] += geodeRobotOreCost;
				resources[OBSIDIAN] += geodeRobotObsidianCost;
				miners[GEODE]--;
				return;
			}
		}

		bool isRedundant(const Resource& miner, map<Resource, int>& miners) const {
			if (miner == GEODE) return false;
			else return miners[miner] >= maxMiners.at(miner);
		}

		/*
			Time to build the chosen miner next (i.e. can't first build something else to allow building that).
			-1 if not possible with existing miners/resources.
			Does not include the 1 minute to actually build the miner.
		*/
		int getTimeToBuild(Resource miner, map<Resource, int>& miners, map<Resource, int>& resources) const {
			// max 0 to handle case where we already have the necessary resources
			switch (miner) {
			case ORE: {
				int delta = oreRobotOreCost - resources[ORE];
				return delta <= 0 ? 0 : divRoundUp(delta, miners[ORE]);
			} 
			case CLAY: {
				int delta = clayRobotOreCost - resources[ORE];
				return delta <= 0 ? 0 : divRoundUp(delta, miners[ORE]);
			}
			case OBSIDIAN: {
				int oreDelta = obsidianRobotOreCost - resources[ORE];
				int oreDelay = oreDelta <= 0 ? 0 : divRoundUp(oreDelta, miners[ORE]);

				int clayDelta = obsidianRobotClayCost - resources[CLAY];
				if (clayDelta <= 0) return oreDelay;
				else if (miners[CLAY] == 0) return -1; // not yet possible
				else return max(oreDelay, divRoundUp(clayDelta, miners[CLAY]));
			}
			case GEODE: {
				int oreDelta = geodeRobotOreCost - resources[ORE];
				int oreDelay = oreDelta <= 0 ? 0 : divRoundUp(oreDelta, miners[ORE]);

				int obsidianDelta = geodeRobotObsidianCost - resources[OBSIDIAN];
				if (obsidianDelta <= 0) return oreDelay;
				else if (miners[OBSIDIAN] == 0) return -1; // not yet possible
				else return max(oreDelay, divRoundUp(obsidianDelta, miners[OBSIDIAN]));
			}
			default: throw invalid_argument("Unknown resource case " + to_string(miner));
			}
		}

		// Should really be const, but then need to initialise maps with a 0 for each entry
		int upperBoundScore(map<Resource, int>& miners, map<Resource, int>& resources, int timeRemaining) const {
			int initialGeodes = resources[GEODE];
			
			// to avoid modification
			int oreMiners = miners[ORE];
			int clayMiners = miners[CLAY];
			int obsidianMiners = miners[OBSIDIAN];
			int geodeMiners = miners[GEODE];

			// reach 1 clay miner if none already
			if (clayMiners == 0) {
				int ore = resources[ORE];
				// eagerly decrements, for cost of building 1 obsidian miner
				while (--timeRemaining > 0 && ore < clayRobotOreCost) {
					ore += oreMiners;
					oreMiners++;
				}
				clayMiners = 1;
				if (timeRemaining <= 0) return initialGeodes;
			}

			// reach 1 obsidian miner if none already
			if (obsidianMiners == 0) {
				int clay = resources[CLAY];
				// eagerly decrements, for cost of building 1 obsidian miner
				while (--timeRemaining > 0 && clay < obsidianRobotClayCost) {
					clay += clayMiners;
					clayMiners++;
				}
				obsidianMiners = 1;
				if (timeRemaining <= 0) return initialGeodes;
			}

			// reach 1 geode miner if none already
			if (geodeMiners == 0) {
				int obsidian = resources[OBSIDIAN];
				// eagerly decrements, for cost of building 1 geode miner
				while (--timeRemaining > 0 && obsidian < geodeRobotObsidianCost) {
					obsidian += obsidianMiners;
					obsidianMiners++;
				}
				geodeMiners = 1;
			}

			// assume new geode miner produced every remaining minute
			return initialGeodes + sumUp(geodeMiners, timeRemaining);
		}

		void search(map<Resource, int>& miners, map<Resource, int>& resources, int timeRemaining, int& bestScore) const {
			if (upperBoundScore(miners, resources, timeRemaining) <= bestScore) return; // not going to find better result

			// consider each possible choice for next miner to build
			bool terminal = true;
			int maxBuildTime = timeRemaining - 2; // 1 min to build, 1 min to actually contribute something
			for (const auto& resource : RESOURCES) {
				if (isRedundant(resource, miners)) continue;

				int timeToBuild = getTimeToBuild(resource, miners, resources);
				if (timeToBuild == -1 || timeToBuild > maxBuildTime) continue; // not viable

				// actually try this case
				terminal = false;
				int elapsed = timeToBuild + 1;

				advance(elapsed, miners, resources);
				build(resource, miners, resources);

				search(miners, resources, timeRemaining - elapsed, bestScore);

				// reverse actions (in reverse order)
				destory(resource, miners, resources);
				advance(-elapsed, miners, resources);
			}

			if (terminal) {
				// handle any remaining time
				int score = resources[GEODE] + miners[GEODE] * timeRemaining;
				if (score > bestScore) {
					bestScore = score;
				}
			}
		}
	};

	vector<Blueprint> readBlueprints() {
		ifstream input{ "Day19.txt" };
		vector<Blueprint> blueprints;

		string line;
		while (getline(input, line)) {
			istringstream iss{ line };

			int id, oreRobotOreCost, clayRobotOreCost, obsidianRobotOreCost, obsidianRobotClayCost, geodeRobotOreCost, geodeRobotObsidianCost;

			iss.ignore(strlen("Blueprint "));
			iss >> id;

			iss.ignore(strlen(": Each ore robot costs "));
			iss >> oreRobotOreCost;

			iss.ignore(strlen(" ore. Each clay robot costs "));
			iss >> clayRobotOreCost;

			iss.ignore(strlen(" ore. Each obsidian robot costs "));
			iss >> obsidianRobotOreCost;
			iss.ignore(strlen(" ore and "));
			iss >> obsidianRobotClayCost;

			iss.ignore(strlen(" clay. Each geode robot costs "));
			iss >> geodeRobotOreCost;
			iss.ignore(strlen(" ore and "));
			iss >> geodeRobotObsidianCost;

			blueprints.emplace_back(Blueprint(id, oreRobotOreCost, clayRobotOreCost, obsidianRobotOreCost, obsidianRobotClayCost, geodeRobotOreCost, geodeRobotObsidianCost));
		}

		return blueprints;
	}

	/*
	Optimisations:
		1. Making actions reversible to avoid copying maps: at least 10x faster (enough to solve in reasonable time)
		2. upper bounding score, to prune search early: another ~10x faster, solves part 1 in a couple seconds.
	*/
	void part1() {
		vector<Blueprint> blueprints = readBlueprints();
		int totalQuality = 0;

		for (const auto& blueprint : blueprints) {
			cout << "Blueprint id=" << blueprint.id << endl;
			int bestScore = 0;
			map<Resource, int> initialResources;
			map<Resource, int> initialMiners = { {ORE, 1} };
			blueprint.search(initialMiners, initialResources, 24, bestScore);
			int quality = bestScore * blueprint.id;
			cout << "High score=" << bestScore << ", quality=" << quality << endl;
			totalQuality += quality;
		}

		cout << "Total quality: " << totalQuality << endl; // 1266
	}

	/*
	Without any changes, takes ~5mins.
	Optimisation:
		Can stop making any more of miner X once we have N = greatest requirement of X for any miner.
		After that, again only takes a couple seconds.
	*/
	void part2() {
		vector<Blueprint> blueprints = readBlueprints();
		int total = 1;

		for (int i = 0; i < 3; i++) {
			const Blueprint& blueprint = blueprints[i];
			cout << "Blueprint id=" << blueprint.id << endl;

			time_t start, end;
			time(&start);
			int bestScore = 0;
			map<Resource, int> initialResources;
			map<Resource, int> initialMiners = { {ORE, 1} };
			blueprint.search(initialMiners, initialResources, 32, bestScore);
			time(&end);

			cout << "Blueprint " << blueprint.id << " took " << (double)(end - start) << endl;

			total *= bestScore;
		}

		cout << "Product of first 3 scores: " << total << endl; // 5800
	}

	void main() {
		part2();
	}
}


