#include "TestApp.hpp"
#include "Graphics.hpp"
#include "Drawing.hpp"

int main(int argc, char** argv)
{
    TestApp app;

    Framebuffers framebuffers(app);
    GraphicsPipeline pipeline(app, framebuffers, "shaders/shader.vert.spv", "shaders/shader.frag.spv");

    DrawCommandPool pool(app);
    pool.CreateBuffers(2);

    uint32_t frame = 0;
    DrawPresentSynchronizer syncs[2] = {DrawPresentSynchronizer(app), DrawPresentSynchronizer(app)};

    while (app.IsRunning())
    {
        uint32_t index = syncs[frame].NextFrame();

        pool.ResetRecord(frame);

        DrawRecorder rec = pool.BeginRecord(frame, framebuffers, index);
        rec.BindPipeline(pipeline);
        rec.SetViewportDefault();
        rec.SetScissorDefault();
        rec.Draw(3, 1);
        rec.EndRecord();

        syncs[frame].SubmitDraw(pool.GetCommandBuffer(frame));
        syncs[frame].PresentOnScreen(index);
        app.PollEvents();

        frame = (frame + 1) % 2;
    }

    app.WaitIdle();

    return 0;
}
