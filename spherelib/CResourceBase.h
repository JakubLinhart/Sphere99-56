#pragma once

class CResourceDef : public CScriptObj
{
public:
	LPCTSTR GetResourceName() const;
};

#define CResourceLinkPtr CResourceLink*
class CResourceLink : public CResourceDef
{

};