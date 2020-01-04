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

	auto geom_data_index = geom_data["index"];

	auto data_attributes = geom_data["attributes"];

	auto attr_position = data_attributes["position"];
	if (!attr_position.is_null()) {
		target->m_positionItemSize = attr_position["itemSize"].get<int>();
		target->m_positionType = attr_position["type"].get<std::string>();
		target->m_positionNormalized = attr_position["normalized"].get<bool>();

		auto pos_array = attr_position["array"];

		target->m_positionVec.clear();
		for (auto it = pos_array.begin(); it != pos_array.end(); ++it) {
			target->m_positionVec.push_back(it->get<float>());
		}
	}

	auto attr_normal = data_attributes["normal"];
	if (!attr_normal.is_null()) {
		target->m_normalItemSize = attr_normal["itemSize"].get<int>();
		target->m_normalType = attr_normal["type"].get<std::string>();
		target->m_normalNormalized = attr_normal["normalized"].get<bool>();

		auto norm_array = attr_normal["array"];

		target->m_normalVec.clear();
		for (auto it = norm_array.begin(); it != norm_array.end(); ++it) {
			target->m_normalVec.push_back(it->get<float>());
		}
	}

	auto attr_uv = data_attributes["uv"];
	if (!attr_uv.is_null()) {
		target->m_uvItemSize = attr_uv["itemSize"].get<int>();
		target->m_uvType = attr_uv["type"].get<std::string>();
		target->m_uvNormalized = attr_uv["normalized"].get<bool>();

		auto norm_array = attr_uv["array"];

		target->m_uvVec.clear();
		for (auto it = norm_array.begin(); it != norm_array.end(); ++it) {
			target->m_uvVec.push_back(it->get<float>());
		}
	}

	if (!geom_data_index.is_null()) {
		target->m_indexedFaces = true;
		target->m_indexType = geom_data_index["type"].get<std::string>();

		auto index_array = geom_data_index["array"];

		target->m_indexVec.clear();
		for (auto it = index_array.begin(); it != index_array.end(); ++it) {
			target->m_indexVec.push_back(it->get<int>());
		}

		auto n = 1;
	} else {
		target->m_indexedFaces = false;
	}
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
