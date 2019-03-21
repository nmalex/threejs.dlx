#include <maxscript/maxscript.h>
#include <maxscript/foundation/numbers.h>
#include <maxscript/foundation/arrays.h>
#include <maxscript/util/listener.h>

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

		TriObject *gb = (TriObject *)ip->CreateInstance(GEOMOBJECT_CLASS_ID, Class_ID(EDITTRIOBJ_CLASS_ID, 0));
		INode* node = ip->CreateObjectNode(gb);
		node->SetName(nodename);
		sceneRoot->AttachChild(node);

		Mesh& mesh = gb->GetMesh();

		int tvertCount = mesh.getNumTVerts();
		UVVert* uvVertPtr = mesh.mapVerts(2);
		int faceCount = mesh.getNumFaces();
		TVFace* uvFacePtr = mesh.mapFaces(2);
		DWORD* tverts = uvFacePtr->getAllTVerts();

		// wchar_t buffer[8192];
		// wsprintf(buffer, L"%d, %d\n", idx++, verts.size());
		// the_listener->edit_stream->wputs(buffer);
		// the_listener->edit_stream->flush();

		mesh.setNumVerts(verts.size() / 3);
		mesh.setNumFaces(verts.size() / 9);

		int vert_idx = 0;
		int face_idx = 0;
		auto iv = verts.begin();
		while (iv != verts.end()) {
			float x = *iv++;
			float y = *iv++;
			float z = *iv++;

			mesh.setVert(vert_idx++, x, y, z);
			if (vert_idx % 3 == 0) {
				mesh.faces[face_idx++].setVerts(vert_idx - 3, vert_idx - 2, vert_idx - 1);
			}

			// wsprintf(buffer, L"%d, %d, %d\n", x, y, z);
			// the_listener->edit_stream->wputs(buffer);
			// the_listener->edit_stream->flush();
		}

		// mesh.setNormal()

		//mesh.faces[0].setVerts(0, 1, 2);
		//mesh.faces[0].setEdgeVisFlags(1, 1, 0);
		//mesh.faces[0].setSmGroup(2);
		//mesh.faces[1].setVerts(3, 1, 0);
		//mesh.faces[1].setEdgeVisFlags(1, 1, 0);
		//mesh.faces[1].setSmGroup(2);
		//mesh.faces[2].setVerts(0, 2, 3);
		//mesh.faces[2].setEdgeVisFlags(1, 1, 0);
		//mesh.faces[2].setSmGroup(4);

		mesh.buildNormals();

		mesh.InvalidateTopologyCache();
		mesh.InvalidateGeomCache();

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

			fprintf_s(pFile, "{\n"); // json

			fprintf_s(pFile, "     \"uuid\": \"%s\",\n", uuidBuf);
			fprintf_s(pFile, "     \"type\": \"BufferGeometry\",\n");

			fprintf_s(pFile, "     \"data\": {\n"); // data

			fprintf_s(pFile, "        \"attributes\": {\n"); // attributes

			// print verts here
			{
				fprintf_s(pFile, "            \"position\": {\n"); // position
				fprintf_s(pFile, "                \"itemSize\": 3,\n");
				fprintf_s(pFile, "                \"type\": \"Float32Array\",\n");
				fprintf_s(pFile, "                \"array\": [");

				for (int i = 0; i < numVerts; i++) {
					if (i != 0) {
						fprintf_s(pFile, ",");
					}
					fprintf_s(pFile, "%f,%f,%f", mesh.verts[i].x, mesh.verts[i].y, mesh.verts[i].z);
				}

				fprintf_s(pFile, "                ],\n");
				fprintf_s(pFile, "                \"normalized\": false\n");
				fprintf_s(pFile, "            },\n"); // end of position
			}

			// print normals here
			{
				fprintf_s(pFile, "            \"normal\": {\n"); // normal
				fprintf_s(pFile, "                \"itemSize\": 3,\n");
				fprintf_s(pFile, "                \"type\": \"Float32Array\",\n");
				fprintf_s(pFile, "                \"array\": [");

				PrintVertexNormals(pFile, &mesh);

				fprintf_s(pFile, "                ],\n");
				fprintf_s(pFile, "                \"normalized\": false\n");
				fprintf_s(pFile, "            },\n"); // end of normal
			}

			// print uv coords here
			{
				fprintf_s(pFile, "            \"uv\": {\n"); // uv
				fprintf_s(pFile, "                \"itemSize\": 2,\n");
				fprintf_s(pFile, "                \"type\": \"Float32Array\",\n");
				fprintf_s(pFile, "                \"array\": [");

				UVVert* tVertPtr1 = mesh.mapVerts(1);
				for (int i = 0; i < numVerts; i++) {
					if (i != 0) {
						fprintf_s(pFile, ",");
					}
					fprintf_s(pFile, "%f,%f", tVertPtr1[i].x, tVertPtr1[i].y);
				}

				fprintf_s(pFile, "                ],\n");
				fprintf_s(pFile, "                \"normalized\": false\n");
				fprintf_s(pFile, "            }\n"); // end of uv
			}

			// print uv2 coords here
			{
				fprintf_s(pFile, "            \"uv2\": {\n"); // uv2
				fprintf_s(pFile, "                \"itemSize\": 2,\n");
				fprintf_s(pFile, "                \"type\": \"Float32Array\",\n");
				fprintf_s(pFile, "                \"array\": [");

				UVVert* tVertPtr2 = mesh.mapVerts(2);
				for (int i = 0; i < numVerts; i++) {
					if (i != 0) {
						fprintf_s(pFile, ",");
					}
					fprintf_s(pFile, "%f,%f", tVertPtr2[i].x, tVertPtr2[i].y);
				}

				fprintf_s(pFile, "                ],\n");
				fprintf_s(pFile, "                \"normalized\": false\n");
				fprintf_s(pFile, "            }\n"); // end of uv2
			}

			// print index here
			if (mesh.numFaces > 0) {
				fprintf_s(pFile, "            ,\n");
				fprintf_s(pFile, "            \"index\": {\n"); // index
				fprintf_s(pFile, "                \"type\": \"Uint16Array\",\n");
				fprintf_s(pFile, "                \"array\": [");

				for (int i = 0; i < mesh.numFaces; i++) {
					if (i != 0) {
						fprintf_s(pFile, ",");
					}
					fprintf_s(pFile, "%d,%d,%d", mesh.faces[i].getVert(0), mesh.faces[i].getVert(1), mesh.faces[i].getVert(2));
				}

				fprintf_s(pFile, "            }\n"); // end of index
			}

			fprintf_s(pFile, "        }\n"); // end of attributes

			fprintf_s(pFile, "    }\n"); // end of data

			fprintf_s(pFile, "}"); // end of json
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
