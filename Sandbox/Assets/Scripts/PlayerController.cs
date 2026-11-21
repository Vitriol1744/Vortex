/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
using Vortex;

namespace Gameplay
{
    /// <summary>
    /// A physics-driven character: WASD/arrows move it on the XZ plane, Shift sprints and
    /// Space jumps while it stands on something. Needs a RigidBodyComponent (dynamic, with
    /// LockRotation so it stays upright) and a collider.
    /// Falls back to moving the transform directly when the actor has no rigid body.
    /// </summary>
    public class PlayerController : Actor
    {
        public float MoveSpeed = 5.0f;
        public float SprintMultiplier = 1.8f;
        public float JumpSpeed = 6.0f;
        /// <summary>How quickly the horizontal velocity reaches the target (1/s).</summary>
        public float Acceleration = 12.0f;
        /// <summary>Seconds between jumps, so one press doesn't fire on consecutive frames.</summary>
        public float JumpCooldown = 0.25f;

        private RigidBodyComponent? m_RigidBody;
        private float m_JumpTimer;
        private bool m_WasGrounded;

        protected override void OnCreate()
        {
            m_RigidBody = GetComponent<RigidBodyComponent>();
            if (m_RigidBody == null)
                Log.Warn("PlayerController on '{0}' has no RigidBodyComponent: moving its transform", Tag);
            else
                Log.Info("PlayerController ready on '{0}'", Tag);
        }

        protected override void OnUpdate(float deltaTime)
        {
            Vector3 input = ReadInput();
            float speed = MoveSpeed * (Input.IsKeyPressed(KeyCode.LShift) ? SprintMultiplier : 1.0f);

            if (m_RigidBody == null)
            {
                Translation += input * speed * deltaTime;
                return;
            }

            m_JumpTimer -= deltaTime;
            bool grounded = IsGrounded();
            if (grounded && !m_WasGrounded)
                Log.Trace("'{0}' landed", Tag);
            m_WasGrounded = grounded;

            Vector3 velocity = m_RigidBody.LinearVelocity;

            // Steer the horizontal velocity towards the input; gravity keeps Y.
            float blend = Mathf.Clamp(Acceleration * deltaTime, 0.0f, 1.0f);
            velocity.X += (input.X * speed - velocity.X) * blend;
            velocity.Z += (input.Z * speed - velocity.Z) * blend;

            if (grounded && m_JumpTimer <= 0.0f && Input.IsKeyPressed(KeyCode.Space))
            {
                velocity.Y = JumpSpeed;
                m_JumpTimer = JumpCooldown;
            }

            m_RigidBody.LinearVelocity = velocity;
        }

        protected override void OnDestroy()
        {
            Log.Info("PlayerController on '{0}' destroyed", Tag);
        }

        private static Vector3 ReadInput()
        {
            Vector3 input = Vector3.Zero;
            if (Input.IsKeyPressed(KeyCode.W) || Input.IsKeyPressed(KeyCode.Up))
                input.Z -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.S) || Input.IsKeyPressed(KeyCode.Down))
                input.Z += 1.0f;
            if (Input.IsKeyPressed(KeyCode.A) || Input.IsKeyPressed(KeyCode.Left))
                input.X -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.D) || Input.IsKeyPressed(KeyCode.Right))
                input.X += 1.0f;

            float length = input.Length();
            return length > 0.0f ? input / length : input;
        }

        /// <summary>A short ray down from the centre, just past the bottom of the collider.</summary>
        private bool IsGrounded()
        {
            float halfHeight = Scale.Y * 0.5f;
            return Physics.Raycast(Translation, Vector3.Down, halfHeight + 0.1f, this);
        }
    }
}
