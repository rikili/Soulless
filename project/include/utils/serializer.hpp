#pragma once

#include <fstream>
#include <iostream>
#include <utils/json.hpp>

using json = nlohmann::json;

class Serializer {
public:
	Serializer();

	static void serialize();
	static void deserialize();
};

// TODO:
// save all components
// Save with control s
// dont allow save when dying
// menu to do this ?
// add to tutorial menu. Save / Load

// NEW GAME / LOAD GAME ?
