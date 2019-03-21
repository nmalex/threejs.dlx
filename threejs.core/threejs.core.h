#pragma once

#include <vector>
#include <map>
#include <string>
#include <fstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

class CThreeParser;

class CThreeGeometry {
public:
	CThreeGeometry() {
	}

	virtual std::vector<float>& getVerts() = 0;

	virtual bool hasIndexedFaces() const = 0;
	virtual void hasIndexedFaces(bool value) = 0;
	virtual std::vector<int>& getFaces() = 0;

	virtual std::vector<float>& getNormals() = 0;

	virtual bool hasColor() const = 0;
	virtual void hasColor(bool value) = 0;
	virtual std::vector<float>& getColor() = 0;

	virtual bool hasUvs() const = 0;
	virtual void hasUvs(bool value) = 0;
	virtual std::vector<float>& getUvs() = 0;

	virtual bool hasUvs2() const = 0;
	virtual void hasUvs2(bool value) = 0;
	virtual std::vector<float>& getUvs2() = 0;

	virtual const char* getType() const {
		return "CThreeGeometry";
	}

	friend class CThreeParser;
};
typedef CThreeGeometry* CThreeGeometryRef;
typedef std::map<std::string, CThreeGeometryRef> CThreeGeometryMap;

class CThreeBufferGeometry : public CThreeGeometry {
private:
	int m_positionItemSize;
	std::string m_positionType;
	std::vector<float> m_positionVec;
	bool m_positionNormalized;

	int m_normalItemSize;
	std::string m_normalType;
	std::vector<float> m_normalVec;
	bool m_normalNormalized;

	int m_colorItemSize;
	std::string m_colorType;
	std::vector<float> m_colorVec;
	bool m_colorNormalized;
	bool m_hasColor;

	std::vector<int> m_indexVec;
	std::string m_indexType;
	bool m_indexedFaces;

	std::vector<float> m_uvVec;
	bool m_hasUvs;

	std::vector<float> m_uv2Vec;
	bool m_hasUvs2;

public:
	CThreeBufferGeometry() {
		m_hasUvs = false;
		m_hasUvs2 = false;
		m_indexedFaces = false;
	}

	virtual ~CThreeBufferGeometry() {
	}

	virtual std::vector<float>& getVerts() {
		return this->m_positionVec;
	}

	virtual bool hasIndexedFaces() const {
		return this->m_indexedFaces;
	}

	virtual void hasIndexedFaces(bool value) {
		this->m_indexedFaces = value;
	}

	virtual std::vector<int>& getFaces() {
		return this->m_indexVec;
	}

	virtual std::vector<float>& getNormals() {
		return this->m_normalVec;
	}

	virtual bool hasColor() const {
		return this->m_hasColor;
	}

	virtual void hasColor(bool value) {
		this->m_hasColor = value;
	}

	virtual std::vector<float>& getColor() {
		return this->m_colorVec;
	}

	virtual bool hasUvs() const {
		return this->m_hasUvs;
	}

	virtual void hasUvs(bool value) {
		this->m_hasUvs = value;
	}

	virtual std::vector<float>& getUvs() {
		return this->m_uvVec;
	}

	virtual bool hasUvs2() const {
		return this->m_hasUvs2;
	}

	virtual void hasUvs2(bool value) {
		this->m_hasUvs2 = value;
	}

	virtual std::vector<float>& getUvs2() {
		return this->m_uv2Vec;
	}

	virtual const char* getType() const override {
		return "CThreeBufferGeometry";
	}

	friend class CThreeParser;
};

CThreeBufferGeometry* ParseThreeBufferGeometry(std::string input_filename);
void SerializeThreeBufferGeometry(std::string uuid, std::string output_filename, CThreeBufferGeometry* geometry);
