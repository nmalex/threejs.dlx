#include <maxscript/maxscript.h>
#include <maxscript/foundation/numbers.h>
#include <maxscript/foundation/arrays.h>
#include <maxscript/util/listener.h>

#include <mesh.h>
#include <MeshNormalSpec.h>
#include <triobj.h>

#include "threejs.core.h"

void ThreejsApiInit()
{
	//Todo: Place initialization code here. This gets called when Maxscript goes live
	//during max startup.
}

// Declare C++ function and register it with MAXScript
#include <maxscript\macros\define_instantiation_functions.h>
def_visible_primitive(threejsImportBufferGeometry, "threejsImportBufferGeometry");
def_visible_primitive(threejsExportBufferGeometry, "threejsExportBufferGeometry");

// threejsImportBufferGeometry "F:\\Temp\\86992EEF-D27B-4FEC-9EBD-A5CC9BBFEBEE.json" "Mesh001"

#define __print0(fmt) { wchar_t buffer[8192]; swprintf(buffer, 8192, fmt); the_listener->edit_stream->wputs(buffer); the_listener->edit_stream->flush(); }
#define __print1(fmt,a) { wchar_t buffer[8192]; swprintf(buffer, 8192, fmt, a); the_listener->edit_stream->wputs(buffer); the_listener->edit_stream->flush(); }
#define __print2(fmt,a,b) { wchar_t buffer[8192]; swprintf(buffer, 8192, fmt, a,b); the_listener->edit_stream->wputs(buffer); the_listener->edit_stream->flush(); }
#define __print3(fmt,a,b,c) { wchar_t buffer[8192]; swprintf(buffer, 8192, fmt, a,b,c); the_listener->edit_stream->wputs(buffer); the_listener->edit_stream->flush(); }
#define __print4(fmt,a,b,c,d) { wchar_t buffer[8192]; swprintf(buffer, 8192, fmt, a,b,c,d); the_listener->edit_stream->wputs(buffer); the_listener->edit_stream->flush(); }

