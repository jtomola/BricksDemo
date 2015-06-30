#include "Crosshair.h"
#include "Demo.h"

// Default constructor
Crosshair::Crosshair()
	:	position(0.0f, 0.0f, 0.0f),
		scale(0.0f, 0.0f, 0.0f),
		color(1.0f, 1.0f, 1.0f, 1.0f)
{
};

// Draw the crosshair
void Crosshair::Draw()
{
	Matrix Trans;
	Trans.setTrans(position[0], position[1], position[2]);

	Matrix Scale;
	Scale.setScale(scale[0], scale[1], scale[2]);

	// Don't need view matrix 
	// We're skipping view and projection matrices for these,
	// and just providing translation and scale in clip space coordinates
	Matrix ModelView = Scale * Trans;

	Demo::SetModelView(ModelView);
	Demo::SetColorInfo(color);

	// Actual draw call
	Demo::GetDeviceContext()->DrawIndexed(12 * 3, 0, 0);
};
