#include "BarrierSprite.h"

bool BarrierSprite::init() {
	bool bRet = false;

	do {
		CC_BREAK_IF(!Sprite::init());

		bRet = true;
	} while(0);

	return bRet;
}
