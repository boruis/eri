/*
 *  xml_helper.cpp
 *  eri
 *
 *  Created by exe on 2010/10/26.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "xml_helper.h"

#include "math_helper.h"

namespace ERI
{

	rapidxml::xml_attribute<>* GetAttrBool(rapidxml::xml_node<>* node, const char* name, bool& out_value)
	{
		rapidxml::xml_attribute<>* attr = node->first_attribute(name);
		if (attr)
		{
			out_value = (strcmp(attr->value(), "true") == 0);
			return attr;
		}
		
		return NULL;
	}

	rapidxml::xml_attribute<>* GetAttrInt(rapidxml::xml_node<>* node, const char* name, int& out_value)
	{
		rapidxml::xml_attribute<>* attr = node->first_attribute(name);
		if (attr)
		{
			out_value = atoi(attr->value());
			return attr;
		}
		
		return NULL;
	}

	rapidxml::xml_attribute<>* GetAttrFloat(rapidxml::xml_node<>* node, const char* name, float& out_value)
	{
		rapidxml::xml_attribute<>* attr = node->first_attribute(name);
		if (attr)
		{
			out_value = static_cast<float>(atof(attr->value()));
			return attr;
		}
		
		return NULL;
	}

	rapidxml::xml_attribute<>* GetAttrStr(rapidxml::xml_node<>* node, const char* name, std::string& out_value)
	{
		rapidxml::xml_attribute<>* attr = node->first_attribute(name);
		if (attr)
		{
			out_value = attr->value();
			return attr;
		}
		
		return NULL;
	}

	rapidxml::xml_attribute<>* GetAttrColor(rapidxml::xml_node<>* node, const char* name, Color& out_value)
	{
		rapidxml::xml_attribute<>* attr = node->first_attribute(name);
		if (attr)
		{
			std::string s = attr->value();
			size_t pos = s.find(',');
			if (pos == std::string::npos)
			{
				out_value.r = static_cast<float>(atof(s.c_str()));
				return attr;
			}
			else
			{
				out_value.r = static_cast<float>(atof(s.substr(0, pos).c_str()));
			}

			s = s.substr(pos + 1);
			pos = s.find(',');
			if (pos == std::string::npos)
			{
				out_value.g = static_cast<float>(atof(s.c_str()));
				return attr;
			}
			else
			{
				out_value.g = static_cast<float>(atof(s.substr(0, pos).c_str()));
			}
			
			s = s.substr(pos + 1);
			pos = s.find(',');
			if (pos == std::string::npos)
			{
				out_value.b = static_cast<float>(atof(s.c_str()));
				return attr;
			}
			else
			{
				out_value.b = static_cast<float>(atof(s.substr(0, pos).c_str()));
			}		

			s = s.substr(pos + 1);
			pos = s.find(',');
			if (pos == std::string::npos)
			{
				out_value.a = static_cast<float>(atof(s.c_str()));
			}
			else
			{
				out_value.a = static_cast<float>(atof(s.substr(0, pos).c_str()));
			}
			
			return attr;
		}
		
		return NULL;	
	}

}
