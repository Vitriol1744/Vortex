/*
 * Created by v1tr10l7 on 21.10.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
using System;
using Coral.Managed.Interop;

namespace Vortex
{
    public abstract class Component
    {
        public Actor Actor { get; internal set; }
    }

    public class TagComponent : Component
    {
        public string? Tag
        {
            get { unsafe { return InternalCalls.TagComponent_GetTag(Actor.ID); } }
            set { unsafe { InternalCalls.TagComponent_SetTag(Actor.ID, value); } }
        }    
    }

	public class TransformComponent : Component
	{
		public Transform LocalTransform
		{
			get
			{
				unsafe
				{
					Transform transform;
					InternalCalls.TransformComponent_GetTransform(Actor.ID, &transform);
					return transform;
				}
			}

			set
			{
				unsafe { InternalCalls.TransformComponent_SetTransform(Actor.ID, &value); }
			}
		}

		// public Transform WorldTransform
		// {
		// 	get
		// 	{
		// 		Transform result;
		// 		unsafe { InternalCalls.TransformComponent_GetWorldSpaceTransform(Actor.ID, &result); }
		// 		return result;
		// 	}
		// }

		public Vector3 Translation
		{
			get
			{
				Vector3 result;
				unsafe { InternalCalls.TransformComponent_GetTranslation(Actor.ID, &result); }
				return result;
			}

			set
			{
				unsafe { InternalCalls.TransformComponent_SetTranslation(Actor.ID, value); }
			}
		}

		public Vector3 Rotation
		{
			get
			{
				Vector3 result;
				unsafe { InternalCalls.TransformComponent_GetRotation(Actor.ID, &result); }
				return result;
			}

			set
			{
				unsafe { InternalCalls.TransformComponent_SetRotation(Actor.ID, &value); }
			}
		}

		public Quaternion RotationQuat
		{
			get
			{
				Quaternion result;
				unsafe { InternalCalls.TransformComponent_GetRotationQuat(Actor.ID, &result); }
				return result;
			}

			set
			{
				unsafe { InternalCalls.TransformComponent_SetRotationQuat(Actor.ID, &value); }
			}
		}

		public Vector3 Scale
		{
			get
			{
				Vector3 result;
				unsafe { InternalCalls.TransformComponent_GetScale(Actor.ID, &result); }
				return result;
			}

			set
			{
				unsafe { InternalCalls.TransformComponent_SetScale(Actor.ID, &value); }
			}
		}

		public Matrix4 TransformMatrix
		{
			get
			{
				Matrix4 result;
				unsafe { InternalCalls.TransformComponent_GetTransformMatrix(Actor.ID, &result); }
				return result;
			}

			set
			{
				unsafe { InternalCalls.TransformComponent_SetTransformMatrix(Actor.ID, &value); }
			}
		}

		public void SetRotation(Quaternion rotation)
		{
			unsafe { InternalCalls.TransformComponent_SetRotationQuat(Actor.ID, &rotation); }
		}
	}
}

namespace Vortex
{
	public enum ForceMode
	{
		/// <summary>Continuous force (mass * distance / time^2), applied over the next frame.</summary>
		Force = 0,
		/// <summary>Instant change in momentum (mass * distance / time).</summary>
		Impulse = 1,
	}

	/// <summary>
	/// The actor's physics body. Changes reach the simulation on its next step.
	/// </summary>
	public class RigidBodyComponent : Component
	{
		public Vector3 LinearVelocity
		{
			get
			{
				Vector3 result;
				unsafe { InternalCalls.RigidBodyComponent_GetLinearVelocity(Actor.ID, &result); }
				return result;
			}

			set
			{
				unsafe { InternalCalls.RigidBodyComponent_SetLinearVelocity(Actor.ID, &value); }
			}
		}

		public void AddForce(Vector3 force, ForceMode mode = ForceMode.Force)
		{
			unsafe { InternalCalls.RigidBodyComponent_AddForce(Actor.ID, &force, mode); }
		}

		public void AddImpulse(Vector3 impulse) => AddForce(impulse, ForceMode.Impulse);
	}

	public class BoxColliderComponent : Component { }

	public class SphereColliderComponent : Component { }
}
