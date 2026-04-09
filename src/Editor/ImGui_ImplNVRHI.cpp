#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "ImGui_ImplNVRHI.h"
#include "../Core/Base.h"
#include <spdlog/spdlog.h> // Flush işlemleri için eklendi
#include <string>
#include <vector>

namespace DoEngine {

// Log kaybını önlemek için anında yazma (flush) makrosu
#define LOG_AND_FLUSH(...)                                                     \
  do {                                                                         \
    DO_CORE_INFO(__VA_ARGS__);                                                 \
    spdlog::default_logger()->flush();                                         \
  } while (0)

struct ImGui_ImplNVRHI_Data {
  nvrhi::IDevice *Device = nullptr;
  nvrhi::TextureHandle FontTexture;
  nvrhi::SamplerHandle FontSampler;
  nvrhi::ShaderHandle VertexShader;
  nvrhi::ShaderHandle PixelShader;
  nvrhi::InputLayoutHandle InputLayout;
  nvrhi::BindingLayoutHandle BindingLayout;
  nvrhi::BindingSetHandle BindingSet;
  nvrhi::GraphicsPipelineHandle Pipeline;

  nvrhi::BufferHandle VertexBuffer;
  nvrhi::BufferHandle IndexBuffer;
  uint32_t VertexBufferSize = 0;
  uint32_t IndexBufferSize = 0;
};

static ImGui_ImplNVRHI_Data *g_BackendData = nullptr;

// Embedded SPIR-V Shaders (Minimal ImGui)
static const uint32_t g_ImGuiVS[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x0000002b, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3534352e,
    0x00000030, 0x0003000e, 0x00000000, 0x00000001, 0x000a000f, 0x00000000,
    0x00000004, 0x6d61696e, 0x00000000, 0x00000009, 0x0000000b, 0x0000001b,
    0x00000021, 0x00000023, 0x00000025, 0x00030003, 0x00000002, 0x000001c2,
    0x00040005, 0x00000004, 0x6d61696e, 0x00000000, 0x00030005, 0x00000009,
    0x00000000, 0x00050005, 0x0000000d, 0x6d70,     0x44617461, 0x00000000,
    0x00050006, 0x0000000d, 0x00000000, 0x70726f6a, 0x00000000, 0x00030005,
    0x0000000f, 0x00000000, 0x00050005, 0x0000001b, 0x6f757443, 0x6f6c6f72,
    0x00000000, 0x00040005, 0x0000001d, 0x696e436f, 0x6c6f72,   0x00030005,
    0x00000021, 0x6f757455, 0x56,       0x00030005, 0x00000023, 0x696e5556,
    0x00000000, 0x00030005, 0x00000025, 0x696e506f, 0x73,       0x00040047,
    0x00000009, 0x0000000b, 0x00000000, 0x00050048, 0x0000000d, 0x00000000,
    0x00000023, 0x00000000, 0x00030047, 0x0000000d, 0x00000002, 0x00040047,
    0x0000001b, 0x0000001e, 0x00000000, 0x00040047, 0x0000001d, 0x0000001e,
    0x00000001, 0x00040047, 0x00000021, 0x0000001e, 0x00000002, 0x00040047,
    0x00000023, 0x0000001e, 0x00000002, 0x00040047, 0x00000025, 0x0000001e,
    0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
    0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006,
    0x00000004, 0x00040018, 0x00000008, 0x00000007, 0x00000004, 0x0004001e,
    0x00000009, 0x00000008, 0x00000008, 0x00040020, 0x0000000a, 0x00000003,
    0x00000009, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000003, 0x0004001e,
    0x0000000d, 0x00000008, 0x00030020, 0x0000000e, 0x00000002, 0x0004003b,
    0x0000000e, 0x0000000f, 0x00000001, 0x00040020, 0x00000010, 0x00000001,
    0x00000008, 0x00040020, 0x00000012, 0x00000003, 0x00000007, 0x00040017,
    0x00000015, 0x00000006, 0x00000002, 0x00040020, 0x0000001a, 0x00000003,
    0x00000007, 0x0004003b, 0x0000001a, 0x0000001b, 0x00000003, 0x00040020,
    0x0000001c, 0x00000001, 0x00000007, 0x0004003b, 0x0000001c, 0x0000001d,
    0x00000001, 0x00040020, 0x00000020, 0x00000003, 0x00000015, 0x0004003b,
    0x00000020, 0x00000021, 0x00000003, 0x00040020, 0x00000022, 0x00000001,
    0x00000015, 0x0004003b, 0x00000022, 0x00000023, 0x00000001, 0x0004003b,
    0x00000020, 0x00000025, 0x00000001, 0x0004002b, 0x00000006, 0x00000027,
    0x00000000, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
    0x000200f8, 0x00000005, 0x0004003d, 0x00000008, 0x00000011, 0x0000000f,
    0x00050041, 0x00000012, 0x00000013, 0x0000000b, 0x00000027, 0x0003003e,
    0x00000013, 0x00000027, 0x0004003d, 0x00000015, 0x00000024, 0x00000025,
    0x00050051, 0x00000006, 0x00000026, 0x00000024, 0x00000000, 0x00050051,
    0x00000006, 0x00000028, 0x00000024, 0x00000001, 0x00070050, 0x00000007,
    0x00000029, 0x00000026, 0x00000028, 0x00000027, 0x3f800000, 0x00050091,
    0x00000007, 0x0000002a, 0x00000011, 0x00000029, 0x00050041, 0x00000012,
    0x0000002b, 0x0000000b, 0x00000027, 0x0003003e, 0x0000002b, 0x0000002a,
    0x0004003d, 0x00000007, 0x0000001e, 0x0000001d, 0x0003003e, 0x0000001b,
    0x0000001e, 0x0004003d, 0x00000015, 0x00000022, 0x00000023, 0x0003003e,
    0x00000021, 0x00000022, 0x000100fd, 0x00010038};

static const uint32_t g_ImGuiPS[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x0000001e, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3534352e,
    0x00000030, 0x0003000e, 0x00000000, 0x00000001, 0x0009000f, 0x00000004,
    0x00000004, 0x6d61696e, 0x00000000, 0x00000009, 0x0000000d, 0x00000011,
    0x00000016, 0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002,
    0x000001c2, 0x00040005, 0x00000004, 0x6d61696e, 0x00000000, 0x00040005,
    0x00000009, 0x6f757443, 0x6f6c6f72, 0x00040005, 0x0000000d, 0x73546578,
    0x74757265, 0x00030005, 0x00000011, 0x696e5556, 0x00000000, 0x00050005,
    0x00000016, 0x696e436f, 0x6c6f72,   0x00000000, 0x00040047, 0x00000009,
    0x0000001e, 0x00000000, 0x00040047, 0x0000000d, 0x00000022, 0x00000000,
    0x00040047, 0x0000000d, 0x00000021, 0x00000001, 0x00040047, 0x00000011,
    0x0000001e, 0x00000002, 0x00040047, 0x00000016, 0x0000001e, 0x00000000,
    0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016,
    0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004,
    0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008,
    0x00000009, 0x00000003, 0x00090019, 0x0000000a, 0x00000006, 0x00000001,
    0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b,
    0x0000000b, 0x0000000a, 0x00040020, 0x0000000c, 0x00000000, 0x0000000b,
    0x0004003b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040017, 0x0000000f,
    0x00000006, 0x00000002, 0x00040020, 0x00000010, 0x00000001, 0x0000000f,
    0x0004003b, 0x00000010, 0x00000011, 0x00000001, 0x00040020, 0x00000015,
    0x00000001, 0x00000007, 0x0004003b, 0x00000015, 0x00000016, 0x00000001,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x0004003d, 0x0000000b, 0x00000012, 0x0000000d, 0x0004003d,
    0x0000000f, 0x00000013, 0x00000011, 0x00050057, 0x00000007, 0x00000014,
    0x00000012, 0x00000013, 0x0004003d, 0x00000007, 0x00000017, 0x00000016,
    0x00050085, 0x00000007, 0x00000018, 0x00000014, 0x00000017, 0x0003003e,
    0x00000009, 0x00000018, 0x000100fd, 0x00010038};

bool ImGui_ImplNVRHI_CreateDeviceObjects();

bool ImGui_ImplNVRHI_Init(nvrhi::IDevice *device) {
  LOG_AND_FLUSH("ImGui_ImplNVRHI_Init: Called");
  if (g_BackendData)
    return true;

  if (!device) {
    LOG_AND_FLUSH("ImGui_ImplNVRHI_Init: ERROR - device pointer is null!");
    return false;
  }

  g_BackendData = new ImGui_ImplNVRHI_Data();
  g_BackendData->Device = device;

  ImGuiIO &io = ImGui::GetIO();
  io.BackendRendererName = "imgui_impl_nvrhi";
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

  return ImGui_ImplNVRHI_CreateDeviceObjects();
}

void ImGui_ImplNVRHI_InvalidateDeviceObjects() {
  if (!g_BackendData)
    return;
  g_BackendData->FontTexture = nullptr;
  g_BackendData->FontSampler = nullptr;
  g_BackendData->VertexShader = nullptr;
  g_BackendData->PixelShader = nullptr;
  g_BackendData->InputLayout = nullptr;
  g_BackendData->BindingLayout = nullptr;
  g_BackendData->BindingSet = nullptr;
  g_BackendData->Pipeline = nullptr;
  g_BackendData->VertexBuffer = nullptr;
  g_BackendData->IndexBuffer = nullptr;
}

void ImGui_ImplNVRHI_Shutdown() {
  ImGui_ImplNVRHI_InvalidateDeviceObjects();
  delete g_BackendData;
  g_BackendData = nullptr;
}

bool ImGui_ImplNVRHI_CreateDeviceObjects() {
  if (!g_BackendData || !g_BackendData->Device) {
    LOG_AND_FLUSH(
        "ImGui_ImplNVRHI_CreateDeviceObjects: ERROR - Invalid state!");
    return false;
  }
  nvrhi::IDevice *device = g_BackendData->Device;

  // 1. Shaders
  LOG_AND_FLUSH("ImGui_ImplNVRHI: Creating Shaders...");
  g_BackendData->VertexShader =
      device->createShader(nvrhi::ShaderDesc(nvrhi::ShaderType::Vertex),
                           g_ImGuiVS, sizeof(g_ImGuiVS));
  if (!g_BackendData->VertexShader) {
    LOG_AND_FLUSH("ERROR: Failed to create Vertex Shader!");
    return false;
  }

  g_BackendData->PixelShader =
      device->createShader(nvrhi::ShaderDesc(nvrhi::ShaderType::Pixel),
                           g_ImGuiPS, sizeof(g_ImGuiPS));
  if (!g_BackendData->PixelShader) {
    LOG_AND_FLUSH("ERROR: Failed to create Pixel Shader!");
    return false;
  }

  // 2. Input Layout
  LOG_AND_FLUSH("ImGui_ImplNVRHI: Creating Input Layout...");
  nvrhi::VertexAttributeDesc attributes[] = {
      nvrhi::VertexAttributeDesc()
          .setName("POSITION")
          .setFormat(nvrhi::Format::RG32_FLOAT)
          .setOffset(offsetof(ImDrawVert, pos))
          .setElementStride(sizeof(ImDrawVert)),
      nvrhi::VertexAttributeDesc()
          .setName("TEXCOORD")
          .setFormat(nvrhi::Format::RG32_FLOAT)
          .setOffset(offsetof(ImDrawVert, uv))
          .setElementStride(sizeof(ImDrawVert)),
      nvrhi::VertexAttributeDesc()
          .setName("COLOR")
          .setFormat(nvrhi::Format::RGBA8_UNORM)
          .setOffset(offsetof(ImDrawVert, col))
          .setElementStride(sizeof(ImDrawVert)),
  };
  g_BackendData->InputLayout =
      device->createInputLayout(attributes, 3, g_BackendData->VertexShader);

  // 3. Binding Layout
  LOG_AND_FLUSH("ImGui_ImplNVRHI: Creating Binding Layout...");
  nvrhi::BindingLayoutDesc layoutDesc;
  layoutDesc.setVisibility(nvrhi::ShaderType::AllGraphics);

  // KRİTİK HATA BURADAYDI: mat4 matrisi 64 byte'dır (16 float). Eski kod 4
  // float (16 byte) diyordu!
  layoutDesc.addItem(
      nvrhi::BindingLayoutItem::PushConstants(0, sizeof(float) * 16));
  layoutDesc.addItem(nvrhi::BindingLayoutItem::Texture_SRV(0));
  layoutDesc.addItem(nvrhi::BindingLayoutItem::Sampler(0));

  g_BackendData->BindingLayout = device->createBindingLayout(layoutDesc);
  if (!g_BackendData->BindingLayout) {
    LOG_AND_FLUSH("ERROR: Failed to create Binding Layout!");
    return false;
  }

  // 4. Font Texture
  LOG_AND_FLUSH("ImGui_ImplNVRHI: Creating Font Texture...");
  ImGuiIO &io = ImGui::GetIO();
  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  nvrhi::TextureDesc td;
  td.width = width;
  td.height = height;
  td.format = nvrhi::Format::RGBA8_UNORM;
  td.debugName = "ImGuiFont";
  td.initialState = nvrhi::ResourceStates::CopyDest;
  g_BackendData->FontTexture = device->createTexture(td);
  if (!g_BackendData->FontTexture) {
    LOG_AND_FLUSH("ERROR: Failed to create Font Texture!");
    return false;
  }

  LOG_AND_FLUSH("ImGui_ImplNVRHI: Uploading Font Data via Command List...");

  // VULKAN POINTER VALIDATION (Dreadnought Fix)
  if (!VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdPipelineBarrier2) {
    LOG_AND_FLUSH("FATAL ERROR: vkCmdPipelineBarrier2 is NULL! Feature 'synchronization2' might be missing.");
    return false;
  }
  if (!VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdCopyBufferToImage) {
    LOG_AND_FLUSH("FATAL ERROR: vkCmdCopyBufferToImage is NULL! Dispatcher failed to load core device functions.");
    return false;
  }

  if (!pixels) {
    LOG_AND_FLUSH("FATAL ERROR: ImGui pixels pointer is NULL!");
    return false;
  }

  LOG_AND_FLUSH(" -> 1. device->createCommandList()");
  nvrhi::CommandListHandle cmd = device->createCommandList();
  if (!cmd) {
    LOG_AND_FLUSH("FATAL ERROR: Failed to create NVRHI Command List!");
    return false;
  }

  LOG_AND_FLUSH(" -> 2. cmd->open()");
  cmd->open();

  LOG_AND_FLUSH(" -> 3. cmd->writeTexture()");
  cmd->writeTexture(g_BackendData->FontTexture, 0, 0, pixels, width * 4, width * height * 4);

  LOG_AND_FLUSH(" -> 4. cmd->setTextureState()");
  cmd->setTextureState(g_BackendData->FontTexture, nvrhi::AllSubresources,
                       nvrhi::ResourceStates::ShaderResource);

  LOG_AND_FLUSH(" -> 5. cmd->close()");
  cmd->close();

  LOG_AND_FLUSH(" -> 6. device->executeCommandList()");
  device->executeCommandList(cmd);

  LOG_AND_FLUSH(" -> 7. device->waitForIdle()");
  device->waitForIdle();

  LOG_AND_FLUSH("ImGui_ImplNVRHI: Creating Sampler and Binding Set...");
  nvrhi::SamplerDesc sd;
  g_BackendData->FontSampler = device->createSampler(sd);

  nvrhi::BindingSetDesc bsd;
  bsd.addItem(
      nvrhi::BindingSetItem::Texture_SRV(0, g_BackendData->FontTexture));
  bsd.addItem(nvrhi::BindingSetItem::Sampler(0, g_BackendData->FontSampler));
  g_BackendData->BindingSet = g_BackendData->Device->createBindingSet(
      bsd, g_BackendData->BindingLayout);

  LOG_AND_FLUSH(
      "ImGui_ImplNVRHI_Init: Success (Deferred Pipeline Creation enabled).");
  return true;
}

void ImGui_ImplNVRHI_NewFrame() {}

static void CreatePipeline(nvrhi::IFramebuffer *fb) {
  if (g_BackendData->Pipeline &&
      g_BackendData->Pipeline->getFramebufferInfo() == fb->getFramebufferInfo())
    return;

  LOG_AND_FLUSH("ImGui_ImplNVRHI: Creating Graphics Pipeline (Deferred)...");

  nvrhi::GraphicsPipelineDesc pd;
  pd.VS = g_BackendData->VertexShader;
  pd.PS = g_BackendData->PixelShader;
  pd.inputLayout = g_BackendData->InputLayout;
  pd.bindingLayouts = {g_BackendData->BindingLayout};
  pd.renderState.rasterState.setFillSolid().setCullNone().setScissorEnable(
      true);
  pd.renderState.depthStencilState.disableDepthTest().disableDepthWrite();
  pd.renderState.blendState.targets[0]
      .setBlendEnable(true)
      .setSrcBlend(nvrhi::BlendFactor::SrcAlpha)
      .setDestBlend(nvrhi::BlendFactor::InvSrcAlpha)
      .setBlendOp(nvrhi::BlendOp::Add)
      .setSrcBlendAlpha(nvrhi::BlendFactor::One)
      .setDestBlendAlpha(nvrhi::BlendFactor::InvSrcAlpha)
      .setBlendOpAlpha(nvrhi::BlendOp::Add);

  g_BackendData->Pipeline =
      g_BackendData->Device->createGraphicsPipeline(pd, fb);

  if (g_BackendData->Pipeline) {
    LOG_AND_FLUSH("ImGui_ImplNVRHI: Graphics Pipeline Created successfully.");
  } else {
    LOG_AND_FLUSH("ImGui_ImplNVRHI: FAILED to create Graphics Pipeline!");
  }
}

void ImGui_ImplNVRHI_RenderDrawData(ImDrawData *draw_data,
                                    nvrhi::ICommandList *cmd,
                                    nvrhi::IFramebuffer *fb) {
  if (!fb || draw_data->DisplaySize.x <= 0.0f ||
      draw_data->DisplaySize.y <= 0.0f)
    return;

  CreatePipeline(fb);

  if (!g_BackendData->Pipeline)
    return; // Pipeline yoksa çökmesini engellemek için güvenlik

  // Update Buffers
  if (!g_BackendData->VertexBuffer ||
      g_BackendData->VertexBufferSize < (uint32_t)draw_data->TotalVtxCount) {
    g_BackendData->VertexBufferSize = draw_data->TotalVtxCount + 5000;
    g_BackendData->VertexBuffer = g_BackendData->Device->createBuffer(
        nvrhi::BufferDesc()
            .setByteSize(g_BackendData->VertexBufferSize * sizeof(ImDrawVert))
            .setIsVertexBuffer(true)
            .setDebugName("ImGuiVB"));
  }
  if (!g_BackendData->IndexBuffer ||
      g_BackendData->IndexBufferSize < (uint32_t)draw_data->TotalIdxCount) {
    g_BackendData->IndexBufferSize = draw_data->TotalIdxCount + 10000;
    g_BackendData->IndexBuffer = g_BackendData->Device->createBuffer(
        nvrhi::BufferDesc()
            .setByteSize(g_BackendData->IndexBufferSize * sizeof(ImDrawIdx))
            .setIsIndexBuffer(true)
            .setDebugName("ImGuiIB"));
  }

  // Upload Data
  std::vector<ImDrawVert> vtx(draw_data->TotalVtxCount);
  std::vector<ImDrawIdx> idx(draw_data->TotalIdxCount);
  ImDrawVert *vtx_dst = vtx.data();
  ImDrawIdx *idx_dst = idx.data();
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList *cmd_list = draw_data->CmdLists[n];
    memcpy(vtx_dst, cmd_list->VtxBuffer.Data,
           cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(idx_dst, cmd_list->IdxBuffer.Data,
           cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtx_dst += cmd_list->VtxBuffer.Size;
    idx_dst += cmd_list->IdxBuffer.Size;
  }
  cmd->writeBuffer(g_BackendData->VertexBuffer, vtx.data(),
                   vtx.size() * sizeof(ImDrawVert));
  cmd->writeBuffer(g_BackendData->IndexBuffer, idx.data(),
                   idx.size() * sizeof(ImDrawIdx));

  // Draw
  nvrhi::GraphicsState state;
  state.pipeline = g_BackendData->Pipeline;
  state.framebuffer = fb;
  state.bindings = {g_BackendData->BindingSet};
  state.vertexBuffers.push_back(
      nvrhi::VertexBufferBinding().setBuffer(g_BackendData->VertexBuffer));
  state.indexBuffer =
      nvrhi::IndexBufferBinding()
          .setBuffer(g_BackendData->IndexBuffer)
          .setFormat(sizeof(ImDrawIdx) == 2 ? nvrhi::Format::R16_UINT
                                            : nvrhi::Format::R32_UINT);

  float L = draw_data->DisplayPos.x;
  float T = draw_data->DisplayPos.y;
  float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
  float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
  float mvp[4][4] = {
      {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
      {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
      {0.0f, 0.0f, 0.5f, 0.0f},
      {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
  };

  uint32_t vtx_offset = 0;
  uint32_t idx_offset = 0;
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList *cmd_list = draw_data->CmdLists[n];
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];

      nvrhi::Rect scissorRect(
          (int)(pcmd->ClipRect.x - L), (int)(pcmd->ClipRect.y - T),
          (int)(pcmd->ClipRect.z - L), (int)(pcmd->ClipRect.w - T));

      // static_vector hatasını çözmek için ViewportState'i sıfırdan güvenle
      // oluşturuyoruz
      state.viewport =
          nvrhi::ViewportState()
              .addViewport(nvrhi::Viewport(draw_data->DisplaySize.x,
                                           draw_data->DisplaySize.y))
              .addScissorRect(scissorRect);

      cmd->setGraphicsState(state);
      cmd->setPushConstants(mvp, sizeof(mvp));

      cmd->drawIndexed(nvrhi::DrawArguments()
                           .setVertexCount(pcmd->ElemCount)
                           .setStartIndexLocation(idx_offset)
                           .setStartVertexLocation(vtx_offset));
      idx_offset += pcmd->ElemCount;
    }
    vtx_offset += cmd_list->VtxBuffer.Size;
  }
}

} // namespace DoEngine