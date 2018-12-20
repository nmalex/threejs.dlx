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
	bool m_indexedFaces;

	std::vector<int> m_faces;
	std::vector<float> m_normals;
	std::vector<float> m_uvs;
	std::vector<float> m_uvs2;

public:
	CThreeBufferGeometry() {
	}

	virtual ~CThreeBufferGeometry() {
	}

	virtual const std::vector<float>& getVerts() const {
		return this->m_positionVec;
	}

	virtual const std::vector<int>& getFaces() const {
		return this->m_faces;
	}

	virtual const std::vector<float>& getNormals() const {
		return this->m_normals;
	}

	virtual const std::vector<float>& getUvs() const {
		return this->m_uvs;
	}

	virtual const std::vector<float>& getUvs2() const {
		return this->m_uvs2;
	}

	virtual const char* getType() const override {
		return "CThreeBufferGeometry";
	}

	friend class CThreeParser;
};

CThreeBufferGeometry* ParseThreeBufferGeometry(std::string input_filename);
