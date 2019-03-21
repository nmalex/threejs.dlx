#include <maxscript/maxscript.h>
#include <maxscript/foundation/numbers.h>
#include <maxscript/foundation/arrays.h>
#include <maxscript/util/listener.h>

#include <triobj.h>

#include "threejs.core.h"
#include "VNormal.h"

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

	TriObject *gb = (TriObject *)ip->CreateInstance(GEOMOBJECT_CLASS_ID, Class_ID(EDITTRIOBJ_CLASS_ID, 0));
	INode* node = ip->CreateObjectNode(gb);
	node->SetName(nodename);
	sceneRoot->AttachChild(node);

	Mesh& mesh = gb->GetMesh();

	if (geom->hasIndexedFaces()) {
		// todo: implement it
	}
	else {
		auto verts = geom->getVerts();
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
		}

		int norm_idx = 0;
		auto norms = geom->getNormals();
		auto in = norms.begin();
		while (in != norms.end()) {
			float x = *in++;
			float y = *in++;
			float z = *in++;

			mesh.setNormal(norm_idx++, Point3(x, y, z));
		}

		if (geom->hasUvs()) {
			mesh.setMapSupport(1, 1);
			mesh.setNumMapVerts(1, geom->getVerts().size());
			int uv_idx = 0;
			auto uvs = geom->getUvs();
			auto iu = uvs.begin();
			while (iu != uvs.end()) {
				float x = *iu++;
				float y = *iu++;
				mesh.setMapVert(1, uv_idx, UVVert(x, y, 0.0f));
			}
		}

		if (geom->hasUvs2()) {
			mesh.setMapSupport(2, 1);
			mesh.setNumMapVerts(2, geom->getVerts().size());
			int uv_idx = 0;
			auto uvs = geom->getUvs2();
			auto iu = uvs.begin();
			while (iu != uvs.end()) {
				float x = *iu++;
				float y = *iu++;
				mesh.setMapVert(2, uv_idx, UVVert(x, y, 0.0f));
			}
		}
	}

	mesh.InvalidateTopologyCache();
	mesh.InvalidateGeomCache();

	wchar_t msgBuf[8192];
	swprintf_s(msgBuf, 8192, L"Successfully imported BufferGeometry from %s\n", filename);
	the_listener->edit_stream->wputs(msgBuf);
	the_listener->edit_stream->flush();

	return &true_value;
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

	try {
		Interface* ip = GetCOREInterface();
		INode* node = ip->GetINodeByName(nodename);

		// Get the object from the node
		Object *obj = node->EvalWorldState(ip->GetTime()).obj;
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
		{
			TriObject *tri = (TriObject *)obj->ConvertToType(ip->GetTime(), Class_ID(TRIOBJ_CLASS_ID, 0));
			auto mesh = tri->GetMesh();

			CThreeBufferGeometry geometry;

			// collect verts
			auto positionVec = geometry.getVerts();
			for (int i = 0; i < mesh.numVerts; i++) {
				positionVec.push_back(mesh.verts[i].x);
				positionVec.push_back(mesh.verts[i].y);
				positionVec.push_back(mesh.verts[i].z);
			}

			// collect normals
			GetVertexNormals(&mesh, geometry.getNormals());

			// collect faces
			if (mesh.numFaces > 0) {
				geometry.hasIndexedFaces(true);
				auto indexVec = geometry.getFaces();
				for (int i = 0; i < mesh.numFaces; i++) {
					indexVec.push_back(mesh.faces[i].getVert(0));
					indexVec.push_back(mesh.faces[i].getVert(1));
					indexVec.push_back(mesh.faces[i].getVert(2));
				}
			}
			else {
				geometry.hasIndexedFaces(false);
			}

			// collect UV coords channel#1
			if (mesh.mapSupport(1)) {
				auto uvVec = geometry.getUvs();
				geometry.hasUvs(true);
				UVVert* tVertPtr1 = mesh.mapVerts(1);
				for (int i = 0; i < mesh.numVerts; i++) {
					uvVec.push_back(tVertPtr1[i].x);
					uvVec.push_back(tVertPtr1[i].y);
				}
			}
			else {
				geometry.hasUvs(false);
			}

			// collect UV coords channel#2
			if (mesh.mapSupport(2)) {
				auto uv2Vec = geometry.getUvs2();
				geometry.hasUvs2(true);
				UVVert* tVertPtr2 = mesh.mapVerts(2);
				for (int i = 0; i < mesh.numVerts; i++) {
					uv2Vec.push_back(tVertPtr2[i].x);
					uv2Vec.push_back(tVertPtr2[i].y);
				}
			}
			else {
				geometry.hasUvs2(false);
			}

			SerializeThreeBufferGeometry(uuidBuf, filenameBuf, &geometry);
		}
	}
	catch (...) {
		wchar_t buffer[8192];
		swprintf(buffer, 8192, L"FAIL | Exception occurred\n");
		the_listener->edit_stream->wputs(buffer);
		the_listener->edit_stream->flush();
		return &false_value;
	}

	return &true_value;
}