Value* threejsImportBufferGeometry_cf(Value **arg_list, int count)
{
	//--------------------------------------------------------
	//Maxscript usage:
	//--------------------------------------------------------
	// Trace <json filename:String>
	check_arg_count(threejsImportBufferGeometry, 2, count);
	Value* pFilename = arg_list[0];
	Value* pNodeName = arg_list[1];

	//First example of how to type check an argument
	if (!(is_string(pFilename)))
	{
		throw RuntimeError(_T("Expected a String for the first argument.\r\nUSAGE: threejsImportBufferGeometry <filename> <node name>"));
	}

	if (!(is_string(pNodeName)))
	{
		throw RuntimeError(_T("Expected a String for the second argument.\r\nUSAGE: threejsImportBufferGeometry <filename> <node name>"));
	}

	const wchar_t* filename = pFilename->to_string();
	const wchar_t* nodename = pNodeName->to_string();

	char filenameBuf[1024];
	wcstombs(filenameBuf, filename, 1024);

	CThreeBufferGeometry* geom = ParseThreeBufferGeometry(filenameBuf);

	Interface* ip = GetCOREInterface();
	INode* sceneRoot = ip->GetRootNode();

	auto verts = geom->getVerts();
	auto norms = geom->getNormals();
	auto uvs = geom->getUvs();
	auto hasIndex = geom->hasIndex();
	auto groups = geom->getGroups();

	TriObject *gb = (TriObject *)ip->CreateInstance(GEOMOBJECT_CLASS_ID, Class_ID(EDITTRIOBJ_CLASS_ID, 0));
	INode* node = ip->CreateObjectNode(gb);
	node->SetName(nodename);
	sceneRoot->AttachChild(node);

	Mesh& mesh = gb->GetMesh();

	{
		std::vector<int> triangleVertIndices = geom->getIndex();
		int numVertices = (int)verts.size() / 3;
		int numTriangles = hasIndex ? ((int)triangleVertIndices.size() / 3) : ((int)numVertices / 3);

		__print1(L"numVertices: %d\n", numVertices);
		__print1(L"numTriangles: %d\n", numTriangles);

		mesh.setNumVerts(numVertices);

		int vert_idx = 0;
		auto iv = verts.begin();
		while (iv != verts.end()) {
			float x = *iv++;
			float y = *iv++;
			float z = *iv++;

			// __print4(L"mesh.setVert: %d, [ %f, %f, %f ]\n", vert_idx, x,y,z);
			mesh.setVert(vert_idx++, x, y, z);
		}

		// set vertex normals
		mesh.SpecifyNormals();
		MeshNormalSpec *normalSpec = mesh.GetSpecifiedNormals();
		normalSpec->ClearNormals();
		normalSpec->SetNumNormals(numVertices);

		int norm_idx = 0;
		auto _in = norms.begin();
		while (_in != norms.end()) {
			float x = *_in++;
			float y = *_in++;
			float z = *_in++;

			// __print4(L"mesh.setNorm: %d, [ %f, %f, %f ]\n", norm_idx, x, y, z);

			auto normals_i = new Point3(x, y, z);
			normalSpec->Normal(norm_idx) = normals_i->Normalize();
			normalSpec->SetNormalExplicit(norm_idx, true);

			norm_idx += 1;
		}

		// set UVs
		// TODO: multiple map channels?
		// channel 0 is reserved for vertex color, channel 1 is the default texture mapping
		mesh.setNumMaps(2);
		mesh.setMapSupport(1, TRUE);  // enable map channel
		MeshMap &map = mesh.Map(1);
		map.setNumVerts(numVertices);

		int uv_idx = 0;
		auto iuv = uvs.begin();
		while (iuv != uvs.end()) {
			float x = *iuv++;
			float y = *iuv++;

			// __print3(L"mesh.setTV: %d, [ %f, %f ]\n", uv_idx, x, y);

			UVVert &texVert = map.tv[uv_idx];
			texVert.x = x;
			texVert.y = y;
			texVert.z = 0.0f;

			uv_idx += 1;
		}

		// set triangles
		mesh.setNumFaces(numTriangles);
		normalSpec->SetNumFaces(numTriangles);
		map.setNumFaces(numTriangles);
		for (int i = 0, j = 0; i < numTriangles; i++, j += 3)
		{
			// three vertex indices of a triangle
			int v0 = hasIndex ? triangleVertIndices[j + 0] : (j + 0);
			int v1 = hasIndex ? triangleVertIndices[j + 1] : (j + 1);
			int v2 = hasIndex ? triangleVertIndices[j + 2] : (j + 2);

			// __print4(L"mesh.setFace: %d, [ %d, %d, %d ]\n", i, v0, v1, v2);

			// vertex positions
			Face &face = mesh.faces[i];
			face.setMatID(1);
			face.setEdgeVisFlags(1, 1, 1);
			face.setVerts(v0, v1, v2);

			// vertex normals
			MeshNormalFace &normalFace = normalSpec->Face(i);
			normalFace.SpecifyAll();
			normalFace.SetNormalID(0, v0);
			normalFace.SetNormalID(1, v1);
			normalFace.SetNormalID(2, v2);

			// vertex UVs
			TVFace &texFace = map.tf[i];
			texFace.setTVerts(v0, v1, v2);
		}

		auto ig = groups.begin();
		//__print1(L"importing groups: %d\n", groups.size());
		while (ig != groups.end()) {
			auto group = *ig++;
			//__print3(L"faces: %d - %d, material ID: %d\n", group->start, group->start + group->count, group->materialIndex);

			auto mtlIdx = group->materialIndex;
			for (int i = 0, c = (int)(group->start / 3); i < group->count; i += 3, c += 1) {
				mesh.setFaceMtlIndex(c, mtlIdx);
			}
		}

		mesh.InvalidateGeomCache();
		mesh.InvalidateTopologyCache();
	}

	wchar_t msgBuf[8192];
	swprintf_s(msgBuf, 8192, L"Successfully imported BufferGeometry from %s\n", filename);
	the_listener->edit_stream->wputs(msgBuf);
	the_listener->edit_stream->flush();

	return &true_value;
}

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

