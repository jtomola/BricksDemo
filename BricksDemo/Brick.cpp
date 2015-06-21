#include "Brick.h"
#include "Demo.h"

Brick::Brick()
	:	pos(0.0f, 0.0f, -20.0f),
		scale(20.0f, 20.0f, 20.0f),
		color(1.0f, 0.0f, 0.0f, 1.0f)
{
};

void Brick::Draw()
{
	Matrix Trans;
	Trans.setTrans(pos[0], pos[1], pos[2]);

	Matrix Scale;
	Scale.setScale(scale[0], scale[1], scale[2]);

	Matrix ModelView = Scale * Trans * Demo::GetCamera()->getViewMatrix();

	Demo::SetModelView(ModelView);
	Demo::SetColorInfo(color);

	Demo::GetDeviceContext()->DrawIndexed(12 * 3, 0, 0);
};