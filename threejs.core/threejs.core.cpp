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
	static void serialize(const char* uuid, CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j);

	static void parse_position(nlohmann::basic_json<> &data_attributes, CThreeBufferGeometry* target);
	static void parse_normal(nlohmann::basic_json<> &data_attributes, CThreeBufferGeometry* target);
	static void parse_color(nlohmann::basic_json<> &data_attributes, CThreeBufferGeometry* target);
	static void parse_index(nlohmann::basic_json<> &data_attributes, CThreeBufferGeometry* target);

	static void serialize_position(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data_attributes);
	static void serialize_normal(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data_attributes);
	static void serialize_color(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data_attributes);
	static void serialize_uv(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data_attributes);
	static void serialize_uv2(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data_attributes);
	static void serialize_index(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data);
};

void CThreeParser::parse(nlohmann::basic_json<> &json, CThreeBufferGeometry* target) {
	auto geom_data = json["data"];

	auto data_attributes = geom_data["attributes"];
	parse_position(data_attributes, target);
	parse_normal(data_attributes, target);
	parse_color(data_attributes, target);
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

void CThreeParser::parse_color(nlohmann::basic_json<> &data_attributes, CThreeBufferGeometry* target) {
	if (data_attributes["color"] == nullptr) {
		target->m_hasColor = false;
		return;
	}

	auto attr_color = data_attributes["color"];

	target->m_normalItemSize = attr_color["itemSize"].get<int>();
	target->m_normalType = attr_color["type"].get<std::string>();
	target->m_normalNormalized = attr_color["normalized"].get<bool>();

	auto arr = attr_color["array"];

	target->m_colorVec.clear();
	for (auto it = arr.begin(); it != arr.end(); ++it) {
		target->m_colorVec.push_back(it->get<float>());
	}

	target->m_hasColor = true;
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

void CThreeParser::serialize(const char* uuid, CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j) {
	j.clear();

	j["uuid"] = uuid;
	j["type"] = "BufferGeometry";

	j["metadata"] = nlohmann::json();
	j["metadata"]["generator"] = "BufferGeometry.toJSON";
	j["metadata"]["type"] = "BufferGeometry";
	j["metadata"]["version"] = 4.5;

	// json j_data;
	j["data"] = nlohmann::json();

	// json j_data_attributes;
	j["data"]["attributes"] = nlohmann::json();

	serialize_position(geometry, j["data"]["attributes"]);
	serialize_normal(geometry, j["data"]["attributes"]);
	serialize_color(geometry, j["data"]["attributes"]);
	serialize_uv(geometry, j["data"]["attributes"]);
	serialize_uv2(geometry, j["data"]["attributes"]);

	serialize_index(geometry, j["data"]);
}

void CThreeParser::serialize_position(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data_attributes) {
	j_data_attributes["position"] = nlohmann::json();
	j_data_attributes["position"]["itemSize"] = 3;
	j_data_attributes["position"]["type"] = "Float32Array";
	j_data_attributes["position"]["array"] = nlohmann::json(geometry->getVerts());
	j_data_attributes["position"]["normalized"] = false;
}

void CThreeParser::serialize_normal(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data_attributes) {
	j_data_attributes["normal"] = nlohmann::json();
	j_data_attributes["normal"]["itemSize"] = 3;
	j_data_attributes["normal"]["type"] = "Float32Array";
	j_data_attributes["normal"]["array"] = nlohmann::json(geometry->getNormals());
	j_data_attributes["normal"]["normalized"] = false;
}

void CThreeParser::serialize_color(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data_attributes) {
	if (geometry->hasColor()) {
		j_data_attributes["color"] = nlohmann::json();
		j_data_attributes["color"]["itemSize"] = 3;
		j_data_attributes["color"]["type"] = "Float32Array";
		j_data_attributes["color"]["array"] = nlohmann::json(geometry->getColor());
		j_data_attributes["color"]["normalized"] = false;
	}
}

void CThreeParser::serialize_uv(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data_attributes) {
	if (geometry->hasUvs()) {
		j_data_attributes["uv"] = nlohmann::json();
		j_data_attributes["uv"]["itemSize"] = 2;
		j_data_attributes["uv"]["type"] = "Float32Array";
		j_data_attributes["uv"]["array"] = nlohmann::json(geometry->getUvs());
		j_data_attributes["uv"]["normalized"] = false;
	}
}

void CThreeParser::serialize_uv2(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data_attributes) {
	if (geometry->hasUvs2()) {
		j_data_attributes["uv2"] = nlohmann::json();
		j_data_attributes["uv2"]["itemSize"] = 2;
		j_data_attributes["uv2"]["type"] = "Float32Array";
		j_data_attributes["uv2"]["array"] = nlohmann::json(geometry->getUvs2());
		j_data_attributes["uv2"]["normalized"] = false;
	}
}

void CThreeParser::serialize_index(CThreeBufferGeometry* geometry, nlohmann::basic_json<> &j_data) {
	if (geometry->hasIndexedFaces()) {
		j_data["index"] = nlohmann::json();
		j_data["index"]["type"] = "Uint16Array";
		j_data["index"]["array"] = nlohmann::json(geometry->getFaces());
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

void SerializeThreeBufferGeometry(std::string uuid, std::string output_filename, CThreeBufferGeometry* geometry) {
	std::ofstream o(output_filename);

	nlohmann::basic_json<> j;
	CThreeParser::serialize(uuid.c_str(), geometry, j);

	o << j.dump(4);
}
