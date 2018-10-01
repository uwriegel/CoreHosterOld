// CoreHoster.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "stdio.h"
#include "mscoree.h"	// Generated from mscoree.idl

void __stdcall Start(wchar_t* moduleName)
{
	wchar_t targetAppPath[MAX_PATH];
	wcscpy(targetAppPath, moduleName);
	auto i = wcslen(targetAppPath) - 1;
	while (i >= 0 && targetAppPath[i] != L'\\') i--;
	targetAppPath[i] = L'\0';

	wchar_t coreDllPath[MAX_PATH];
	wcscpy(coreDllPath, targetAppPath);
	wcscat(coreDllPath, L"\\");
	wcscat(coreDllPath, L"coreclr.dll");

	auto coreCLRModule = LoadLibraryExW(coreDllPath, nullptr, 0);

	ICLRRuntimeHost2* runtimeHost{ nullptr };
	auto GetCLRRuntimeHost = (FnGetCLRRuntimeHost)::GetProcAddress(coreCLRModule, "GetCLRRuntimeHost");
	auto hr = GetCLRRuntimeHost(IID_ICLRRuntimeHost2, (IUnknown**)&runtimeHost);
	hr = runtimeHost->SetStartupFlags(
		// These startup flags control runtime-wide behaviors.
		// A complete list of STARTUP_FLAGS can be found in mscoree.h,
		// but some of the more common ones are listed below.
		static_cast<STARTUP_FLAGS>(
			// STARTUP_FLAGS::STARTUP_SERVER_GC |								// Use server GC
			// STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN |		// Maximize domain-neutral loading
			// STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST |	// Domain-neutral loading for strongly-named assemblies
			STARTUP_FLAGS::STARTUP_CONCURRENT_GC |						// Use concurrent GC
			STARTUP_FLAGS::STARTUP_SINGLE_APPDOMAIN |					// All code executes in the default AppDomain 
																		// (required to use the runtimeHost->ExecuteAssembly helper function)
			STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN	// Prevents domain-neutral loading
			)
	);
	hr = runtimeHost->Start();

	auto appDomainFlags =
		// APPDOMAIN_FORCE_TRIVIAL_WAIT_OPERATIONS |		// Do not pump messages during wait
		// APPDOMAIN_SECURITY_SANDBOXED |					// Causes assemblies not from the TPA list to be loaded as partially trusted
		APPDOMAIN_ENABLE_PLATFORM_SPECIFIC_APPS |			// Enable platform-specific assemblies to run
		APPDOMAIN_ENABLE_PINVOKE_AND_CLASSIC_COMINTEROP |	// Allow PInvoking from non-TPA assemblies
		APPDOMAIN_DISABLE_TRANSPARENCY_ENFORCEMENT;			// Entirely disables transparency checks 
	// </Snippet5>

	// <Snippet6>
	// TRUSTED_PLATFORM_ASSEMBLIES
	// "Trusted Platform Assemblies" are prioritized by the loader and always loaded with full trust.
	// A common pattern is to include any assemblies next to CoreCLR.dll as platform assemblies.
	// More sophisticated hosts may also include their own Framework extensions (such as AppDomain managers)
	// in this list.
	auto tpaSize = 100 * MAX_PATH; // Starting size for our TPA (Trusted Platform Assemblies) list
	auto* trustedPlatformAssemblies = new wchar_t[tpaSize];
	trustedPlatformAssemblies[0] = L'\0';

	// Extensions to probe for when finding TPA list files
	const wchar_t *tpaExtensions[] = {
		L"*.dll",
		L"*.exe",
		L"*.winmd"
	};	
	
	// Probe next to CoreCLR.dll for any files matching the extensions from tpaExtensions and
	// add them to the TPA list. In a real host, this would likely be extracted into a separate function
	// and perhaps also run on other directories of interest.
	for (int i = 0; i < _countof(tpaExtensions); i++)
	{
		// Construct the file name search pattern
		wchar_t searchPath[MAX_PATH];
		wcscpy(searchPath, targetAppPath);
		wcscat(searchPath, L"\\");
		wcscat(searchPath, tpaExtensions[i]);

		// Find files matching the search pattern
		WIN32_FIND_DATAW findData;
		auto fileHandle = FindFirstFileW(searchPath, &findData);

		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			do
			{
				// Construct the full path of the trusted assembly
				wchar_t pathToAdd[MAX_PATH];
				wcscpy_s(pathToAdd, MAX_PATH, targetAppPath);
				wcscat_s(pathToAdd, MAX_PATH, L"\\");
				wcscat_s(pathToAdd, MAX_PATH, findData.cFileName);

				// Check to see if TPA list needs expanded
				if (wcslen(pathToAdd) + (3) + wcslen(trustedPlatformAssemblies) >= tpaSize)
				{
					// Expand, if needed
					tpaSize *= 2;
					wchar_t* newTPAList = new wchar_t[tpaSize];
					wcscpy_s(newTPAList, tpaSize, trustedPlatformAssemblies);
					trustedPlatformAssemblies = newTPAList;
				}

				// Add the assembly to the list and delimited with a semi-colon
				wcscat_s(trustedPlatformAssemblies, tpaSize, pathToAdd);
				wcscat_s(trustedPlatformAssemblies, tpaSize, L";");

				// Note that the CLR does not guarantee which assembly will be loaded if an assembly
				// is in the TPA list multiple times (perhaps from different paths or perhaps with different NI/NI.dll
				// extensions. Therefore, a real host should probably add items to the list in priority order and only
				// add a file if it's not already present on the list.
				//
				// For this simple sample, though, and because we're only loading TPA assemblies from a single path,
				// we can ignore that complication.
			} while (FindNextFileW(fileHandle, &findData));
			FindClose(fileHandle);
		}
	}

	// APP_PATHS
	// App paths are directories to probe in for assemblies which are not one of the well-known Framework assemblies
	// included in the TPA list.
	//
	// For this simple sample, we just include the directory the target application is in.
	// More complex hosts may want to also check the current working directory or other
	// locations known to contain application assets.
	wchar_t appPaths[MAX_PATH * 50];

	// Just use the targetApp provided by the user and remove the file name
	wcscpy(appPaths, targetAppPath);


	// APP_NI_PATHS
	// App (NI) paths are the paths that will be probed for native images not found on the TPA list. 
	// It will typically be similar to the app paths.
	// For this sample, we probe next to the app and in a hypothetical directory of the same name with 'NI' suffixed to the end.
	wchar_t appNiPaths[MAX_PATH * 50];
	wcscpy(appNiPaths, targetAppPath);
	wcscat(appNiPaths, L";");
	wcscat(appNiPaths, targetAppPath);
	wcscat(appNiPaths, L"NI");


	// NATIVE_DLL_SEARCH_DIRECTORIES
	// Native dll search directories are paths that the runtime will probe for native DLLs called via PInvoke
	wchar_t nativeDllSearchDirectories[MAX_PATH * 50];
	wcscpy(nativeDllSearchDirectories, appPaths);
	wcscat(nativeDllSearchDirectories, L";");
	wcscat(nativeDllSearchDirectories, targetAppPath);

	// PLATFORM_RESOURCE_ROOTS
		// Platform resource roots are paths to probe in for resource assemblies (in culture-specific sub-directories)
	wchar_t platformResourceRoots[MAX_PATH * 50];
	wcscpy(platformResourceRoots, appPaths);


	// AppDomainCompatSwitch
	// Specifies compatibility behavior for the app domain. This indicates which compatibility
	// quirks to apply if an assembly doesn't have an explicit Target Framework Moniker. If a TFM is
	// present on an assembly, the runtime will always attempt to use quirks appropriate to the version
	// of the TFM.
	// 
	// Typically the latest behavior is desired, but some hosts may want to default to older Silverlight
	// or Windows Phone behaviors for compatibility reasons.
	const wchar_t* appDomainCompatSwitch = L"UseLatestBehaviorWhenTFMNotSpecified";
	// </Snippet6>

	//
	// STEP 6: Create the AppDomain
	//

	// <Snippet7>
	DWORD domainId;

	// Setup key/value pairs for AppDomain  properties
	const wchar_t* propertyKeys[] = {
		L"TRUSTED_PLATFORM_ASSEMBLIES",
		L"APP_PATHS",
		L"APP_NI_PATHS",
		L"NATIVE_DLL_SEARCH_DIRECTORIES",
		L"PLATFORM_RESOURCE_ROOTS",
		L"AppDomainCompatSwitch"
	};

	// Property values which were constructed in step 5
	const wchar_t* propertyValues[] = {
		trustedPlatformAssemblies,
		appPaths,
		appNiPaths,
		nativeDllSearchDirectories,
		platformResourceRoots,
		appDomainCompatSwitch
	};

	// Create the AppDomain
	hr = runtimeHost->CreateAppDomainWithManager(
		L"Sample Host AppDomain",		// Friendly AD name
		appDomainFlags,
		NULL,							// Optional AppDomain manager assembly name
		NULL,							// Optional AppDomain manager type (including namespace)
		sizeof(propertyKeys) / sizeof(wchar_t*),
		propertyKeys,
		propertyValues,
		&domainId);
	// </Snippet7>

	if (FAILED(hr))
		printf("ERROR - Failed to create AppDomain.\nError code:%x\n", hr);
	else
		printf("AppDomain %d created\n\n", domainId);

	//
	// STEP 7: Run managed code
	//

	// ExecuteAssembly will load a managed assembly and execute its entry point.
	wprintf(L"Executing %s...\n\n", moduleName);

	// <Snippet8>
	DWORD exitCode = -1;
	hr = runtimeHost->ExecuteAssembly(domainId, moduleName, 1, (LPCWSTR*)&moduleName, &exitCode);
	// </Snippet8>

	if (FAILED(hr))
		wprintf(L"ERROR - Failed to execute %s.\nError code:%x\n", moduleName, hr);


	// Clean up
	runtimeHost->UnloadAppDomain(domainId, true);
	runtimeHost->Stop();
	runtimeHost->Release();
}