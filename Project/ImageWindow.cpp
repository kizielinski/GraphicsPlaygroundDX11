#include "ImageWindow.h"

ImageWindow::ImageWindow(HWND _handle, UIWindowCreation* windowParam)
{
	srvHeight = 0;
	srvWidth = 0;
}

ImageWindow::~ImageWindow()
{
	SRVs.clear();
}

void ImageWindow::DisplayWindow(int windowWidth, int windowHeight)
{
	ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Once);
	ImGui::Begin(name.c_str());

	for (auto srv : SRVs)
	{
		ImGui::Image(srv.Get(), ImVec2((float)srvWidth / 4, (float)srvHeight / 4));
	}

	ImGui::End();
}

void ImageWindow::SetSRVs(vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> _SRVs)
{
	SRVs = _SRVs;
}

void ImageWindow::DisplaySettings(int _srvWidth, int _srvHeight, string _name)
{
	srvWidth = _srvWidth;
	srvHeight = _srvHeight;
	name = _name;
}
