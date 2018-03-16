// driveinfo.cpp : Defines the entry point for the console application.
//

//	reference:
//	https://msdn.microsoft.com/en-us/library/aa390423(v=vs.85).aspx
//
#include <atlbase.h>
#include <comdef.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <Wbemidl.h>
#include <limits>


#pragma comment(lib, "wbemuuid.lib")

using namespace std;

namespace
{
	class ComInitializer
	{
	public:
		ComInitializer()
		{

			// Step 1: --------------------------------------------------
			// Initialize COM. ------------------------------------------
			HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
			if (FAILED(hres))
			{
				cout << "Failed to initialize COM library. Error code = 0x"
					<< hex << hres << endl;
				exit(0);                  // Program has failed.
			}

			// Step 2: --------------------------------------------------
			// Set general COM security levels --------------------------
			hres = CoInitializeSecurity(
				NULL,
				-1,                          // COM authentication
				NULL,                        // Authentication services
				NULL,                        // Reserved
				RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
				RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
				NULL,                        // Authentication info
				EOAC_NONE,                   // Additional capabilities 
				NULL                         // Reserved
			);


			if (FAILED(hres))
			{
				cout << "Failed to initialize security. Error code = 0x"
					<< hex << hres << endl;
				CoUninitialize();
				exit(0);                 // Program has failed.
			}
		}

		~ComInitializer()
		{
			CoUninitialize();
		}
	};
}



typedef std::vector<USHORT> USHORTVector;
typedef std::vector<UINT32> UINT32Vector;
typedef std::vector<std::wstring> WStringVector;


class DiskQuery
{
private:
	CComPtr<IWbemServices> _psvc;


	DiskQuery(CComPtr<IWbemServices> psvc) : _psvc(psvc)
	{
		
	}

    DiskQuery(const DiskQuery&) = delete;
    DiskQuery& operator=(const DiskQuery&) = delete;
    DiskQuery(DiskQuery&&) = delete;

public:

    static std::shared_ptr<DiskQuery> GetInstance()
    {
        std::shared_ptr<DiskQuery> diskQryPtr;
        CComPtr<IWbemServices> _psvc = getIWbemServices(L"root\\microsoft\\windows\\storage");
        if (_psvc != nullptr)
            diskQryPtr.reset(new DiskQuery(_psvc));
        return diskQryPtr;
    }

    ~DiskQuery() = default;

	const USHORTVector GetPhysicalDiskType() const
	{
		return getPropStringFromQuery<USHORT>(bstr_t("SELECT * FROM MSFT_PhysicalDisk"), bstr_t("MediaType"));
	}

	const WStringVector GetPhysicalDiskDeviceId() const
	{
		return getPropStringFromQuery<std::wstring>(bstr_t("SELECT * FROM MSFT_PhysicalDisk"), bstr_t("DeviceID"));
	}

	const WStringVector GetPhysicalDiskModel() const
	{
		return getPropStringFromQuery<std::wstring>(bstr_t("SELECT * FROM MSFT_PhysicalDisk"), bstr_t("Model"));
	}

	const UINT32Vector GetSpindleSpeeds() const
	{
		return getPropStringFromQuery<UINT32>(bstr_t("SELECT * FROM MSFT_PhysicalDisk"), bstr_t("SpindleSpeed"));
	}

private:


