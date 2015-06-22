#include "Crosshair.h"
#include "Demo.h"

Crosshair::Crosshair()
	:	pos(0.0f, 0.0f, 0.0f),
		scale(0.0f, 0.0f, 0.0f),
		color(1.0f, 1.0f, 1.0f, 1.0f)
{
};

void Crosshair::Draw()
{
	Matrix Trans;
	Trans.setTrans(pos[0], pos[1], pos[2]);

	Matrix Scale;
	Scale.setScale(scale[0], scale[1], scale[2]);

	//Matrix ModelView(Scale);
	Matrix ModelView = Scale * Trans * Demo::GetCamera()->getViewMatrix();

	Demo::SetModelView(ModelView);
	Demo::SetColorInfo(color);

	Demo::GetDeviceContext()->DrawIndexed(12 * 3, 0, 0);
};

void Crosshair::Update(const float timeIn)
{
	// Grab our camera
	Camera* pCam = Demo::GetCamera();

	//this->pos = pCam->vPos - pCam->vDir * 1.005f;
	this->pos = pCam->vPos - pCam->vDir * 2.005f;
}
