/*
 * Created by v1tr10l7 on 21.10.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
using System;
using System.Collections.Generic;
using Coral.Managed.Interop;

namespace Vortex
{
	// [EditorAssignable]
	public class Actor : IEquatable<Actor>
	{
		public event Action<Actor>? DestroyedEvent;
		private Dictionary<Type, Component> m_ComponentCache = new Dictionary<Type, Component>();

		protected Actor() { ID = 0; }

		internal Actor(ulong id)
		{
			ID = id;
		}

		public readonly ulong ID;
		public string Tag => GetComponent<TagComponent>()?.Tag ?? "";
		private TransformComponent m_TransformComponent;

		protected virtual void OnCreate() { }
		protected virtual void OnUpdate(float ts) { }
		protected virtual void OnDestroy() { }

        public TransformComponent Transform
		{
			get
			{
				if (m_TransformComponent == null)
					m_TransformComponent = GetComponent<TransformComponent>();

				return m_TransformComponent;
			}
		}

		public Vector3 Translation
		{
			get => Transform.Translation;
			set => Transform.Translation = value;
		}

		public Vector3 Rotation
		{
			get => Transform.Rotation;
			set => Transform.Rotation = value;
		}

		public Quaternion RotationQuat
		{
			get => Transform.RotationQuat;
			set => Transform.RotationQuat = value;
		}

		public Vector3 Scale
		{
			get => Transform.Scale;
			set => Transform.Scale = value;
		}
		public void SetRotation(Quaternion rotation)
		{
			GetComponent<TransformComponent>()?.SetRotation(rotation);
		}

        public T? CreateComponent<T>() where T : Component, new()
		{
			if (HasComponent<T>())
				return GetComponent<T>();

			unsafe { InternalCalls.Actor_CreateComponent(ID, typeof(T)); }
			var component = new T { Actor = this };
			m_ComponentCache.Add(typeof(T), component);
			return component;
		}

		public bool HasComponent<T>() where T : Component
		{
			unsafe { return InternalCalls.Actor_HasComponent(ID, typeof(T)); }
		}

		public bool HasComponent(Type type)
		{
			unsafe { return InternalCalls.Actor_HasComponent(ID, type); }
		}

		public T? GetComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);

			if (!HasComponent<T>())
			{
				m_ComponentCache.Remove(componentType);
				return null;
			}

			if (!m_ComponentCache.ContainsKey(componentType))
			{
				var component = new T { Actor = this };
				m_ComponentCache.Add(componentType, component);
				return component;
			}

			return m_ComponentCache[componentType] as T;
		}

		public bool RemoveComponent<T>() where T : Component
		{
			Type componentType = typeof(T);
			bool removed;

			unsafe { removed = InternalCalls.Actor_RemoveComponent(ID, componentType); }

			if (removed && m_ComponentCache.ContainsKey(componentType))
				m_ComponentCache.Remove(componentType);

			return removed;
		}


		public override bool Equals(object? obj) => obj is Actor other && Equals(other);

		public bool Equals(Actor? other)
		{
			if (other is null)
				return false;

			if (ReferenceEquals(this, other))
				return true;

			return ID == other.ID;
		}

		private static bool IsValid(Actor? entity)
		{
			if (entity is null) 
				return false;
			
			unsafe { return InternalCalls.Scene_IsActorValid(entity.ID); }
		}

		public override int GetHashCode() => (int)ID;

		public static bool operator ==(Actor? entityA, Actor? entityB) => entityA is null ? entityB is null : entityA.Equals(entityB);
		public static bool operator !=(Actor? entityA, Actor? entityB) => !(entityA == entityB);
		
		public static implicit operator bool(Actor entity) => IsValid(entity);

	}
}
