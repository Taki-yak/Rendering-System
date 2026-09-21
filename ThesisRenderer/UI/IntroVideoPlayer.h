#pragma once

#include <string>
#include <vector>


struct IMFSourceReader;


class IntroVideoPlayer
{
public:

    IntroVideoPlayer();

    ~IntroVideoPlayer();


    bool Load(
        const std::wstring& path
    );


    void Update(
        float deltaTime
    );


    bool Draw();


    void Stop();


    bool IsFinished() const;

    bool IsLoaded() const;


private:

    bool ReadNextFrame();

    void UploadPendingFrame();


private:

    IMFSourceReader* reader =
        nullptr;


    unsigned int textureId =
        0;


    int videoWidth =
        0;

    int videoHeight =
        0;


    bool loaded =
        false;

    bool finished =
        false;


    bool mediaFoundationStarted =
        false;

    bool comInitializedByUs =
        false;


    double playbackTime =
        0.0;


    double pendingFrameTime =
        0.0;


    bool hasPendingFrame =
        false;


    std::vector<unsigned char>
        pendingFrame;
};