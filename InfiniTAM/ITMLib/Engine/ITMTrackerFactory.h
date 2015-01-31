// Copyright 2014 Isis Innovation Limited and the authors of InfiniTAM

#pragma once

#include <map>
#include <stdexcept>

#include "ITMCompositeTracker.h"
#include "ITMIMUTracker.h"
#include "ITMLowLevelEngine.h"
#include "ITMTracker.h"

#include "DeviceSpecific/CPU/ITMDepthTracker_CPU.h"
#include "../Utils/ITMLibSettings.h"

#ifndef COMPILE_WITHOUT_CUDA
#include "DeviceSpecific/CUDA/ITMDepthTracker_CUDA.h"
#endif

#ifdef COMPILE_WITH_METAL
#include "DeviceSpecific/Metal/ITMDepthTracker_Metal.h"
#endif

namespace ITMLib
{
  namespace Engine
  {
    /**
     * \brief An instance of this class can be used to construct trackers.
     */
    template <typename TVoxel, typename TIndex>
    class ITMTrackerFactory
    {
      //#################### TYPEDEFS ####################
    private:
      typedef ITMTracker *(*Maker)(const Vector2i&,const ITMLibSettings*,ITMLowLevelEngine*,ITMIMUCalibrator*);

      //#################### PRIVATE VARIABLES ####################
    private:
      /** A map of maker functions for the various different tracker types. */
      std::map<ITMLibSettings::TrackerType,Maker> makers;

      //#################### SINGLETON IMPLEMENTATION ####################
    private:
      /**
       * \brief Constructs a tracker factory.
       */
      ITMTrackerFactory()
      {
        makers.insert(std::make_pair(ITMLibSettings::TRACKER_ICP, &MakeICPTracker));
        makers.insert(std::make_pair(ITMLibSettings::TRACKER_IMU, &MakeIMUTracker));
        // TODO
      }

    public:
      /**
       * \brief Gets the singleton instance for the current set of template parameters.
       */
      static ITMTrackerFactory& Instance()
      {
        static ITMTrackerFactory s_instance;
        return s_instance;
      }

      //#################### PUBLIC MEMBER FUNCTIONS ####################
    public:
      /**
       * \brief Makes a tracker of the specified type.
       */
      ITMTracker *Make(ITMLibSettings::TrackerType trackerType, const Vector2i& trackedImageSize, const ITMLibSettings *settings,
                       ITMLowLevelEngine *lowLevelEngine, ITMIMUCalibrator *imuCalibrator)
      {
        // TODO
        return NULL;
      }

      //#################### PRIVATE STATIC MEMBER FUNCTIONS ####################
    private:
      /**
       * \brief Makes an ICP tracker.
       */
      static ITMTracker *MakeICPTracker(const Vector2i& trackedImageSize, const ITMLibSettings *settings, ITMLowLevelEngine *lowLevelEngine,
                                        ITMIMUCalibrator *imuCalibrator)
      {
        switch(settings->deviceType)
        {
          case ITMLibSettings::DEVICE_CPU:
          {
            return new ITMDepthTracker_CPU(
              trackedImageSize,
              settings->trackingRegime,
              settings->noHierarchyLevels,
              settings->noICPRunTillLevel,
              settings->depthTrackerICPThreshold,
              lowLevelEngine
            );
          }
          case ITMLibSettings::DEVICE_CUDA:
          {
#ifndef COMPILE_WITHOUT_CUDA
            return new ITMDepthTracker_CUDA(
              trackedImageSize,
              settings->trackingRegime,
              settings->noHierarchyLevels,
              settings->noICPRunTillLevel,
							settings->depthTrackerICPThreshold,
              lowLevelEngine
            );
#else
            break;
#endif
          }
          case ITMLibSettings::DEVICE_METAL:
          {
#ifdef COMPILE_WITH_METAL
            return new ITMDepthTracker_Metal(
              trackedImageSize,
              settings->trackingRegime,
              settings->noHierarchyLevels,
              settings->noICPRunTillLevel,
              settings->depthTrackerICPThreshold,
              lowLevelEngine
            );
#else
            break;
#endif
          }
          default: break;
        }

        throw std::runtime_error("Failed to make ICP tracker");
      }

      /**
       * \brief Makes an IMU tracker.
       */
      static ITMTracker *MakeIMUTracker(const Vector2i& trackedImageSize, const ITMLibSettings *settings, ITMLowLevelEngine *lowLevelEngine,
                                        ITMIMUCalibrator *imuCalibrator)
      {
        switch(settings->deviceType)
        {
          case ITMLibSettings::DEVICE_CPU:
          {
            ITMCompositeTracker *compositeTracker = new ITMCompositeTracker(2);
            compositeTracker->SetTracker(new ITMIMUTracker(imuCalibrator), 0);
            compositeTracker->SetTracker(
              new ITMDepthTracker_CPU(
                trackedImageSize,
                settings->trackingRegime,
                settings->noHierarchyLevels,
                settings->noICPRunTillLevel,
                settings->depthTrackerICPThreshold,
                lowLevelEngine
              ), 1
            );
						return compositeTracker;
          }
          case ITMLibSettings::DEVICE_CUDA:
          {
#ifndef COMPILE_WITHOUT_CUDA
            ITMCompositeTracker *compositeTracker = new ITMCompositeTracker(2);
            compositeTracker->SetTracker(new ITMIMUTracker(imuCalibrator), 0);
            compositeTracker->SetTracker(
              new ITMDepthTracker_CUDA(
                trackedImageSize,
                settings->trackingRegime,
                settings->noHierarchyLevels,
                settings->noICPRunTillLevel,
                settings->depthTrackerICPThreshold,
                lowLevelEngine
              ), 1
            );
						return compositeTracker;
#else
            break;
#endif
          }
          case ITMLibSettings::DEVICE_METAL:
          {
#ifdef COMPILE_WITH_METAL
            ITMCompositeTracker *compositeTracker = new ITMCompositeTracker(2);
            compositeTracker->SetTracker(new ITMIMUTracker(imuCalibrator), 0);
            compositeTracker->SetTracker(
              new ITMDepthTracker_Metal(
                trackedImageSize,
                settings->trackingRegime,
                settings->noHierarchyLevels,
                settings->noICPRunTillLevel,
                settings->depthTrackerICPThreshold,
                lowLevelEngine
              ), 1
            );
            return compositeTracker;
#else
            break;
#endif
          }
          default: break;
        }

        throw std::runtime_error("Failed to make IMU tracker");
      }
    };
  }
}
