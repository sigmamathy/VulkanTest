#include "TestApp.hpp"
#include "Graphics.hpp"
#include "Drawing.hpp"

int main(int argc, char** argv)
{
    TestApp app;

    Framebuffers framebuffers(app);
    GraphicsPipeline pipeline(app, framebuffers, "shaders/shader.vert.spv", "shaders/shader.frag.spv");

    DrawCommandPool pool(app);
    pool.CreateBuffers(3);

    for (int i = 0; i < 3; i++)
    {
        DrawRecorder rec = pool.BeginRecord(i, framebuffers, i);
        rec.BindPipeline(pipeline);
        rec.SetViewportDefault();
        rec.SetScissorDefault();
        rec.Draw(3, 1);
        rec.EndRecord();
    }

    DrawPresentSynchronizer sync(app);

    while (app.IsRunning())
    {
        uint32_t index = sync.NextFrame();

        sync.SubmitDraw(pool.GetCommandBuffer(index));
        sync.PresentOnScreen(index);
        app.PollEvents();
    }

    app.WaitIdle();

    return 0;
}
