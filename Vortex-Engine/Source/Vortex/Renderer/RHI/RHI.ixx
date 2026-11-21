/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
export module Vortex.RHI;

/**
 * Vortex's Render Hardware Interface: a backend-agnostic, explicit GPU API.
 *
 *   Vortex.RHI.Types        formats, usages, states, fixed-function state
 *   Vortex.RHI.Resources    Buffer, Texture, Sampler
 *   Vortex.RHI.BindGroup    BindGroupLayout, BindGroup
 *   Vortex.RHI.Shader       Shader, ShaderLayout (reflection)
 *   Vortex.RHI.Pipeline     Pipeline, GraphicsPipelineDesc
 *   Vortex.RHI.PipelineCache pipelines created on demand from descriptions
 *   Vortex.RHI.CommandList  CommandList, barriers, rendering
 *   Vortex.RHI.SwapChain    SwapChain
 *   Vortex.RHI.Device       Device: creation, frames, submission, uploads
 *
 * Engine code normally uses the RenderGraph on top of this; use the RHI
 * directly for backend-agnostic tools and new rendering features.
 */
export import Vortex.RHI.Types;
export import Vortex.RHI.Resources;
export import Vortex.RHI.BindGroup;
export import Vortex.RHI.Shader;
export import Vortex.RHI.Pipeline;
export import Vortex.RHI.PipelineCache;
export import Vortex.RHI.CommandList;
export import Vortex.RHI.SwapChain;
export import Vortex.RHI.Device;
