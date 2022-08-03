//
// Created by 张锦 on 2022/8/3.
//

#ifndef FFMPEGPLAY_AUDIOCHANNEL_H
#define FFMPEGPLAY_AUDIOCHANNEL_H

#include "BaseChannel.h"

class AudioChannel : public BaseChannel {

public:
    AudioChannel(int id, JavaCallHelper *pHelper, AVCodecContext *pContext);

    virtual void play();

    virtual void stop();
};


#endif //FFMPEGPLAY_AUDIOCHANNEL_H
