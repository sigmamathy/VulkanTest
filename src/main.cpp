#include "Graphics/API.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/Pipeline.hpp"
#include "Graphics/Sync.hpp"
#include "Graphics/Buffer.hpp"

int main(int argc, char** argv)
{
    GraphicsAPI api;
    DisplayWindow window(api, 1600, 900, "Hello World");
    GraphicsDevice device(api, window);

    GraphicsPipeline::CreateInfo info;
    info.Device = &device;
    info.Vertex = "shaders/shader.vert.spv";
    info.Fragment = "shaders/shader.frag.spv";
    info.Input.Add(0, 3);
    info.Input.Add(1, 3);

    GraphicsPipeline pipeline(info);

    float vertices[] = {
        -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
    };

    unsigned indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    device.CreateDrawCmdBuffers(2);

    VertexBuffer vb(device, sizeof(vertices));
    vb.MapData(vertices);

    IndexBuffer ib(device, sizeof(indices));
    ib.MapData(indices);

    uint32_t frame = 0;
    DrawPresentSynchronizer syncs[2] = {DrawPresentSynchronizer(device), DrawPresentSynchronizer(device)};

    while (window.IsRunning())
    {
        uint32_t index = syncs[frame].NextFrame();

        device.ResetRecord(frame);

        DrawCmdRecorder rec = device.BeginRecord(frame, index);
        rec.BindPipeline(pipeline);
        rec.SetViewportDefault();
        rec.SetScissorDefault();
        rec.BindVertexBuffer(vb);
        rec.BindIndexBuffer(ib);
        rec.DrawIndexed(6, 1);
        rec.EndRecord();

        syncs[frame].SubmitDraw(device.GetCommandBuffer(frame));
        syncs[frame].PresentOnScreen(index);
        window.PollEvents();

        frame = (frame + 1) % 2;
    }

    device.WaitIdle();

    return 0;
}
