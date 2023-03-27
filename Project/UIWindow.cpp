#include "UIWindow.h"

UIWindow::UIWindow()
{
	name = "Test";
}

UIWindow::UIWindow(HWND _handle)
{
	handle = _handle;
	//DefaultInit();
}

UIWindow::UIWindow(HWND _handle, UIWindowCreation* inputData)
{
	handle = _handle;
	windowToDisplay = inputData;
	Init();
}

UIWindow::~UIWindow()
{
}

void UIWindow::Init()
{
	/*for (string s : windowToDisplay.buttons)
	{
		buttonsToDisplay.push_back(s);
	}*/

	//buttonsToDisplay.push_back("Mesh");
	//buttonsToDisplay.push_back("Albedo");
	//buttonsToDisplay.push_back("Normal");
	//buttonsToDisplay.push_back("Rough");
	//buttonsToDisplay.push_back("Metal");
}

void BrowseButton(std::string browseName)
{
	std::string temp = "Browse " + browseName;
	if (ImGui::SmallButton(temp.c_str()))
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
			COINIT_DISABLE_OLE1DDE);
		IFileDialog* dialog;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&dialog));

		if (SUCCEEDED(hr))
		{
			hr = dialog->Show(NULL);
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = dialog->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					if (SUCCEEDED(hr))
					{
						std::wstring ws(pszFilePath);
						std::string temp(ws.begin(), ws.end());
						std::cout << temp << std::endl;
						ImGui::Text(temp.c_str());

						MessageBox(NULL, temp.c_str(), "File Path", MB_OK);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			dialog->Release();

		}
		CoUninitialize();
	}
}

void UIWindow::StartFrame()
{
	//Start ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

//Prints key:value string:int pairs on the same line.
void UIWindow::StringIntSameLine(map<string, int> pairs)
{
	for (auto kV : pairs)
	{
		ImGui::Text(kV.first.c_str());
		ImGui::SameLine(1.0f);
		ImGui::Text(to_string(kV.second).c_str());
	}
}
