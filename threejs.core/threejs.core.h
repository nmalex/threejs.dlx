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

	virtual const std::vector<float>& getVerts() const = 0;
	virtual const std::vector<int>& getFaces() const = 0;
	virtual const std::vector<float>& getNormals() const = 0;
	virtual const std::vector<float>& getUvs() const = 0;

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

	std::vector<int> m_indexVec;
	std::string m_indexType;
	bool m_indexedFaces;

	std::vector<float> m_uvVec;

public:
	CThreeBufferGeometry() {
	}

	virtual ~CThreeBufferGeometry() {
	}

	virtual const std::vector<float>& getVerts() const {
		return this->m_positionVec;
	}

	virtual const std::vector<int>& getFaces() const {
		return this->m_indexVec;
	}

	virtual const std::vector<float>& getNormals() const {
		return this->m_normalVec;
	}

	virtual const std::vector<float>& getUvs() const {
		return this->m_uvVec;
	}

	virtual const char* getType() const override {
		return "CThreeBufferGeometry";
	}

	friend class CThreeParser;
};

CThreeBufferGeometry* ParseThreeBufferGeometry(std::string input_filename);
