#include "TestApp.hpp"
#include "Graphics.hpp"
#include "Drawing.hpp"
#include "Buffer.hpp"

int main(int argc, char** argv)
{
    TestApp app;
    Framebuffers framebuffers(app);

    GraphicsPipeline::CreateInfo info;
    info.RenderPass = framebuffers.GetRenderPass();
    info.Vertex = "shaders/shader.vert.spv";
    info.Fragment = "shaders/shader.frag.spv";
    info.Input.Add(0, 3);
    info.Input.Add(1, 3);

    GraphicsPipeline pipeline(app, info);

    float vertices[] = {
        -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    DrawCommandPool pool(app);
    pool.CreateBuffers(2);

    VertexBuffer buffer(app, sizeof(vertices));
    buffer.MapData(pool.GetPool(), vertices);

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
        rec.BindVertexBuffer(buffer);
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
