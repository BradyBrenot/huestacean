//
//  screencapture.h
//  
//
//  Created by Brady Brenot on 2018-03-22.
//
//

#ifndef screencapture_h
#define screencapture_h

#include <functional>

namespace macScreenCapture {
    //data, rowstride, width, height
    typedef std::function<void(void*, int, int, int)> captureCallbackFunc;
    void stop();
    void start(uint32_t displayId, captureCallbackFunc frameCallback);
}

#endif /* screencapture_h */
