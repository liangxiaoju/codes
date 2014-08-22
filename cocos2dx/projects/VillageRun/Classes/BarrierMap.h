#ifndef __BARRIERMAP_H__
#define __BARRIERMAP_H__

#include "cocos2d.h"

USING_NS_CC;

class BarrierMap : public TMXTiledMap {
public:
    static BarrierMap* create(const std::string& tmxFile);

protected:
    bool initWithTMXFile(const std::string& tmxFile);
};

#endif
