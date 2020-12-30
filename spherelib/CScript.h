#pragma once

class CScriptMethod
{

};

class CScriptProp
{

};

class CScriptPropX
{

};

class CScript : public CFileText
{
public:
	virtual bool ReadTextLine(bool fRemoveBlanks);	// looking for a section or reading strangly formated section. 
	TCHAR* GetLineBuffer();
};