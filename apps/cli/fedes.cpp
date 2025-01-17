﻿#include <iostream>
#include <float.h>

// #pragma float_control(except, on)

#include <BS_thread_pool.hpp>

#include "fedes/indexing/octree/octree.h"
#include "fedes/common/log.h"
#include "fedes/model/examples.h"
#include "fedes/model/export.h"

#include "octree.h"

/*
* @brief Prompts the user for model, Octree index parameters etc
*/
void prompt(BS::thread_pool& pool) {
	fedes::Model source, target;
	size_t model{}, max_depth{}, points_per_leaf{}, interpolation_type{};

	std::cout << "Enter example ID to use (1 - 4): \n";
	std::cin >> model;
	if (model < 1 || model > 4 || std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore();
		std::cerr << "[Error] Invalid model ID!\n";
		return;
	}
	fedes::SetExampleModels(source, target, model);
	fedes::ExportRawModel(target, ("oct-M" + std::to_string(model)));


	std::cout << "Enter maximum depth of Octree: \n";
	std::cin >> max_depth;
	std::cout << "Enter minimum points per leaf splitting threshold for Octree: \n";
	std::cin >> points_per_leaf;
	if (points_per_leaf < 0 || max_depth < 0 || std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore();
		std::cerr << "[Error] Invalid Octree parameters!\n";
		return;
	}	

	std::cout << "Parallelizing root construction\n";

	std::cout << "Enter interpolation type (1 - NPM, 2 - FOP, 3 - DMUE, 4 - ESF) \n";
	std::cin >> interpolation_type;
	if (interpolation_type < 1 || interpolation_type > 4 || std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore();
		std::cerr << "[Error] Invalid interplation type!\n";
		return;
	}

	double radius = 10.0;
	if (interpolation_type == 2) {
		std::cout << "Enter radius for Field of Points search \n";
		std::cin >> radius;
		if (radius < 0.00 || std::isinf(radius) || std::isnan(radius) || std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore();
			std::cerr << "[Error] Invalid radius value!\n";
			return;
		}
	}

	size_t min_scan_dmue = 50;
	if (interpolation_type == 3) {
		std::cout << "Enter minimum number of element scans for DMUE \n";
		std::cin >> min_scan_dmue;
		if (min_scan_dmue < 0 || std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore();
			std::cerr << "[Error] Invalid minimal leaf scan value!\n";
			return;
		}
	}

	size_t max_leaf_scan_threshold = 1000;
	if (interpolation_type == 4) {
		std::cout << "Enter max leaves to scan prior to relaxing geometry boundaries (boundary shift threshold) \n";
		std::cin >> max_leaf_scan_threshold;
		if (max_leaf_scan_threshold <= 0 || std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore();
			std::cerr << "[Error] Invalid threshold value!\n";

			return;
		}
	}

	FEDES_INFO("[CLI]: Example model {}, max depth {}, leaf split threshold {}", model, max_depth,
		points_per_leaf);

	switch (interpolation_type) {
		case 1:
			OctreeNPM(source, target, max_depth, points_per_leaf, pool);
			fedes::ExportModels(source, target, ("oct-M" + std::to_string(model)), ("oct-M" + std::to_string(model) + "-npm"));
			break;
		case 2:
			OctreeDMUFOP(source, target, max_depth, points_per_leaf, radius, pool);
			fedes::ExportModels(source, target, ("oct-M" + std::to_string(model)), ("oct-M" + std::to_string(model) + "-dmufop"));
			break;
		case 3:
			OctreeDMUE(source, target, max_depth, points_per_leaf, min_scan_dmue, pool);
			fedes::ExportModels(source, target, ("oct-M" + std::to_string(model)), ("oct-M" + std::to_string(model) + "-dmue"));
			break;
		case 4:
			OctreeESF(source, target, max_depth, points_per_leaf, max_leaf_scan_threshold, pool);
			fedes::ExportModels(source, target, ("oct-M" + std::to_string(model)), ("oct-M" + std::to_string(model) + "-esf"));
			break;
	}

	std::cout << "Source mesh: number of nodes = " << source.nodes.size() << ", number of elements = " << source.elements.size() << ", element type = " << 
		fedes::DetermineElementType(source.elements) << std::endl;

	std::cout << "Target mesh: number of nodes = " << target.nodes.size() << ", number of elements = " << target.elements.size() << ", element type = " <<
		fedes::DetermineElementType(target.elements) << std::endl;

	std::cout << "Models exported\n\n\n";
}


int main() {
#if (defined _DEBUG == 1)
	spdlog::set_level(spdlog::level::debug);
#elif (defined FEDES_VERBOSE == 1 || defined FEDES_STATS == 1) 
	spdlog::set_level(spdlog::level::info);
#endif

#if (defined _DEBUG == 1 || defined FEDES_VERBOSE == 1 || defined FEDES_STATS == 1)
	FEDES_WARN("[FEDES CLI] Running in debug or with verbose/stats output. Console logging will affect execution time of interpolations.");
#endif 
	std::ios::sync_with_stdio(false);

	BS::thread_pool pool(std::thread::hardware_concurrency());
	FEDES_INFO("Number of threads: {}", pool.get_thread_count());

	while (true) {
		prompt(pool);
	}

	return 0;
}