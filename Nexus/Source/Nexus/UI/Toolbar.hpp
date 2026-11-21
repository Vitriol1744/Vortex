/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/TextureAsset.hpp>

import Prism.Memory.Ref;
import Prism.String.StringView;

namespace Vortex::Editor
{
    class EditorContext;

    /// The play-mode strip under the menu bar: play/simulate while editing,
    /// stop/pause/step while running. Its buttons run the Scene.* commands.
    class Toolbar
    {
      public:
        Toolbar();
        void Draw(EditorContext& context);

      private:
        ::Ref<TextureAsset> m_Play;
        ::Ref<TextureAsset> m_Simulate;
        ::Ref<TextureAsset> m_Stop;
        ::Ref<TextureAsset> m_Pause;
        ::Ref<TextureAsset> m_Step;

        /// A button running `command`; dimmed and inert when it's disabled.
        void Button(EditorContext& context, const ::Ref<TextureAsset>& icon,
                    StringView command, bool highlighted = false);
    };
} // namespace Vortex::Editor
