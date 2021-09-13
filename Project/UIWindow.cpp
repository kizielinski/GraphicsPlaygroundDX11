#include "UIWindow.h"

UIWindow::UIWindow()
{
	name = "Test";
	currentList = "Test List";
	index = 0;
	isEnabled = true;
}

UIWindow::~UIWindow()
{
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

void UIWindow::displayWindow(HWND windowHandle)
{
	//Start ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowSize(ImVec2(200, 500), ImGuiCond_Once);

	std::string titleString = "Name: " + name;
	ImGui::Begin(titleString.c_str());

	std::string infoString = std::string(currentList + " Index: ");
	infoString += std::to_string(index);
	ImGui::Text(infoString.c_str());
	std::string clickCount = "Click Count: ";
	if (ImGui::GetIO().WantCaptureMouse)
	{
		BrowseButton("Mesh");
		BrowseButton("Albedo");
		BrowseButton("Normal");
		BrowseButton("Rough");
		BrowseButton("Metal");
	}

	ImGui::Text(clickCount.c_str());
	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
