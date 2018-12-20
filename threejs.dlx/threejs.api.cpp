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

Value* threejsImportBufferGeometry_cf(Value **arg_list, int count)
{
	//--------------------------------------------------------
	//Maxscript usage:
	//--------------------------------------------------------
	// Trace <json filename:String>
	check_arg_count(threejsImportJson, 2, count);
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
