// NOT: Bu dosyada Vulkan başlıklarını include etmiyoruz.
// VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE sadece RHI.cpp'de tanımlanmalı.
// Bu backend doğrudan Vulkan çağrısı yapmaz; sadece NVRHI device arayüzünü kullanır.
#include "ImGui_ImplNVRHI.h"
#include "../Graphics/RHI.h"
#include "../Core/Base.h"
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

#include <fstream>
#include <vector>

static std::vector<uint32_t> LoadSPV(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};
    size_t size = file.tellg();
    file.seekg(0);
    std::vector<uint32_t> buffer((size + 3) / 4, 0);
    file.read((char*)buffer.data(), size);
    return buffer;
}

namespace DoEngine {

// Log kaybını önlemek için anında yazma (flush) makrosu
#define LOG_AND_FLUSH(...)                                                     \
  do {                                                                         \
    DO_CORE_INFO(__VA_ARGS__);                                                 \
    spdlog::default_logger()->flush();                                         \
  } while (0)

class RHIDevice;

struct ImGui_ImplNVRHI_Data {
  RHIDevice* RHI = nullptr;
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

  // Resources managed by this backend
  std::vector<nvrhi::BindingSetHandle> ManagedBindingSets;
  std::vector<nvrhi::TextureHandle> ManagedTextures;

  // Font texture lazily uploaded on first RenderDrawData call
  bool FontTextureUploaded = false;
};

static ImGui_ImplNVRHI_Data *g_BackendData = nullptr;

bool ImGui_ImplNVRHI_CreateDeviceObjects();

bool ImGui_ImplNVRHI_Init(RHIDevice *rhi) {
  LOG_AND_FLUSH("ImGui_ImplNVRHI_Init: Called");
  if (g_BackendData)
    return true;

  if (!rhi || !rhi->GetDevice()) {
    LOG_AND_FLUSH("ImGui_ImplNVRHI_Init: ERROR - RHI or device is null!");
    return false;
  }

  g_BackendData = new ImGui_ImplNVRHI_Data();
  g_BackendData->RHI = rhi;
  g_BackendData->Device = rhi->GetDevice();

  ImGuiIO &io = ImGui::GetIO();
  io.BackendRendererName = "imgui_impl_nvrhi";
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
  // ImGui 1.92+: Bu flag set edilince ImGui font atlas'ı backend'e bırakır.
  // UploadFontTexture() ilk RenderDrawData çağrısında lazy olarak çalışır.
  io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

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
  g_BackendData->ManagedBindingSets.clear();
  g_BackendData->ManagedTextures.clear();
}

void ImGui_ImplNVRHI_Shutdown() {
  ImGui_ImplNVRHI_InvalidateDeviceObjects();
  delete g_BackendData;
  g_BackendData = nullptr;
}

  g_BackendData->Device = device;

  // 1. Shaders
  LOG_AND_FLUSH("ImGui_ImplNVRHI: Loading Shaders...");
  auto vs_data = LoadSPV("shaders/imgui.vs.spv");
  auto fs_data = LoadSPV("shaders/imgui.fs.spv");

  if (vs_data.empty() || fs_data.empty()) {
    LOG_AND_FLUSH("ERROR: Failed to load SPIR-V shaders!");
    return false;
  }

  g_BackendData->VertexShader = device->createShader(
      nvrhi::ShaderDesc(nvrhi::ShaderType::Vertex), vs_data.data(),
      vs_data.size() * 4);
  g_BackendData->PixelShader = device->createShader(
      nvrhi::ShaderDesc(nvrhi::ShaderType::Pixel), fs_data.data(),
      fs_data.size() * 4);

  // 2. Input Layout
  nvrhi::VertexAttributeDesc attributes[] = {
      {"POSITION", nvrhi::Format::RG32_FLOAT, 0, offsetof(ImDrawVert, pos),
       sizeof(ImDrawVert), false},
      {"TEXCOORD", nvrhi::Format::RG32_FLOAT, 0, offsetof(ImDrawVert, uv),
       sizeof(ImDrawVert), false},
      {"COLOR", nvrhi::Format::RGBA8_UNORM, 0, offsetof(ImDrawVert, col),
       sizeof(ImDrawVert), false},
  };
  g_BackendData->InputLayout = device->createInputLayout(
      attributes, sizeof(attributes) / sizeof(attributes[0]),
      g_BackendData->VertexShader);

  // 3. Binding Layout
  nvrhi::BindingLayoutDesc layoutDesc;
  layoutDesc.visibility = nvrhi::ShaderType::All;
  layoutDesc.addItem(nvrhi::BindingSetItem::Texture_SRV(0));
  layoutDesc.addItem(nvrhi::BindingSetItem::Sampler(0));
  layoutDesc.bindingOffsets.shaderResource = 0;
  layoutDesc.bindingOffsets.sampler = 1;

  g_BackendData->BindingLayout = device->createBindingLayout(layoutDesc);
  
  // 4. Sampler
  nvrhi::SamplerDesc sd;
  g_BackendData->FontSampler = device->createSampler(sd);

  LOG_AND_FLUSH("ImGui_ImplNVRHI_Init: Success.");
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
  if (!g_BackendData || !draw_data || !cmd || !fb)
    return;

  if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
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

      // Texture binding (ImGui 1.92+)
      ImTextureID texID = pcmd->GetTexID();
      if (texID != ImTextureID_Invalid) {
        state.bindings = { (nvrhi::IBindingSet*)texID };
      } else {
        state.bindings = { g_BackendData->BindingSet };
      }

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