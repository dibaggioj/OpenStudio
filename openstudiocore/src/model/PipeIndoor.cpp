/**********************************************************************
 *  Copyright (c) 2008-2016, Alliance for Sustainable Energy.
 *  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/

#include "PipeIndoor.hpp"
#include "PipeIndoor_Impl.hpp"

#include "Construction.hpp"
#include "Construction_Impl.hpp"
#include "Node.hpp"
#include "Node_Impl.hpp"
#include "Schedule.hpp"
#include "Schedule_Impl.hpp"
#include "ScheduleTypeLimits.hpp"
#include "ScheduleTypeRegistry.hpp"
#include "ThermalZone.hpp"
#include "ThermalZone_Impl.hpp"

#include <utilities/idd/IddEnums.hxx>
#include <utilities/idd/IddFactory.hxx>
#include <utilities/idd/OS_Pipe_Indoor_FieldEnums.hxx>

#include "../utilities/core/Assert.hpp"
#include "../utilities/units/Unit.hpp"

namespace openstudio {
namespace model {

namespace detail {

  PipeIndoor_Impl::PipeIndoor_Impl(const IdfObject& idfObject,
                                   Model_Impl* model,
                                   bool keepHandle)
    :  StraightComponent_Impl(idfObject,model,keepHandle)
  {
    OS_ASSERT(idfObject.iddObject().type() == PipeIndoor::iddObjectType());
  }

  PipeIndoor_Impl::PipeIndoor_Impl(const openstudio::detail::WorkspaceObject_Impl& other,
                                   Model_Impl* model,
                                   bool keepHandle)
    :  StraightComponent_Impl(other,model,keepHandle)
  {
    OS_ASSERT(other.iddObject().type() == PipeIndoor::iddObjectType());
  }

  PipeIndoor_Impl::PipeIndoor_Impl(const PipeIndoor_Impl& other,
                                   Model_Impl* model,
                                   bool keepHandle)
    :  StraightComponent_Impl(other,model,keepHandle)
  {}

  const std::vector<std::string>& PipeIndoor_Impl::outputVariableNames() const
  {
    static std::vector<std::string> result;
    if (result.empty()){
    }
    return result;
  }

  IddObjectType PipeIndoor_Impl::iddObjectType() const {
    return PipeIndoor::iddObjectType();
  }

  unsigned PipeIndoor_Impl::inletPort()
  {
    return OS_Pipe_IndoorFields::FluidInletNode;
  }

  unsigned PipeIndoor_Impl::outletPort()
  {
    return OS_Pipe_IndoorFields::FluidOutletNode;
  }

  std::vector<ScheduleTypeKey> PipeIndoor_Impl::getScheduleTypeKeys(const Schedule& schedule) const
  {
    std::vector<ScheduleTypeKey> result;
    UnsignedVector fieldIndices = getSourceIndices(schedule.handle());
    UnsignedVector::const_iterator b(fieldIndices.begin()), e(fieldIndices.end());
    if (std::find(b,e,OS_Pipe_IndoorFields::AmbientTemperatureSchedule) != e)
    {
      result.push_back(ScheduleTypeKey("PipeIndoor","Ambient Temperature Schedule"));
    }
    if (std::find(b,e,OS_Pipe_IndoorFields::AmbientAirVelocitySchedule) != e)
    {
      result.push_back(ScheduleTypeKey("PipeIndoor","Ambient Air Velocity Schedule"));
    }
    return result;
  }

  boost::optional<Construction> PipeIndoor_Impl::construction() const {
    return getObject<ModelObject>().getModelObjectTarget<Construction>(OS_Pipe_IndoorFields::Construction);
  }

  std::string PipeIndoor_Impl::environmentType() const {
    boost::optional<std::string> value = getString(OS_Pipe_IndoorFields::EnvironmentType,true);
    OS_ASSERT(value);
    return value.get();
  }

  boost::optional<ThermalZone> PipeIndoor_Impl::ambientTemperatureZone() const {
    return getObject<ModelObject>().getModelObjectTarget<ThermalZone>(OS_Pipe_IndoorFields::AmbientTemperatureZone);
  }

  boost::optional<Schedule> PipeIndoor_Impl::ambientTemperatureSchedule() const {
    return getObject<ModelObject>().getModelObjectTarget<Schedule>(OS_Pipe_IndoorFields::AmbientTemperatureSchedule);
  }

  boost::optional<Schedule> PipeIndoor_Impl::ambientAirVelocitySchedule() const {
    return getObject<ModelObject>().getModelObjectTarget<Schedule>(OS_Pipe_IndoorFields::AmbientAirVelocitySchedule);
  }

  double PipeIndoor_Impl::pipeInsideDiameter() const {
    boost::optional<double> value = getDouble(OS_Pipe_IndoorFields::PipeInsideDiameter,true);
    OS_ASSERT(value);
    return value.get();
  }

  double PipeIndoor_Impl::pipeLength() const {
    boost::optional<double> value = getDouble(OS_Pipe_IndoorFields::PipeLength,true);
    OS_ASSERT(value);
    return value.get();
  }

  bool PipeIndoor_Impl::setConstruction(const boost::optional<Construction>& construction) {
    bool result(false);
    if (construction) {
      result = setPointer(OS_Pipe_IndoorFields::Construction, construction.get().handle());
    }
    else {
      resetConstruction();
      result = true;
    }
    return result;
  }

  void PipeIndoor_Impl::resetConstruction() {
    bool result = setString(OS_Pipe_IndoorFields::Construction, "");
    OS_ASSERT(result);
  }

  bool PipeIndoor_Impl::setEnvironmentType(std::string environmentType) {
    bool result = setString(OS_Pipe_IndoorFields::EnvironmentType, environmentType);
    return result;
  }

  bool PipeIndoor_Impl::setAmbientTemperatureZone(const boost::optional<ThermalZone>& thermalZone) {
    bool result(false);
    if (thermalZone) {
      result = setPointer(OS_Pipe_IndoorFields::AmbientTemperatureZone, thermalZone.get().handle());
    }
    else {
      resetAmbientTemperatureZone();
      result = true;
    }
    return result;
  }

  void PipeIndoor_Impl::resetAmbientTemperatureZone() {
    bool result = setString(OS_Pipe_IndoorFields::AmbientTemperatureZone, "");
    OS_ASSERT(result);
  }

  bool PipeIndoor_Impl::setAmbientTemperatureSchedule(Schedule& schedule) {
    bool result = setSchedule(OS_Pipe_IndoorFields::AmbientTemperatureSchedule,
                              "PipeIndoor",
                              "Ambient Temperature Schedule",
                              schedule);
    return result;
  }

  void PipeIndoor_Impl::resetAmbientTemperatureSchedule() {
    bool result = setString(OS_Pipe_IndoorFields::AmbientTemperatureSchedule, "");
    OS_ASSERT(result);
  }

  bool PipeIndoor_Impl::setAmbientAirVelocitySchedule(Schedule& schedule) {
    bool result = setSchedule(OS_Pipe_IndoorFields::AmbientAirVelocitySchedule,
                              "PipeIndoor",
                              "Ambient Air Velocity Schedule",
                              schedule);
    return result;
  }

  void PipeIndoor_Impl::resetAmbientAirVelocitySchedule() {
    bool result = setString(OS_Pipe_IndoorFields::AmbientAirVelocitySchedule, "");
    OS_ASSERT(result);
  }

  bool PipeIndoor_Impl::setPipeInsideDiameter(double pipeInsideDiameter) {
    bool result = setDouble(OS_Pipe_IndoorFields::PipeInsideDiameter, pipeInsideDiameter);
    return result;
  }

  bool PipeIndoor_Impl::setPipeLength(double pipeLength) {
    bool result = setDouble(OS_Pipe_IndoorFields::PipeLength, pipeLength);
    return result;
  }

  bool PipeIndoor_Impl::addToNode(Node & node)
  {
    if(node.plantLoop()) {
      return StraightComponent_Impl::addToNode(node);
    }

    return false;
  }

} // detail

PipeIndoor::PipeIndoor(const Model& model)
  :  StraightComponent(PipeIndoor::iddObjectType(),model)
{
  OS_ASSERT(getImpl<detail::PipeIndoor_Impl>());

  bool ok = true;
  // ok = setHandle();
  OS_ASSERT(ok);
  ok = setEnvironmentType("Zone"); // e+ default Zone
  OS_ASSERT(ok);
  ok = setPipeInsideDiameter(0.05);
  OS_ASSERT(ok);
  ok = setPipeLength(100.0);
  OS_ASSERT(ok);
}

IddObjectType PipeIndoor::iddObjectType() {
  return IddObjectType(IddObjectType::OS_Pipe_Indoor);
}

std::vector<std::string> PipeIndoor::environmentTypeValues() {
  return getIddKeyNames(IddFactory::instance().getObject(iddObjectType()).get(),
                        OS_Pipe_IndoorFields::EnvironmentType);
}

boost::optional<Construction> PipeIndoor::construction() const {
  return getImpl<detail::PipeIndoor_Impl>()->construction();
}

std::string PipeIndoor::environmentType() const {
  return getImpl<detail::PipeIndoor_Impl>()->environmentType();
}

boost::optional<ThermalZone> PipeIndoor::ambientTemperatureZone() const {
  return getImpl<detail::PipeIndoor_Impl>()->ambientTemperatureZone();
}

boost::optional<Schedule> PipeIndoor::ambientTemperatureSchedule() const {
  return getImpl<detail::PipeIndoor_Impl>()->ambientTemperatureSchedule();
}

boost::optional<Schedule> PipeIndoor::ambientAirVelocitySchedule() const {
  return getImpl<detail::PipeIndoor_Impl>()->ambientAirVelocitySchedule();
}

double PipeIndoor::pipeInsideDiameter() const {
  return getImpl<detail::PipeIndoor_Impl>()->pipeInsideDiameter();
}

double PipeIndoor::pipeLength() const {
  return getImpl<detail::PipeIndoor_Impl>()->pipeLength();
}

bool PipeIndoor::setConstruction(const Construction& construction) {
  return getImpl<detail::PipeIndoor_Impl>()->setConstruction(construction);
}

void PipeIndoor::resetConstruction() {
  getImpl<detail::PipeIndoor_Impl>()->resetConstruction();
}

bool PipeIndoor::setEnvironmentType(std::string environmentType) {
  return getImpl<detail::PipeIndoor_Impl>()->setEnvironmentType(environmentType);
}

bool PipeIndoor::setAmbientTemperatureZone(const ThermalZone& thermalZone) {
  return getImpl<detail::PipeIndoor_Impl>()->setAmbientTemperatureZone(thermalZone);
}

void PipeIndoor::resetAmbientTemperatureZone() {
  getImpl<detail::PipeIndoor_Impl>()->resetAmbientTemperatureZone();
}

bool PipeIndoor::setAmbientTemperatureSchedule(Schedule& schedule) {
  return getImpl<detail::PipeIndoor_Impl>()->setAmbientTemperatureSchedule(schedule);
}

void PipeIndoor::resetAmbientTemperatureSchedule() {
  getImpl<detail::PipeIndoor_Impl>()->resetAmbientTemperatureSchedule();
}

bool PipeIndoor::setAmbientAirVelocitySchedule(Schedule& schedule) {
  return getImpl<detail::PipeIndoor_Impl>()->setAmbientAirVelocitySchedule(schedule);
}

void PipeIndoor::resetAmbientAirVelocitySchedule() {
  getImpl<detail::PipeIndoor_Impl>()->resetAmbientAirVelocitySchedule();
}

bool PipeIndoor::setPipeInsideDiameter(double pipeInsideDiameter) {
  return getImpl<detail::PipeIndoor_Impl>()->setPipeInsideDiameter(pipeInsideDiameter);
}

bool PipeIndoor::setPipeLength(double pipeLength) {
  return getImpl<detail::PipeIndoor_Impl>()->setPipeLength(pipeLength);
}

/// @cond
PipeIndoor::PipeIndoor(std::shared_ptr<detail::PipeIndoor_Impl> impl)
  :  StraightComponent(impl)
{}
/// @endcond

} // model
} // openstudio

