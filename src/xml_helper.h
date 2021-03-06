/*
 *  xml_helper.h
 *  eri
 *
 *  Created by exe on 2010/10/26.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_XML_HELPER_H
#define ERI_XML_HELPER_H

#include <string>
#include "rapidxml.hpp"

namespace ERI
{

	struct Vector2;
	struct Color;
	
	struct XmlParseData
	{
		XmlParseData() : current_node(NULL), buffer(NULL) {}
		~XmlParseData() { if (buffer) delete [] buffer; }
		
		rapidxml::xml_document<>	doc;
		rapidxml::xml_node<>*		current_node;
		char*						buffer;
	};
	
	bool ParseFile(const std::string& path, XmlParseData& out_data);
	
	rapidxml::xml_attribute<>* GetAttrBool(rapidxml::xml_node<>* node, const char* name, bool& out_value);
	rapidxml::xml_attribute<>* GetAttrInt(rapidxml::xml_node<>* node, const char* name, int& out_value);
	rapidxml::xml_attribute<>* GetAttrFloat(rapidxml::xml_node<>* node, const char* name, float& out_value);
	rapidxml::xml_attribute<>* GetAttrDouble(rapidxml::xml_node<>* node, const char* name, double& out_value);
	rapidxml::xml_attribute<>* GetAttrStr(rapidxml::xml_node<>* node, const char* name, std::string& out_value);
	rapidxml::xml_attribute<>* GetAttrVector2(rapidxml::xml_node<>* node, const char* name, Vector2& out_value);
	rapidxml::xml_attribute<>* GetAttrColor(rapidxml::xml_node<>* node, const char* name, Color& out_value, bool base_255 = false);
	rapidxml::xml_attribute<>* GetAttrRect(rapidxml::xml_node<>* node, const char* name, Vector2& out_left_top, Vector2& out_right_bottom);

	struct XmlCreateData
	{
		XmlCreateData() : current_node(NULL) {}
		
		rapidxml::xml_document<>	doc;
		rapidxml::xml_node<>*		current_node;
	};
	
	bool SaveFile(const std::string& path, const XmlCreateData& data);
	
	rapidxml::xml_node<>* CreateNode(rapidxml::xml_document<>& doc, const char* name);
	void PutAttrBool(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const char* name, bool value);
	void PutAttrInt(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const char* name, int value);
	void PutAttrFloat(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const char* name, float value);
	void PutAttrDouble(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const char* name, double value);
	void PutAttrStr(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const char* name, const std::string& value);
	void PutAttrVector2(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const char* name, const Vector2& value);
	void PutAttrColor(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const char* name, const Color& value, bool base_255 = false);
	void PutAttrRect(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const char* name, const Vector2& left_top, const Vector2& right_bottom);
}

#endif // ERI_XML_HELPER_H
