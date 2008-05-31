#pragma once


/**
 * This class factory object creates the main handlers -
 * its constructor says which OLE class it has to make.
 */
class CShellExtClassFactory : public IClassFactory
{
protected:
    ULONG m_cRef;
	
public:
    CShellExtClassFactory();
    virtual ~CShellExtClassFactory();
	
	//@{
    /// IUnknown members
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
	//@}
    
	//@{
    /// IClassFactory members
    STDMETHODIMP      CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP      LockServer(BOOL);
	//@}
};
