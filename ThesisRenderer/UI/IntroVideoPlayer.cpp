#define NOMINMAX

#include "IntroVideoPlayer.h"

#include <Windows.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include <iostream>

#include <glad/glad.h>

#include "imgui.h"


#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")


// ============================================================
// CONSTRUCTOR
// ============================================================

IntroVideoPlayer::IntroVideoPlayer()
{
}


// ============================================================
// DESTRUCTOR
// ============================================================

IntroVideoPlayer::~IntroVideoPlayer()
{
    Stop();
}


// ============================================================
// LOAD
// ============================================================

bool IntroVideoPlayer::Load(
    const std::wstring& path
)
{
    Stop();


    // ================= COM =================

    HRESULT result =
        CoInitializeEx(
            nullptr,
            COINIT_MULTITHREADED
        );


    if (SUCCEEDED(result))
    {
        comInitializedByUs =
            true;
    }
    else if (
        result !=
        RPC_E_CHANGED_MODE
        )
    {
        std::cout
            << "Intro Video: COM initialization failed."
            << std::endl;

        return false;
    }


    // ================= MEDIA FOUNDATION =================

    result =
        MFStartup(
            MF_VERSION
        );


    if (FAILED(result))
    {
        std::cout
            << "Intro Video: MFStartup failed."
            << std::endl;

        Stop();

        return false;
    }


    mediaFoundationStarted =
        true;


    // Enable Media Foundation video processing.
    // This lets it convert common MP4 formats to RGB.

    IMFAttributes* attributes =
        nullptr;


    result =
        MFCreateAttributes(
            &attributes,
            1
        );


    if (FAILED(result))
    {
        Stop();

        return false;
    }


    attributes->SetUINT32(
        MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING,
        TRUE
    );


    // ================= SOURCE READER =================

    result =
        MFCreateSourceReaderFromURL(
            path.c_str(),
            attributes,
            &reader
        );


    attributes->Release();


    if (FAILED(result))
    {
        std::cout
            << "Intro Video: unable to open video."
            << std::endl;

        Stop();

        return false;
    }


    reader->SetStreamSelection(
        MF_SOURCE_READER_ALL_STREAMS,
        FALSE
    );


    reader->SetStreamSelection(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        TRUE
    );


    // ================= FORCE RGB32 OUTPUT =================

    IMFMediaType* outputType =
        nullptr;


    result =
        MFCreateMediaType(
            &outputType
        );


    if (FAILED(result))
    {
        Stop();

        return false;
    }


    outputType->SetGUID(
        MF_MT_MAJOR_TYPE,
        MFMediaType_Video
    );


    outputType->SetGUID(
        MF_MT_SUBTYPE,
        MFVideoFormat_RGB32
    );


    result =
        reader->SetCurrentMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            nullptr,
            outputType
        );


    outputType->Release();


    if (FAILED(result))
    {
        std::cout
            << "Intro Video: RGB conversion failed."
            << std::endl;

        Stop();

        return false;
    }


    // ================= GET VIDEO SIZE =================

    IMFMediaType* currentType =
        nullptr;


    result =
        reader->GetCurrentMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            &currentType
        );


    if (FAILED(result))
    {
        Stop();

        return false;
    }


    UINT32 width =
        0;

    UINT32 height =
        0;


    result =
        MFGetAttributeSize(
            currentType,
            MF_MT_FRAME_SIZE,
            &width,
            &height
        );


    currentType->Release();


    if (
        FAILED(result) ||
        width == 0 ||
        height == 0
        )
    {
        std::cout
            << "Intro Video: invalid video dimensions."
            << std::endl;

        Stop();

        return false;
    }


    videoWidth =
        static_cast<int>(
            width
            );


    videoHeight =
        static_cast<int>(
            height
            );


    // ================= OPENGL TEXTURE =================

    glGenTextures(
        1,
        &textureId
    );


    glBindTexture(
        GL_TEXTURE_2D,
        textureId
    );


    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        videoWidth,
        videoHeight,
        0,
        GL_BGRA,
        GL_UNSIGNED_BYTE,
        nullptr
    );


    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );


    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );


    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );


    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );


    glBindTexture(
        GL_TEXTURE_2D,
        0
    );


    playbackTime =
        0.0;


    finished =
        false;


    loaded =
        true;


    hasPendingFrame =
        false;


    std::cout
        << "Intro Video loaded: "
        << videoWidth
        << "x"
        << videoHeight
        << std::endl;


    // Load the first decoded frame.

    ReadNextFrame();

    Update(
        0.0f
    );


    return true;
}


// ============================================================
// READ NEXT FRAME
// ============================================================

bool IntroVideoPlayer::ReadNextFrame()
{
    if (
        reader == nullptr ||
        finished
        )
    {
        return false;
    }


    while (true)
    {
        DWORD streamIndex =
            0;


        DWORD flags =
            0;


        LONGLONG timestamp =
            0;


        IMFSample* sample =
            nullptr;


        HRESULT result =
            reader->ReadSample(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                0,
                &streamIndex,
                &flags,
                &timestamp,
                &sample
            );


        if (FAILED(result))
        {
            finished =
                true;

            return false;
        }


        if (
            flags &
            MF_SOURCE_READERF_ENDOFSTREAM
            )
        {
            finished =
                true;


            if (sample != nullptr)
            {
                sample->Release();
            }


            return false;
        }


        if (sample == nullptr)
        {
            continue;
        }


        IMFMediaBuffer* buffer =
            nullptr;


        result =
            sample->ConvertToContiguousBuffer(
                &buffer
            );


        sample->Release();


        if (FAILED(result))
        {
            continue;
        }


        BYTE* data =
            nullptr;


        DWORD maximumLength =
            0;


        DWORD currentLength =
            0;


        result =
            buffer->Lock(
                &data,
                &maximumLength,
                &currentLength
            );


        if (FAILED(result))
        {
            buffer->Release();

            continue;
        }


        const size_t requiredBytes =
            static_cast<size_t>(
                videoWidth
                ) *
            static_cast<size_t>(
                videoHeight
                ) *
            4;


        if (
            currentLength >=
            requiredBytes
            )
        {
            pendingFrame.resize(
                requiredBytes
            );


            memcpy(
                pendingFrame.data(),
                data,
                requiredBytes
            );


            pendingFrameTime =
                static_cast<double>(
                    timestamp
                    ) /
                10000000.0;


            hasPendingFrame =
                true;
        }


        buffer->Unlock();

        buffer->Release();


        return
            hasPendingFrame;
    }
}


