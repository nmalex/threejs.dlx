#pragma once

#include <vector>
#include <triobj.h>

// Linked list of vertex normals
class VNormal
{
public:
	Point3 norm;
	DWORD smooth;
	VNormal *next;
	BOOL init;

	VNormal() { smooth = 0; next = NULL; init = FALSE; norm = Point3(0, 0, 0); }
	VNormal(Point3 &n, DWORD s) { next = NULL; init = TRUE; norm = n; smooth = s; }
	~VNormal() { delete next; }
	void AddNormal(Point3 &n, DWORD s);
	Point3 &GetNormal(DWORD s);
	void Normalize();
};

void GetVertexNormals(Mesh *mesh, std::vector<float> &normalVec);