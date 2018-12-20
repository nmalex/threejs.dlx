#include "stdafx.h"

#include "threejs.core.h"

#include "nlohmann\json.hpp"
using json = nlohmann::json;

template <class T>
T safe_get(nlohmann::basic_json<> &json, const std::string& key, T fallback) {
	auto el = json.find(key);
	if (el != json.end()) {
		return el->get<T>();
	}
	else {
		return fallback;
	}
}

template <class T>
T safe_get(std::map<std::string, T> map, std::string key, T fallback) {
	auto el = map.find(key);
	if (el != map.end()) {
		return el->second;
	}
	else {
		return fallback;
	}
}

class CThreeParser
{
public:
	static void parse(nlohmann::basic_json<> &json, CThreeBufferGeometry* target);
};

void CThreeParser::parse(nlohmann::basic_json<> &json, CThreeBufferGeometry* target) {
	auto geom_data = json["data"];

	auto data_attributes = geom_data["attributes"];
	auto attr_position = data_attributes["position"];

	target->m_positionItemSize = attr_position["itemSize"].get<int>();
	target->m_positionType = attr_position["type"].get<std::string>();
	target->m_positionNormalized = attr_position["normalized"].get<bool>();

	auto pos_array = attr_position["array"];

	target->m_positionVec.clear();
	for (auto it = pos_array.begin(); it != pos_array.end(); ++it) {
		target->m_positionVec.push_back(it->get<float>());
	}
}

CThreeBufferGeometry* ParseThreeBufferGeometry(std::string input_filename) {
	std::ifstream i(input_filename);
	json j;
	i >> j;

	auto metadata = j["metadata"];

	auto metadata_version = metadata["version"].get<float>();
	auto metadata_type = metadata["type"].get<std::string>();
	auto metadata_generator = metadata["generator"].get<std::string>();

	if (metadata_version < 4.5) {
		return nullptr;
	}

	if (metadata_type != "BufferGeometry") {
		return nullptr;
	}

	if (metadata_generator != "BufferGeometry.toJSON") {
		return nullptr;
	}

	auto geom_uuid = j["uuid"].get<std::string>();
	auto geom_type = j["type"].get<std::string>();
	if (geom_type != "BufferGeometry") {
		return nullptr;
	}

	auto buffer_geometry_ref = new CThreeBufferGeometry();
	CThreeParser::parse(j, buffer_geometry_ref);

	return buffer_geometry_ref;
}
