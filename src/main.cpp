#include "Graphics/API.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/Pipeline.hpp"
#include "Graphics/Sync.hpp"
#include "Graphics/Buffer.hpp"
#include "Math/Vector.hpp"

struct Vertex
{
    Fvec3 position;
    Fvec3 color;
};

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

    std::array vertices = {
        Vertex{Fvec3(-0.5f, 0.5f, 0.0f), Fvec3(1.0f, 0.0f, 0.0f)},
        Vertex{Fvec3(0.5f, 0.5f, 0.0f), Fvec3(0.0f, 1.0f, 0.0f)},
        Vertex{Fvec3(0.5f, -0.5f, 0.0f), Fvec3(0.0f, 0.0f, 1.0f)},
        Vertex{Fvec3(-0.5f, -0.5f, 0.0f), Fvec3(1.0f, 1.0f, 1.0f)}
    };

    std::array<unsigned, 6> indices = {
        0, 1, 2,
        0, 2, 3
    };

    VkCommandBuffer cmd[2];
    device.CreateDrawCmdBuffers(cmd, 2);

    VertexBuffer vb(device, sizeof(Vertex) * vertices.size());
    vb.MapData(vertices.data());

    IndexBuffer ib(device, sizeof(unsigned) * indices.size());
    ib.MapData(indices.data());

    uint32_t frame = 0;
    DrawPresentSynchronizer syncs[2] = {DrawPresentSynchronizer(device), DrawPresentSynchronizer(device)};

    while (window.IsRunning())
    {
        uint32_t index = syncs[frame].NextFrame();

        device.ResetRecord(cmd[frame]);

        DrawCmdRecorder rec = device.BeginRecord(cmd[frame], index);
        rec.BindPipeline(pipeline);
        rec.SetViewportDefault();
        rec.SetScissorDefault();
        rec.BindVertexBuffer(vb);
        rec.BindIndexBuffer(ib);
        rec.DrawIndexed(indices.size(), 1);
        rec.EndRecord();

        syncs[frame].SubmitDraw(cmd[frame]);
        syncs[frame].PresentOnScreen(index);
        window.PollEvents();

        frame = (frame + 1) % 2;
    }

    device.WaitIdle();

    return 0;
}
