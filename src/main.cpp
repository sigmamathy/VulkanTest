#include "Graphics/API.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/Pipeline.hpp"
#include "Graphics/Sync.hpp"
#include "Graphics/Buffer.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include "Math/Transform.hpp"

struct Vertex
{
    Fvec3 position;
    Fvec3 color;
};

struct MVP_Matrix
{
	Fmat4 model;
	Fmat4 view;
	Fmat4 proj;
};

static void EventCallback(WindowEvent const& e)
{
    if (e.Type == e.KEY_EVENT) {
        std::cout << e.KE.Code << '\n';
    }
}

int main(int argc, char** argv)
{
    GraphicsAPI api;
    DisplayWindow window(api, 1600, 900, "Hello World");
    window.SetEventCallback(EventCallback);

    GraphicsDevice device(api, window);

    GraphicsPipeline::CreateInfo info;
    info.Device = &device;
    info.Vertex = "out/shaders/shader.vert.spv";
    info.Fragment = "out/shaders/shader.frag.spv";
    info.Input.Add(0, 3);
    info.Input.Add(1, 3);
	info.Descriptors[0].AddUniformBuffer(0, VK_SHADER_STAGE_VERTEX_BIT);
	info.DescriptorSetsMultiplier = 2;

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

	UniformBuffer uniforms[2] = {UniformBuffer(device, sizeof(MVP_Matrix)), UniformBuffer(device, sizeof(MVP_Matrix))};

	pipeline.WriteDescriptor(0, 0, uniforms[0].GetBuffer(), uniforms[0].GetSize());
	pipeline.WriteDescriptor(0, 1, uniforms[1].GetBuffer(), uniforms[1].GetSize());

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

		MVP_Matrix mvp;
		mvp.model = Fmat4(1.f);
		mvp.view = LookAtView(Fvec3(0.f, -1.f, 0.f), Fvec3(0.f, 1.f, 0.f));
		mvp.proj = PerspectiveProjection(3.1415f, 16.f/9, .1f, 100.f);
		uniforms[frame].Update(&mvp);

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
