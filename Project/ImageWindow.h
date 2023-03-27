#pragma once
#include "UIWindow.h"
#include "DXCore.h"
class ImageWindow : UIWindow
{
public:

	ImageWindow(HWND _handle = 0, UIWindowCreation* windowParam = nullptr);
	~ImageWindow();
	void DisplayWindow(int windowWidth, int windowHeight);
	void SetSRVs(vector <Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> _SRVs);
	void DisplaySettings(int _srvWidth, int _srvHeight, string _name);

private:

	vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> SRVs;
	int srvWidth;
	int srvHeight;
};

