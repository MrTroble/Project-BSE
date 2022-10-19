#include "TGAppIO.hpp"
#include "../TGApp.hpp"

bool isFocused()
{
	return guiModul->focused;
}

void TGAppIO::tryUpdateY(const double delta)
{
	if (this->changeY != 0)
	{
		guiModul->currentY += this->changeY * 0.03 * delta;
		makeVulkan();
		this->changeY = 0;
	}
}
