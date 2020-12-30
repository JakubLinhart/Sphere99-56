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
	virtual bool ReadTextLine(bool fRemoveBlanks) { throw "not implemented"; } // looking for a section or reading strangly formated section. 
	TCHAR* GetLineBuffer() { throw "not implemented"; }

	LPCTSTR GetKey() { throw "not implemented"; }
	LPCTSTR GetArgStr() { throw "not implemented"; }
	CGVariant& GetArgVar() { throw "not implemented"; }
	
	bool FindTextHeader(LPCTSTR pszName) { throw "not implemented"; } // Find a section in the current script
	bool FindNextSection() { throw "not implemented"; }
	bool ReadKeyParse() { throw "not implemented"; }
	bool ReadLine() { throw "not implemented"; }

	void WriteSection(LPCTSTR pszSection, ...) { throw "not implemented"; }
	void WriteKey(LPCTSTR lpszKey, LPCTSTR lpszVal) { throw "not implemented"; }
	void WriteKeyInt(LPCTSTR lpszKey, int iValue) { throw "not implemented"; }
	void WriteKeyDWORD(LPCTSTR lpszKey, DWORD iValue) { throw "not implemented"; }

};