// ============================================================
// UPLOAD FRAME
// ============================================================

void IntroVideoPlayer::UploadPendingFrame()
{
    if (
        !hasPendingFrame ||
        pendingFrame.empty() ||
        textureId == 0
        )
    {
        return;
    }


    glBindTexture(
        GL_TEXTURE_2D,
        textureId
    );


    glPixelStorei(
        GL_UNPACK_ALIGNMENT,
        1
    );


    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        videoWidth,
        videoHeight,
        GL_BGRA,
        GL_UNSIGNED_BYTE,
        pendingFrame.data()
    );


    glBindTexture(
        GL_TEXTURE_2D,
        0
    );
}


// ============================================================
// UPDATE
// ============================================================

void IntroVideoPlayer::Update(
    float deltaTime
)
{
    if (
        !loaded ||
        finished
        )
    {
        return;
    }


    playbackTime +=
        static_cast<double>(
            deltaTime
            );


    if (!hasPendingFrame)
    {
        if (!ReadNextFrame())
            return;
    }


    while (
        hasPendingFrame &&
        pendingFrameTime <=
        playbackTime +
        0.001
        )
    {
        UploadPendingFrame();


        hasPendingFrame =
            false;


        if (!ReadNextFrame())
        {
            break;
        }
    }
}


// ============================================================
// DRAW
// ============================================================

bool IntroVideoPlayer::Draw()
{
    if (
        !loaded ||
        textureId == 0
        )
    {
        return false;
    }


    ImGuiIO& io =
        ImGui::GetIO();


    ImGui::SetNextWindowPos(
        ImVec2(
            0.0f,
            0.0f
        ),
        ImGuiCond_Always
    );


    ImGui::SetNextWindowSize(
        io.DisplaySize,
        ImGuiCond_Always
    );


    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings;


    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowPadding,
        ImVec2(
            0.0f,
            0.0f
        )
    );


    ImGui::Begin(
        "##ORION_INTRO_VIDEO",
        nullptr,
        flags
    );


    ImDrawList* drawList =
        ImGui::GetWindowDrawList();


    drawList->AddRectFilled(
        ImVec2(
            0.0f,
            0.0f
        ),
        io.DisplaySize,
        IM_COL32(
            0,
            0,
            0,
            255
        )
    );


    // ================= KEEP VIDEO ASPECT RATIO =================

    float videoAspect =
        static_cast<float>(
            videoWidth
            ) /
        static_cast<float>(
            videoHeight
            );


    float screenAspect =
        io.DisplaySize.x /
        io.DisplaySize.y;


    ImVec2 videoSize =
        io.DisplaySize;


    if (
        screenAspect >
        videoAspect
        )
    {
        videoSize.x =
            io.DisplaySize.y *
            videoAspect;
    }
    else
    {
        videoSize.y =
            io.DisplaySize.x /
            videoAspect;
    }


    ImVec2 videoPosition =
        ImVec2(
            (
                io.DisplaySize.x -
                videoSize.x
                ) *
            0.5f,

            (
                io.DisplaySize.y -
                videoSize.y
                ) *
            0.5f
        );


    ImGui::SetCursorScreenPos(
        videoPosition
    );


    ImGui::Image(
        static_cast<ImTextureID>(
            textureId
            ),
        videoSize,
        ImVec2(
            0.0f,
            1.0f
        ),
        ImVec2(
            1.0f,
            0.0f
        )
    );


    // ================= SKIP BUTTON =================

    ImVec2 skipButtonSize =
        ImVec2(
            120.0f,
            42.0f
        );


    ImGui::SetCursorScreenPos(
        ImVec2(
            io.DisplaySize.x -
            skipButtonSize.x -
            30.0f,

            io.DisplaySize.y -
            skipButtonSize.y -
            30.0f
        )
    );


    bool skipClicked =
        ImGui::Button(
            "SKIP",
            skipButtonSize
        );


    ImGui::End();


    ImGui::PopStyleVar();


    return
        skipClicked;
}


// ============================================================
// STOP
// ============================================================

void IntroVideoPlayer::Stop()
{
    if (reader != nullptr)
    {
        reader->Release();

        reader =
            nullptr;
    }


    if (textureId != 0)
    {
        glDeleteTextures(
            1,
            &textureId
        );


        textureId =
            0;
    }


    if (mediaFoundationStarted)
    {
        MFShutdown();

        mediaFoundationStarted =
            false;
    }


    if (comInitializedByUs)
    {
        CoUninitialize();

        comInitializedByUs =
            false;
    }


    pendingFrame.clear();


    hasPendingFrame =
        false;


    playbackTime =
        0.0;


    loaded =
        false;


    finished =
        false;
}

bool IntroVideoPlayer::IsFinished() const
{
    return
        finished;
}


bool IntroVideoPlayer::IsLoaded() const
{
    return
        loaded;
}