#pragma once

#include "registry.h"
#include "resource.h"
#include <vector>

extern  UINT                g_cRefThisDll;          // Reference count of this DLL.
extern  HINSTANCE           g_hmodThisDll;          // Instance handle for this DLL
#ifdef _WIN64
// {6A6B7688-3B34-41b2-8487-F1CE4C23FC60}
DEFINE_GUID(CLSID_SKTIMESTAMP,
            0x6a6b7688, 0x3b34, 0x41b2, 0x84, 0x87, 0xf1, 0xce, 0x4c, 0x23, 0xfc, 0x60);
#define SKTIMESTAMP_GUID "{6A6B7688-3B34-41b2-8487-F1CE4C23FC60}"
#else
// {01D8AD1E-46C9-4882-925C-CC29D9A99858}
DEFINE_GUID(CLSID_SKTIMESTAMP,
            0x1d8ad1e, 0x46c9, 0x4882, 0x92, 0x5c, 0xcc, 0x29, 0xd9, 0xa9, 0x98, 0x58);
#define SKTIMESTAMP_GUID "{01D8AD1E-46C9-4882-925C-CC29D9A99858}"
#endif
// The actual OLE Shell context menu handler
/**
 * \ingroup TortoiseShell
 * The main class of our COM object / Shell Extension.
 * It contains all Interfaces we implement for the shell to use.
 * \remark The implementations of the different interfaces are
 * split into several *.cpp files to keep them in a reasonable size.
 */
class CShellExt : public    IShellExtInit,
                            IShellPropSheetExt
{
protected:
    ULONG                   m_cRef;
    std::vector<stdstring>  files_;

private:
public:
    CShellExt();
    virtual ~CShellExt();

    /** \name IUnknown
     * IUnknown members
     */
    //@{
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    //@}


    /** \name IShellExtInit
     * IShellExtInit methods
     */
    //@{
    STDMETHODIMP    Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID);
    //@}

    /** \name IShellPropSheetExt
     * IShellPropSheetExt methods
     */
    //@{
    STDMETHODIMP    AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHODIMP    ReplacePage (UINT, LPFNADDPROPSHEETPAGE, LPARAM);
    //@}

};
