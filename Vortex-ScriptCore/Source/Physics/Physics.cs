/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
using System.Runtime.InteropServices;

namespace Vortex
{
	/// <summary>Mirrors ManagedRaycastHit in ScriptEngine.cpp.</summary>
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct RaycastHit
	{
		public ulong ActorID;
		public Vector3 Position;
		public Vector3 Normal;
		public float Distance;
	}

	public static class Physics
	{
		/// <summary>
		/// Casts a ray into the running scene's physics world. Returns the closest hit within
		/// maxDistance, skipping the actor `ignore` (pass the caster so it doesn't hit itself).
		/// </summary>
		public static bool Raycast(Vector3 origin, Vector3 direction, float maxDistance,
		                           out RaycastHit hit, Actor? ignore = null)
		{
			RaycastHit result = default;
			bool found = false;
			ulong ignored = ignore is null ? 0xFFFFFFFFUL : ignore.ID;
			unsafe { InternalCalls.Physics_Raycast(&origin, &direction, maxDistance, ignored, &result, &found); }
			hit = result;
			return found;
		}

		public static bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, Actor? ignore = null)
			=> Raycast(origin, direction, maxDistance, out _, ignore);
	}
}
