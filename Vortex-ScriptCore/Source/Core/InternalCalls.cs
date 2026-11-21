/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
using Coral.Managed.Interop;
using System;

namespace Vortex
{
    internal static unsafe class InternalCalls
    {
#pragma warning disable CS0649
        internal static delegate* unmanaged<KeyCode, bool*, void> Input_IsKeyPressed;
        internal static delegate* unmanaged<KeyCode, Bool32> Input_IsKeyHeld;
        internal static delegate* unmanaged<KeyCode, Bool32> Input_IsKeyDown;
        internal static delegate* unmanaged<KeyCode, bool*, void> Input_IsKeyReleased;
        internal static delegate* unmanaged<MouseCode, bool> Input_IsMouseButtonPressed;
        internal static delegate* unmanaged<MouseCode, bool> Input_IsMouseButtonHeld;
        internal static delegate* unmanaged<MouseCode, bool> Input_IsMouseButtonDown;
        internal static delegate* unmanaged<MouseCode, bool> Input_IsMouseButtonReleased;
        internal static delegate* unmanaged<Vector2*, void> Input_GetMousePosition;
        internal static delegate* unmanaged<CursorMode, void> Input_SetCursorMode;
        internal static delegate* unmanaged<CursorMode> Input_GetCursorMode;

        internal static delegate* unmanaged<Log.LogLevel, NativeString, void> Log_LogMessage;


		internal static delegate* unmanaged<Vector3*, Vector3*, Vector3*, Matrix4*, void> Matrix4_LookAt;

		internal static delegate* unmanaged<NativeString, ulong> Scene_FindActorByTag;
		internal static delegate* unmanaged<NativeArray<ulong>> Scene_GetActors;
		internal static delegate* unmanaged<ulong, bool> Scene_IsActorValid;
		internal static delegate* unmanaged<NativeString, ulong> Scene_CreateActor;
		internal static delegate* unmanaged<ulong, void> Scene_DestroyActor;
		internal static delegate* unmanaged<ulong, void> Scene_DestroyAllChildren;
		internal static delegate* unmanaged<ulong, NativeArray<ulong>> Scene_GetChildrenIDs;
		internal static delegate* unmanaged<float, void> Scene_SetTimeScale;

#region Actor
		internal static delegate* unmanaged<ulong, ulong> Actor_GetParent;
		internal static delegate* unmanaged<ulong, ulong, void> Actor_SetParent;
		internal static delegate* unmanaged<ulong, NativeArray<ulong>> Actor_GetChildren;
		internal static delegate* unmanaged<ulong, ReflectionType, void> Actor_CreateComponent;
		internal static delegate* unmanaged<ulong, ReflectionType, bool> Actor_HasComponent;
		internal static delegate* unmanaged<ulong, ReflectionType, bool> Actor_RemoveComponent;
#endregion

		#region TagComponent
		internal static delegate* unmanaged<ulong, NativeString> TagComponent_GetTag;
		internal static delegate* unmanaged<ulong, NativeString, void> TagComponent_SetTag;
		#endregion

#region TransformComponent

		internal static delegate* unmanaged<ulong, Transform*, void> TransformComponent_GetTransform;
		internal static delegate* unmanaged<ulong, Transform*, void> TransformComponent_SetTransform;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_GetTranslation;
		internal static delegate* unmanaged<ulong, Vector3, void> TransformComponent_SetTranslation;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_GetRotation;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_SetRotation;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_GetScale;
		internal static delegate* unmanaged<ulong, Vector3*, void> TransformComponent_SetScale;
		// internal static delegate* unmanaged<ulong, Transform*, void> TransformComponent_GetWorldSpaceTransform;
		internal static delegate* unmanaged<ulong, Matrix4*, void> TransformComponent_GetTransformMatrix;
		internal static delegate* unmanaged<ulong, Matrix4*, void> TransformComponent_SetTransformMatrix;
		internal static delegate* unmanaged<ulong, Quaternion*, void> TransformComponent_GetRotationQuat;
		internal static delegate* unmanaged<ulong, Quaternion*, void> TransformComponent_SetRotationQuat;

		#endregion

        internal static delegate* unmanaged<Transform*, Transform*, Transform*, void> TransformMultiply_Native;

		#region RigidBodyComponent
		internal static delegate* unmanaged<ulong, Vector3*, void> RigidBodyComponent_GetLinearVelocity;
		internal static delegate* unmanaged<ulong, Vector3*, void> RigidBodyComponent_SetLinearVelocity;
		internal static delegate* unmanaged<ulong, Vector3*, ForceMode, void> RigidBodyComponent_AddForce;
		#endregion

		#region Physics
		internal static delegate* unmanaged<Vector3*, Vector3*, float, ulong, RaycastHit*, bool*, void> Physics_Raycast;
		#endregion
#pragma warning restore CS0649
    }
}
