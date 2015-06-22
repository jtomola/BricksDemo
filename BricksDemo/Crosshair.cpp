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

	// Don't need view matrix, since these will always be drawn in the same position
	Matrix ModelView = Scale * Trans;

	Demo::SetModelView(ModelView);
	Demo::SetColorInfo(color);

	Demo::GetDeviceContext()->DrawIndexed(12 * 3, 0, 0);
};
