#include "VNormal.h"

// Add a normal to the list if the smoothing group bits overlap,
// otherwise create a new vertex normal in the list
void VNormal::AddNormal(Point3 &n, DWORD s) {
	if (!(s&smooth) && init) {
		if (next) next->AddNormal(n, s);
		else {
			next = new VNormal(n, s);
		}
	}
	else {
		norm += n;
		smooth |= s;
		init = TRUE;
	}
}

// Retrieves a normal if the smoothing groups overlap or there is
// only one in the list
Point3 &VNormal::GetNormal(DWORD s)
{
	if (smooth& s || !next) return norm;
	else return next->GetNormal(s);
}

// Normalize each normal in the list
void VNormal::Normalize() {
	VNormal *ptr = next, *prev = this;
	while (ptr)
	{
		if (ptr->smooth&smooth) {
			norm += ptr->norm;
			prev->next = ptr->next;
			delete ptr;
			ptr = prev->next;
		}
		else {
			prev = ptr;
			ptr = ptr->next;
		}
	}
	norm = ::Normalize(norm);
	if (next) next->Normalize();
}

void GetVertexNormals(Mesh *mesh, std::vector<float> &normalVec)
{
	Face *face;
	Point3 *verts;
	Point3 v0, v1, v2;
	Tab<VNormal> vnorms;
	Tab<Point3> fnorms;
	face = mesh->faces;
	verts = mesh->verts;

	vnorms.SetCount(mesh->getNumVerts());
	fnorms.SetCount(mesh->getNumFaces());

	// Compute face and vertex surface normals
	for (int i = 0; i < mesh->getNumVerts(); i++) {
		vnorms[i] = VNormal();
	}

	for (int i = 0; i < mesh->getNumFaces(); i++, face++) {
		// Calculate the surface normal
		v0 = verts[face->v[0]];
		v1 = verts[face->v[1]];
		v2 = verts[face->v[2]];
		fnorms[i] = (v1 - v0) ^ (v2 - v1);
		for (int j = 0; j<3; j++) {
			vnorms[face->v[j]].AddNormal(fnorms[i], face->smGroup);
		}
		fnorms[i] = Normalize(fnorms[i]);
	}
	for (int i = 0; i < mesh->getNumVerts(); i++) {
		vnorms[i].Normalize();
	}

	normalVec.clear();

	for (int i = 0; i < vnorms.Count(); i++) {
		auto vn = vnorms.Addr(i);

		normalVec.push_back(vn->norm.x);
		normalVec.push_back(vn->norm.y);
		normalVec.push_back(vn->norm.z);
	}
}