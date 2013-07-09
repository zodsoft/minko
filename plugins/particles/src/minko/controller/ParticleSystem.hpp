/*
Copyright (c) 2013 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <vector>

#include "minko/ParticlesCommon.hpp"
#include "minko/controller/AbstractController.hpp"

namespace minko
{
	namespace controller
	{
		class ParticleSystem :
			public AbstractController,
			public std::enable_shared_from_this<ParticleSystem>
		{
		public:
			typedef std::shared_ptr<ParticleSystem>	Ptr;

		private:
			typedef std::shared_ptr<scene::NodeSet>								NodeSetPtr;
			typedef std::shared_ptr<scene::Node>								NodePtr;
			typedef std::shared_ptr<AbstractController>							AbsCtrlPtr;
			typedef std::shared_ptr<RenderingController>						RenderingCtrlPtr;
			
			typedef std::shared_ptr<Surface>									SurfacePtr;
			typedef std::shared_ptr<geometry::Geometry>							GeometryPtr;
			typedef std::shared_ptr<data::Provider>								ProviderPtr;
			typedef std::shared_ptr<render::Effect>								EffectPtr;

			typedef std::shared_ptr<particle::shape::EmitterShape>				ShapePtr;
			typedef std::shared_ptr<particle::sampler::Sampler<float> >			FloatSamplerPtr;
			typedef std::shared_ptr<particle::modifier::IParticleInitializer>	IInitializerPtr;
			typedef std::shared_ptr<particle::modifier::IParticleUpdater>		IUpdaterPtr;
			typedef std::shared_ptr<particle::modifier::IParticleModifier>		ModifierPtr;	

		private:
			class ParticleDistanceToCameraComparison
			{
			public:
				controller::ParticleSystem*	system;

				inline
				bool
				operator() (unsigned int p1Index, unsigned int p2Index)
				{
					return system->getParticleSquaredDistanceToCamera(p1Index)
						 > system->getParticleSquaredDistanceToCamera(p2Index);
				};
			};	

		private:
			std::map<RenderingCtrlPtr, Signal<RenderingCtrlPtr>::Slot>	_enterFrameSlots;
			Signal<AbsCtrlPtr, NodePtr>::Slot							_targetAddedSlot;
			Signal<AbsCtrlPtr, NodePtr>::Slot							_targetRemovedSlot;
			Signal<NodeSetPtr, NodePtr>::Slot							_rendererAddedSlot;
			Signal<NodeSetPtr, NodePtr>::Slot							_rendererRemovedSlot;
			
			NodeSetPtr													_renderers;

			unsigned int 												_particlesCountLimit;
			unsigned int												_maxParticlesCount;
			unsigned int												_liveParticlesCount;
			std::vector<IInitializerPtr> 								_initializers;
			std::vector<IUpdaterPtr> 									_updaters;	
			std::vector<particle::ParticleData>							_particles;
			std::vector<unsigned int>									_particleOrder;
			std::vector<float>											_particleDistanceToCamera;

			bool														_isInWorldSpace;
			float 														_localToWorld[16];
			bool														_isZSorted;
			float 														_cameraCoords[3];
			ParticleDistanceToCameraComparison							_comparisonObject;
			bool														_useOldPosition;

			float														_rate;
			FloatSamplerPtr												_lifetime;
			ShapePtr													_shape;
			particle::StartDirection									_startDirection;
			FloatSamplerPtr 											_startVelocity;
			
			float														_createTimer;

			int															_format;
			unsigned int												_floatsPerVertex;
			std::vector<float>											_vertexStream;
			
		public:
			static
			Ptr
			create(float					rate,
				   FloatSamplerPtr			lifetime,
				   ShapePtr					shape,
				   particle::StartDirection	startDirection,
				   FloatSamplerPtr 			startVelocity)
			{
				Ptr system = std::shared_ptr<ParticleSystem> (new ParticleSystem(rate,
																				 lifetime,
																				 shape,
																				 startDirection,
																				 startVelocity));

				system->initialize();

				return system;
			};
			
			inline
			void 
			rate(float value)
			{
				_rate =  1 / value;

				updateMaxParticlesCount();
			};

			inline
			void 
			lifetime(FloatSamplerPtr value)
			{
				_lifetime = value;

				updateMaxParticlesCount();
			};
			
			inline
			void
				shape(ShapePtr value)
			{
				_shape = value;
			};
			
			inline
			void
			startDirection(particle::StartDirection value)
			{
				_startDirection = value;
			};
			
			inline
			void
			startVelocity(FloatSamplerPtr value)
			{
				_startVelocity = value;
			};

		public:
			void
			updateSystem(float		timeStep,
					     bool		emit);
			
			void
			reset();

		public:
			void
			add(ModifierPtr 	modifier);
			
			void
			remove(ModifierPtr 	modifier);
			
			bool
			has(ModifierPtr 	modifier);

		public:
			inline
			void
			isInWorldSpace(bool value)
			{
				_isInWorldSpace = value;
			};
			
			inline
			float*
			localToWorld()
			{
				return _localToWorld;
			}
			
			inline
			void  
			iIsZSorted(bool value)
			{
				_isZSorted = value;
				resizeParticlesVector();
			};
			
			inline
			float*
			cameraPos()
			{
				return _cameraCoords;
			};

			inline
			void
			useOldPosition(bool value)
			{
				if (value == _useOldPosition)
					return;

				_useOldPosition = value;
				updateVertexFormat();
			};

			inline
			float
			getParticleSquaredDistanceToCamera(unsigned int particleIndex)
			{
				return _particleDistanceToCamera[particleIndex];
			};
			
			void
			updateParticleDistancesToCamera();

		public:
			inline
			unsigned int
			maxParticlesCount() const
			{
				return _maxParticlesCount;
			};
			
			void
			updateMaxParticlesCount();
			
			inline
			unsigned int
			liveParticlesCount() const
			{
				return _liveParticlesCount;
			};
			
			inline
			void
			particlesCountLimit(unsigned int value)
			{
				_particlesCountLimit = value;

				updateMaxParticlesCount();
			};
			
			inline
			std::vector<particle::ParticleData>&
			getParticles()
			{
				return _particles;
			};

			void
			createParticle(unsigned int 						particleIndex,
						   const particle::shape::EmitterShape&	emitter,
						   float								timeLived);
			
			void
			killParticle(unsigned int							particleIndex);
			
		public:
			inline
			unsigned int
			formatFlags() const
			{
				return _format;
			};

			inline
			std::vector<float>&
			getVertexStream()
			{
				updateVertexStream();
				return _vertexStream;
			};
			
			unsigned int
			updateVertexFormat();

		private:
			void
			resizeParticlesVector();
			
			void
			addComponents(unsigned int components, bool blockVSInit = false);

			void
			initVertexStream();
			
			inline
			void
			setInVertexStream(float* ptr, unsigned int offset, float value)
			{
				*(ptr + offset) = value;
				*(ptr + offset + _floatsPerVertex) = value;
				*(ptr + offset + _floatsPerVertex * 2) = value;
				*(ptr + offset + _floatsPerVertex * 3) = value;
			};
			
			void
			updateVertexStream();

		protected:
			ParticleSystem(float					rate,
						   FloatSamplerPtr			lifetime,
						   ShapePtr					shape,
						   particle::StartDirection	startDirection,
						   FloatSamplerPtr 			startVelocity);

			void
			initialize();

			void
			targetAddedHandler(AbsCtrlPtr ctrl, NodePtr target);

			void
			targetRemovedHandler(AbsCtrlPtr ctrl, NodePtr target);
			
			void
			rendererAddedHandler(NodeSetPtr renderers, NodePtr rendererNode);

			void
			rendererRemovedHandler(NodeSetPtr renderers, NodePtr rendererNode);

			void
			enterFrameHandler(RenderingCtrlPtr renderer);
		};
	}
}