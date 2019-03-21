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
	static void CThreeParser::parse_position(nlohmann::basic_json<> &data_attributes, CThreeBufferGeometry* target);
	static void CThreeParser::parse_normal(nlohmann::basic_json<> &data_attributes, CThreeBufferGeometry* target);
	static void CThreeParser::parse_index(nlohmann::basic_json<> &data_attributes, CThreeBufferGeometry* target);
};

void CThreeParser::parse(nlohmann::basic_json<> &json, CThreeBufferGeometry* target) {
	auto geom_data = json["data"];

	auto data_attributes = geom_data["attributes"];
	parse_position(data_attributes, target);
	parse_normal(data_attributes, target);
	parse_index(geom_data, target);
}

void CThreeParser::parse_position(nlohmann::basic_json<> &data_attributes, CThreeBufferGeometry* target) {
	auto attr_position = data_attributes["position"];

	target->m_positionItemSize = attr_position["itemSize"].get<int>();
	target->m_positionType = attr_position["type"].get<std::string>();
	target->m_positionNormalized = attr_position["normalized"].get<bool>();

	auto arr = attr_position["array"];

	target->m_positionVec.clear();
	for (auto it = arr.begin(); it != arr.end(); ++it) {
		target->m_positionVec.push_back(it->get<float>());
	}
}

void CThreeParser::parse_normal(nlohmann::basic_json<> &data_attributes, CThreeBufferGeometry* target) {
	auto attr_position = data_attributes["normal"];

	target->m_normalItemSize = attr_position["itemSize"].get<int>();
	target->m_normalType = attr_position["type"].get<std::string>();
	target->m_normalNormalized = attr_position["normalized"].get<bool>();

	auto arr = attr_position["array"];

	target->m_normalVec.clear();
	for (auto it = arr.begin(); it != arr.end(); ++it) {
		target->m_normalVec.push_back(it->get<float>());
	}
}

void CThreeParser::parse_index(nlohmann::basic_json<> &data, CThreeBufferGeometry* target) {
	if (data["index"] == nullptr) {
		target->m_indexedFaces = false;
		return;
	}

	auto index = data["index"];

	target->m_indexType = index["type"].get<std::string>();

	auto arr = index["array"];

	target->m_indexVec.clear();
	for (auto it = arr.begin(); it != arr.end(); ++it) {
		target->m_indexVec.push_back(it->get<int>());
	}

	target->m_indexedFaces = true;
}

CThreeBufferGeometry* ParseThreeBufferGeometry(std::string input_filename) {
	std::ifstream i(input_filename);
	json j;
	i >> j;

	auto geom_uuid = j["uuid"].get<std::string>();
	auto geom_type = j["type"].get<std::string>();
	if (geom_type != "BufferGeometry") {
		return nullptr;
	}

	auto buffer_geometry_ref = new CThreeBufferGeometry();
	CThreeParser::parse(j, buffer_geometry_ref);

	return buffer_geometry_ref;
}