	/*
	* VARENUM usage key,
	*
	* * [V] - may appear in a VARIANT
	* * [T] - may appear in a TYPEDESC
	* * [P] - may appear in an OLE property set
	* * [S] - may appear in a Safe Array
	*
	*
	*  VT_EMPTY            [V]   [P]     nothing
	*  VT_NULL             [V]   [P]     SQL style Null
	*  VT_I2               [V][T][P][S]  2 byte signed int
	*  VT_I4               [V][T][P][S]  4 byte signed int
	*  VT_R4               [V][T][P][S]  4 byte real
	*  VT_R8               [V][T][P][S]  8 byte real
	*  VT_CY               [V][T][P][S]  currency
	*  VT_DATE             [V][T][P][S]  date
	*  VT_BSTR             [V][T][P][S]  OLE Automation string
	*  VT_DISPATCH         [V][T]   [S]  IDispatch *
	*  VT_ERROR            [V][T][P][S]  SCODE
	*  VT_BOOL             [V][T][P][S]  True=-1, False=0
	*  VT_VARIANT          [V][T][P][S]  VARIANT *
	*  VT_UNKNOWN          [V][T]   [S]  IUnknown *
	*  VT_DECIMAL          [V][T]   [S]  16 byte fixed point
	*  VT_RECORD           [V]   [P][S]  user defined type
	*  VT_I1               [V][T][P][s]  signed char
	*  VT_UI1              [V][T][P][S]  unsigned char
	*  VT_UI2              [V][T][P][S]  unsigned short
	*  VT_UI4              [V][T][P][S]  ULONG
	*  VT_I8                  [T][P]     signed 64-bit int
	*  VT_UI8                 [T][P]     unsigned 64-bit int
	*  VT_INT              [V][T][P][S]  signed machine int
	*  VT_UINT             [V][T]   [S]  unsigned machine int
	*  VT_INT_PTR             [T]        signed machine register size width
	*  VT_UINT_PTR            [T]        unsigned machine register size width
	*  VT_VOID                [T]        C style void
	*  VT_HRESULT             [T]        Standard return type
	*  VT_PTR                 [T]        pointer type
	*  VT_SAFEARRAY           [T]        (use VT_ARRAY in VARIANT)
	*  VT_CARRAY              [T]        C style array
	*  VT_USERDEFINED         [T]        user defined type
	*  VT_LPSTR               [T][P]     null terminated string
	*  VT_LPWSTR              [T][P]     wide null terminated string
	*  VT_FILETIME               [P]     FILETIME
	*  VT_BLOB                   [P]     Length prefixed bytes
	*  VT_STREAM                 [P]     Name of the stream follows
	*  VT_STORAGE                [P]     Name of the storage follows
	*  VT_STREAMED_OBJECT        [P]     Stream contains an object
	*  VT_STORED_OBJECT          [P]     Storage contains an object
	*  VT_VERSIONED_STREAM       [P]     Stream with a GUID version
	*  VT_BLOB_OBJECT            [P]     Blob contains an object
	*  VT_CF                     [P]     Clipboard format
	*  VT_CLSID                  [P]     A Class ID
	*  VT_VECTOR                 [P]     simple counted array
	*  VT_ARRAY            [V]           SAFEARRAY*
	*  VT_BYREF            [V]           void* for local use
	*  VT_BSTR_BLOB                      Reserved for system use
	*/

	template<class Type>
	bool getVariantValue(VARIANT& variant, Type* val) const
	{
		return false;
	}

	template<>
	bool getVariantValue<std::wstring>(VARIANT& variant, std::wstring* val) const
	{
		if (variant.bstrVal != nullptr)
		{
			*val = variant.bstrVal;
			return true;
		}
		return false;
	}

	
	template<>
	bool getVariantValue<USHORT>(VARIANT& variant, USHORT* val) const
	{
		*val = variant.uiVal;
		return true;
	}

	template<>
	bool getVariantValue<UINT32>(VARIANT& variant, UINT32* val) const
	{
		if (variant.bstrVal != nullptr)
		{
			*val = variant.uintVal;
			return true;
		}
		return false;
	}

	template<class Type>
	const std::vector<Type> getPropStringFromQuery(BSTR queryString, BSTR propName) const
	{
		std::vector<Type> queryStrs;
		CComPtr<IEnumWbemClassObject> pEnumerator;
		HRESULT hres = _psvc->ExecQuery(
			bstr_t("WQL"),
			queryString,
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&pEnumerator);

		if (FAILED(hres))
			return queryStrs;

		ULONG uReturn = 0;
		while (pEnumerator)
		{
			CComPtr<IWbemClassObject> pclsObj;
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

			if (0 == uReturn)
				break;

			VARIANT vtProp;
			// Get the value of the Name property
			hr = pclsObj->Get(propName, 0, &vtProp, 0, 0);

			Type value;
			if(getVariantValue(vtProp, &value))
				queryStrs.push_back(value);

			VariantClear(&vtProp);
		}

		return queryStrs;
	}

