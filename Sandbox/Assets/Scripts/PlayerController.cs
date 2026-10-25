/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
namespace Gameplay
{
    public class PlayerController : Vortex.Actor
    {
        // Speed of the player movement in units per second
        private float m_MoveSpeed = 5.0f;

        protected override void OnCreate()
        {
            Vortex.Log.Warn("Player created with ID: {0}", ID);
        }

        protected override void OnUpdate(float deltaTime)
        {
            if (Vortex.Input.IsKeyPressed(Vortex.KeyCode.Space))
                Vortex.Log.Info("Space key pressed");

            var transformComponent = GetComponent<Vortex.TransformComponent>();
            if (transformComponent != null)
            {
                var translation = transformComponent.Translation;
                float moveStep = m_MoveSpeed * deltaTime;

                if (Vortex.Input.IsKeyPressed(Vortex.KeyCode.W) || Vortex.Input.IsKeyPressed(Vortex.KeyCode.Up))
                    translation.Y += moveStep;
                if (Vortex.Input.IsKeyPressed(Vortex.KeyCode.S) || Vortex.Input.IsKeyPressed(Vortex.KeyCode.Down))
                    translation.Y -= moveStep;
                if (Vortex.Input.IsKeyPressed(Vortex.KeyCode.A) || Vortex.Input.IsKeyPressed(Vortex.KeyCode.Left))
                    translation.X -= moveStep;
                if (Vortex.Input.IsKeyPressed(Vortex.KeyCode.D) || Vortex.Input.IsKeyPressed(Vortex.KeyCode.Right))
                    translation.X += moveStep;

                transformComponent.Translation = translation;
            }
        }

        protected override void OnDestroy()
        {
            Vortex.Log.Info("Player destroyed");
        }
    }
}
