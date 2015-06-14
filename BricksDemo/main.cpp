#include "D3DHeader.h"
#include "Demo.h"

// Main entry point for program
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNUSED(_In_opt_);
	UNUSED(hPrevInstance);
	UNUSED(lpCmdLine);

	// Initialize and run engine
	Demo::Initialize(hInstance, nCmdShow);
	Demo::Run();

	return 0;
};