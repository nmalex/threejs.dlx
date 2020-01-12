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
protected:
	bool m_indexedFaces;

public:
	struct Group {
		int start;
		int count;
		int materialIndex;
	};

	CThreeGeometry() {
		m_indexedFaces = false;
	}

	virtual const std::vector<float>& getVerts() const = 0;
	virtual const std::vector<int>& getIndex() const = 0;
	virtual const std::vector<float>& getNormals() const = 0;
	virtual const std::vector<float>& getUvs() const = 0;
	virtual const std::vector<const Group*>& getGroups() const = 0;

	virtual const char* getType() const {
		return "CThreeGeometry";
	}

	bool hasIndex() const {
		return this->m_indexedFaces;
	}

	friend class CThreeParser;
};
typedef CThreeGeometry* CThreeGeometryRef;
typedef std::map<std::string, CThreeGeometryRef> CThreeGeometryMap;

class CThreeBufferGeometry : public CThreeGeometry {
private:
	std::string m_indexType;
	std::vector<int> m_indexVec;

	int m_positionItemSize;
	std::string m_positionType;
	std::vector<float> m_positionVec;
	bool m_positionNormalized;

	int m_normalItemSize;
	std::string m_normalType;
	std::vector<float> m_normalVec;
	bool m_normalNormalized;

	int m_uvItemSize;
	std::string m_uvType;
	std::vector<float> m_uvVec;
	bool m_uvNormalized;

	std::vector<const Group*> m_groups;

public:
	CThreeBufferGeometry() {
	}

	virtual ~CThreeBufferGeometry() {
	}

	virtual const std::vector<float>& getVerts() const {
		return this->m_positionVec;
	}

	virtual const std::vector<int>& getIndex() const {
		return this->m_indexVec;
	}

	virtual const std::vector<float>& getNormals() const {
		return this->m_normalVec;
	}

	virtual const std::vector<float>& getUvs() const {
		return this->m_uvVec;
	}

	virtual const std::vector<const Group*>& getGroups() const {
		return this->m_groups;
	}

	virtual const char* getType() const override {
		return "CThreeBufferGeometry";
	}

	friend class CThreeParser;
};

CThreeBufferGeometry* ParseThreeBufferGeometry(std::string input_filename);
