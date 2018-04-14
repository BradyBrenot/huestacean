//
//  screencapture.m
//  
//
//  Created by Brady Brenot on 2018-03-22.
//
//

#include "mac/screencapture.h"

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#include <QMutex>

QMutex macScreenCaptureLock;
static macScreenCapture::captureCallbackFunc frameCallback;

/////////////////////////////////////////////////////////////////////////////////////

@interface ScreenCapture : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

@property (nonatomic, retain) AVCaptureSession *captureSession;

@end

@implementation ScreenCapture

- (instancetype)initWithDisplayID:(CGDirectDisplayID)displayID
{
    self = [super init];
    if (self) {
        self.captureSession = [[AVCaptureSession alloc] init];
        
        AVCaptureScreenInput *input = [[AVCaptureScreenInput alloc] initWithDisplayID:displayID];
        [self.captureSession addInput:input];
        
        AVCaptureVideoDataOutput *output = [[AVCaptureVideoDataOutput alloc] init];
        
        NSDictionary* videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey, nil];
        
        [output setVideoSettings:videoSettings];
        [output setAlwaysDiscardsLateVideoFrames:true];
        [input setMinFrameDuration:CMTimeMake(1, 30)];
        [input setScaleFactor:0.2];
        input.capturesCursor = false;
        input.capturesMouseClicks = false;
        
        [self.captureSession addOutput:output];
        [output setSampleBufferDelegate:self queue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0)];
        
        [self.captureSession startRunning];
    }
    return self;
}

- (void)dealloc
{
    [self.captureSession stopRunning];
    [super dealloc];
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
    
    macScreenCaptureLock.lock();
    if(frameCallback)
    {
        CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
        CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
        
        size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
        size_t width = CVPixelBufferGetWidth(imageBuffer);
        size_t height = CVPixelBufferGetHeight(imageBuffer);
        void* src_buff = CVPixelBufferGetBaseAddress(imageBuffer);
        
        frameCallback(src_buff, bytesPerRow, width, height);
        
        CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    }
    macScreenCaptureLock.unlock();
}

@end

/////////////////////////////////////////////////////////////////////////////////////

static ScreenCapture* capture = NULL;

//is this thread-safe?
// do I have *any* idea what Objective-C is doing?
//  no, I just put this together from docs and searches and compile errors,
//   use lots of locks and hope for the best.

void macScreenCapture::stop()
{
    macScreenCaptureLock.lock();
    if(capture)
    {
        [capture release];
    }
    capture = NULL;
    
    frameCallback = captureCallbackFunc();
    macScreenCaptureLock.unlock();
}

void macScreenCapture::start(uint32_t displayId, captureCallbackFunc f)
{
    stop();

    macScreenCaptureLock.lock();
    frameCallback = f;
    capture = [[ScreenCapture alloc] initWithDisplayID:displayId];
    macScreenCaptureLock.unlock();
}
