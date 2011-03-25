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

	struct Color;
	
	struct XmlParseData
	{
		XmlParseData() : buffer(NULL) {}
		~XmlParseData() { if (buffer) delete [] buffer; }
		
		rapidxml::xml_document<>	doc;
		char*						buffer;
	};
	
	bool ParseFile(const std::string& path, XmlParseData& out_data);
	
	rapidxml::xml_attribute<>* GetAttrBool(rapidxml::xml_node<>* node, const char* name, bool& out_value);
	rapidxml::xml_attribute<>* GetAttrInt(rapidxml::xml_node<>* node, const char* name, int& out_value);
	rapidxml::xml_attribute<>* GetAttrFloat(rapidxml::xml_node<>* node, const char* name, float& out_value);
	rapidxml::xml_attribute<>* GetAttrStr(rapidxml::xml_node<>* node, const char* name, std::string& out_value);
	rapidxml::xml_attribute<>* GetAttrColor(rapidxml::xml_node<>* node, const char* name, Color& out_value);

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
	void PutAttrStr(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const char* name, const std::string& value);
	void PutAttrColor(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* node, const char* name, const Color& value);
}

#endif // ERI_XML_HELPER_H