	static CComPtr<IWbemServices> getIWbemServices(const wchar_t* namespaceStr)
	{
		// Step 3: ---------------------------------------------------
		// Obtain the initial locator to WMI -------------------------

		CComPtr<IWbemLocator> pLoc;

		HRESULT hres = CoCreateInstance(
			CLSID_WbemLocator,
			0,
			CLSCTX_INPROC_SERVER,
			IID_IWbemLocator, (LPVOID *)&pLoc);

		if (FAILED(hres))
		{
			cout << "Failed to create IWbemLocator object."
				<< " Err code = 0x"
				<< hex << hres << endl;
			return nullptr;                 // Program has failed.
		}


		// Step 4: -----------------------------------------------------
		// Connect to WMI through the IWbemLocator::ConnectServer method

		CComPtr<IWbemServices> pSvc;

		// Connect to the root\cimv2 namespace with
		// the current user and obtain pointer pSvc
		// to make IWbemServices calls.
		hres = pLoc->ConnectServer(
			_bstr_t(namespaceStr), // Object path of WMI namespace
			NULL,                    // User name. NULL = current user
			NULL,                    // User password. NULL = current
			0,                       // Locale. NULL indicates current
			NULL,                    // Security flags.
			0,                       // Authority (for example, Kerberos)
			0,                       // Context object 
			&pSvc                    // pointer to IWbemServices proxy
		);

		if (FAILED(hres))
		{
			cout << "Could not connect. Error code = 0x"
				<< hex << hres << endl;
			return nullptr;                // Program has failed.
		}

		// Step 5: --------------------------------------------------
		// Set security levels on the proxy -------------------------

		hres = CoSetProxyBlanket(
			pSvc,                        // Indicates the proxy to set
			RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
			RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
			NULL,                        // Server principal name 
			RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
			RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
			NULL,                        // client identity
			EOAC_NONE                    // proxy capabilities 
		);

		if (FAILED(hres))
		{
			cout << "Could not set proxy blanket. Error code = 0x"
				<< hex << hres << endl;
			return nullptr;               // Program has failed.
		}

		return pSvc;
	}


	
};


//--------------------------------------------------------------------------------------
static const wchar_t* getDiskTypeString(int mediaType)
{
	if (mediaType == 3)
		return L"HDD";
	else if (mediaType == 4)
		return L"SSD";
	else if (mediaType == 0)
		return L"Unspecified";
	else
		return L"Unknown";
}

//--------------------------------------------------------------------------------------
void printHeader()
{
	using namespace std;
	wcout << left 
		<< setw(8) << setfill(L' ') << L"Disk#"
		<< setw(32) << setfill(L' ') << "Disk Model"
		<< setw(10) << setfill(L' ') << "Disk Type"
		<< endl;

	wcout << L"----------------------------------------------------------" << endl;
}

//--------------------------------------------------------------------------------------
void printRow(int idx, const std::wstring& diskModel, const std::wstring& diskType)
{
	using namespace std;
	wcout << left
		<< setw(04) << setfill(L' ') << L"Disk" 
		<< setw(04) << setfill(L' ') << idx
		<< setw(32) << setfill(L' ') << diskModel
		<< setw(06) << setfill(L' ') << diskType
		<< std::endl;
	
}

//--------------------------------------------------------------------------------------
static void printInfo()
{
	using namespace std;
	wcout << endl;
	wcout << L"A handy utility to print drive information" << endl;
	wcout << L"Copyright c Sarang Baheti 2018" << endl;
	wcout << L"source available at : https://www.github.com/angeleno/driveinfo" << endl;
	wcout << endl;
}

//--------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	printInfo();

	ComInitializer initializer;
	
    std::shared_ptr<DiskQuery> diskQuery = DiskQuery::GetInstance();
    if (diskQuery == nullptr)
    {
        std::wcout << L"failed to connect to wmi" << std::endl;
        return 0;
    }

	auto diskModelVec = diskQuery->GetPhysicalDiskModel();
	auto diskTypeVec = diskQuery->GetPhysicalDiskType();

	printHeader();
	for (int idx = 0; idx < diskModelVec.size(); ++idx)
	{
		USHORT diskType = diskTypeVec[idx];
		printRow(idx, diskModelVec[idx], getDiskTypeString(diskType));
	}

	return 0;
}