// Compute and print vertex normals
void PrintVertexNormals(FILE * pFile, Mesh *mesh)
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

	for (int i = 0; i < vnorms.Count(); i++) {
		auto vn = vnorms.Addr(i);
		if (i != 0) {
			fprintf_s(pFile, ",");
		}
		fprintf_s(pFile, "%f,%f,%f", vn->norm.x, vn->norm.y, vn->norm.z);
	}
}

Value* threejsExportBufferGeometry_cf(Value **arg_list, int count)
{
	//--------------------------------------------------------
	//Maxscript usage:
	//--------------------------------------------------------
	// Trace <json filename:String>
	check_arg_count(threejsExportBufferGeometry, 3, count);
	Value* pFilename = arg_list[0];
	Value* pNodeName = arg_list[1];
	Value* pUuid     = arg_list[2];

	//First example of how to type check an argument
	if (!(is_string(pFilename)))
	{
		throw RuntimeError(_T("Expected a String for the first argument.\r\nUSAGE: threejsExportBufferGeometry <filename> <node name> <uuid>"));
	}

	if (!(is_string(pNodeName)))
	{
		throw RuntimeError(_T("Expected a String for the second argument.\r\nUSAGE: threejsExportBufferGeometry <filename> <node name> <uuid>"));
	}

	if (!(is_string(pUuid)))
	{
		throw RuntimeError(_T("Expected a String for the third argument.\r\nUSAGE: threejsExportBufferGeometry <filename> <node name> <uuid>"));
	}

	const wchar_t* filename = pFilename->to_string();
	const wchar_t* nodename = pNodeName->to_string();
	const wchar_t* uuid     = pUuid->to_string();

	char filenameBuf[1024];
	wcstombs(filenameBuf, filename, 1024);

	char uuidBuf[1024];
	wcstombs(uuidBuf, uuid, 1024);

	FILE * pFile = fopen(filenameBuf, "w");
	if (!pFile) {
		wchar_t buffer[8192];
		swprintf(buffer, 8192, L"FAIL | Can't open file: %s\n", filename);
		the_listener->edit_stream->wputs(buffer);
		the_listener->edit_stream->flush();
		return &false_value;
	}

	try {
		Interface* ip = GetCOREInterface();
		INode* node = ip->GetINodeByName(nodename);

		// Get the object from the node
		Object *obj = node->EvalWorldState(ip->GetTime()).obj;
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
		{
			TriObject *tri = (TriObject *)obj->ConvertToType(ip->GetTime(), Class_ID(TRIOBJ_CLASS_ID, 0));
			auto mesh = tri->GetMesh();

			int numVerts = mesh.numVerts;
			int numFaces = mesh.numFaces;

			// wchar_t buffer[8192];
			// swprintf(buffer, 8192, L"numVerts: %d, numFaces: %d\n", numVerts, numFaces);
			// the_listener->edit_stream->wputs(buffer);

			fprintf_s(pFile, "{\n");
			fprintf_s(pFile, "     \"uuid\": \"%s\",\n", uuidBuf);
			fprintf_s(pFile, "     \"type\": \"BufferGeometry\",\n");
			fprintf_s(pFile, "     \"data\": {\n");
			fprintf_s(pFile, "        \"attributes\": {\n");
			fprintf_s(pFile, "            \"position\": {\n");
			fprintf_s(pFile, "                \"itemSize\": 3,\n");
			fprintf_s(pFile, "                \"type\": \"Float32Array\",\n");
			fprintf_s(pFile, "                \"array\": [");

//todo: print verts here
			for (int i = 0; i < numVerts; i++) {
				if (i != 0) {
					fprintf_s(pFile, ",");
				}
				fprintf_s(pFile, "%f,%f,%f", mesh.verts[i].x, mesh.verts[i].y, mesh.verts[i].z);
			}

			fprintf_s(pFile, "                ],\n");
			fprintf_s(pFile, "                \"normalized\": false\n");
			fprintf_s(pFile, "            },\n");
			fprintf_s(pFile, "            \"normal\": {\n");
			fprintf_s(pFile, "                \"itemSize\": 3,\n");
			fprintf_s(pFile, "                \"type\": \"Float32Array\",\n");
			fprintf_s(pFile, "                \"array\": [");
//todo: print normals here

			PrintVertexNormals(pFile, &mesh);

			fprintf_s(pFile, "                ],\n");
			fprintf_s(pFile, "                \"normalized\": false\n");
			fprintf_s(pFile, "            },\n");
/*
			fprintf_s(pFile, "            \"uv\": {\n");
			fprintf_s(pFile, "                \"itemSize\": 2,\n");
			fprintf_s(pFile, "                \"type\": \"Float32Array\",\n");
			fprintf_s(pFile, "                \"array\": [");
//todo: print uv coords here

			fprintf_s(pFile, "                ],\n");
			fprintf_s(pFile, "                \"normalized\": false\n");
			fprintf_s(pFile, "            }\n"); */

			fprintf_s(pFile, "            \"uv2\": {\n");
			fprintf_s(pFile, "                \"itemSize\": 2,\n");
			fprintf_s(pFile, "                \"type\": \"Float32Array\",\n");
			fprintf_s(pFile, "                \"array\": [");
//todo: print uv2 coords here
			UVVert* tVertPtr2 = mesh.mapVerts(2);
			for (int i = 0; i < numVerts; i++) {
				if (i != 0) {
					fprintf_s(pFile, ",");
				}
				fprintf_s(pFile, "%f,%f", tVertPtr2[i].x, tVertPtr2[i].y);
			}

			fprintf_s(pFile, "                ],\n");
			fprintf_s(pFile, "                \"normalized\": false\n");
			fprintf_s(pFile, "            }\n");
			fprintf_s(pFile, "        }\n");
			fprintf_s(pFile, "    }\n");
			fprintf_s(pFile, "}");

			/* 
			for (int i = 0; i < numFaces; i++) {
				swprintf(buffer, 8192, L"face[%d]: [ %d, %d, %d]\n", i, mesh.faces[i].getVert(0), mesh.faces[i].getVert(1), mesh.faces[i].getVert(2));
				the_listener->edit_stream->wputs(buffer);
			}

			UVVert* tVertPtr1 = mesh.mapVerts(1);
			for (int i = 0; i < numVerts; i++) {
				swprintf(buffer, 8192, L"tvert1[%d]: { x: %4.2f, y: %4.2f, z: %4.2f }\n", i, tVertPtr1[i].x, tVertPtr1[i].y, tVertPtr1[i].z);
				the_listener->edit_stream->wputs(buffer);
			}

			UVVert* tVertPtr2 = mesh.mapVerts(2);
			for (int i = 0; i < numVerts; i++) {
				swprintf(buffer, 8192, L"tvert2[%d]: { x: %4.2f, y: %4.2f, z: %4.2f }\n", i, tVertPtr2[i].x, tVertPtr2[i].y, tVertPtr2[i].z);
				the_listener->edit_stream->wputs(buffer);
			}

			the_listener->edit_stream->flush(); */
		}
	}
	catch (...) {
		wchar_t buffer[8192];
		swprintf(buffer, 8192, L"FAIL | Exception occurred\n");
		the_listener->edit_stream->wputs(buffer);
		the_listener->edit_stream->flush();
		fclose(pFile);
		return &false_value;
	}

	fclose(pFile);

	return &true_value;
}
