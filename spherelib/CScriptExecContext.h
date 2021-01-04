#pragma once

class CScriptExecContext : public CExpression
{
public:
	static CScriptPropArray sm_FunctionsAll;

	static void InitFunctions() { throw "not implemented"; }

public:
	virtual HRESULT Function_Dispatch(LPCTSTR pszKey, CGVariant& vArgs, CGVariant& vValRet) { throw "not implemented"; }
};