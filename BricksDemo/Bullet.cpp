#include "Bullet.h"
#include "Demo.h"

Bullet::Bullet()
	:	pos(0.0f, 0.0f, 0.0f),
		scale(2.0f, 2.0f, 2.0f),
		color(0.0f, 0.0f, 0.0f, 1.0f),
		velocity(0.0f, 0.0f, 0.0f),
		active(false)
{
};

void Bullet::Draw()
{
	if (!this->active) return;
	Matrix Trans;
	Trans.setTrans(pos[0], pos[1], pos[2]);

	Matrix Scale;
	Scale.setScale(scale[0], scale[1], scale[2]);

	Matrix ModelView = Scale * Trans * Demo::GetCamera()->getViewMatrix();

	Demo::SetModelView(ModelView);
	Demo::SetColorInfo(color);

	Demo::GetDeviceContext()->DrawIndexed(12 * 3, 0, 0);
};

void Bullet::Update(const float timeIn)
{
	if (!this->active) return;

	this->pos += (this->velocity * timeIn);
}
