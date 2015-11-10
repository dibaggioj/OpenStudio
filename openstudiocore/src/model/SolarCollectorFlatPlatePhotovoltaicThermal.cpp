/**********************************************************************
 *  Copyright (c) 2008-2015, Alliance for Sustainable Energy.
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

#include "SolarCollectorFlatPlatePhotovoltaicThermal.hpp"
#include "SolarCollectorFlatPlatePhotovoltaicThermal_Impl.hpp"

#include "PlanarSurface.hpp"
#include "PlanarSurface_Impl.hpp"
#include "Surface.hpp"
#include "Surface_Impl.hpp"
#include "ShadingSurface.hpp"
#include "ShadingSurface_Impl.hpp"
#include "SolarCollectorPerformancePhotovoltaicThermalSimple.hpp"
#include "SolarCollectorPerformancePhotovoltaicThermalSimple_Impl.hpp"
#include "Node.hpp"
#include "Node_Impl.hpp"
#include "GeneratorPhotovoltaic.hpp"
#include "GeneratorPhotovoltaic_Impl.hpp"
#include "Model.hpp"
#include "AirLoopHVACOutdoorAirSystem.hpp"

#include <utilities/idd/OS_SolarCollector_FlatPlate_PhotovoltaicThermal_FieldEnums.hxx>
#include <utilities/idd/IddEnums.hxx>

#include "../utilities/units/Unit.hpp"

#include "../utilities/core/Assert.hpp"

namespace openstudio {
namespace model {

namespace detail {

  SolarCollectorFlatPlatePhotovoltaicThermal_Impl::SolarCollectorFlatPlatePhotovoltaicThermal_Impl(const IdfObject& idfObject,
                                                                                                   Model_Impl* model,
                                                                                                   bool keepHandle)
    : StraightComponent_Impl(idfObject,model,keepHandle)
  {
    OS_ASSERT(idfObject.iddObject().type() == SolarCollectorFlatPlatePhotovoltaicThermal::iddObjectType());
  }

  SolarCollectorFlatPlatePhotovoltaicThermal_Impl::SolarCollectorFlatPlatePhotovoltaicThermal_Impl(const openstudio::detail::WorkspaceObject_Impl& other,
                                                                                                   Model_Impl* model,
                                                                                                   bool keepHandle)
    : StraightComponent_Impl(other,model,keepHandle)
  {
    OS_ASSERT(other.iddObject().type() == SolarCollectorFlatPlatePhotovoltaicThermal::iddObjectType());
  }

  SolarCollectorFlatPlatePhotovoltaicThermal_Impl::SolarCollectorFlatPlatePhotovoltaicThermal_Impl(const SolarCollectorFlatPlatePhotovoltaicThermal_Impl& other,
                                                                                                   Model_Impl* model,
                                                                                                   bool keepHandle)
    : StraightComponent_Impl(other,model,keepHandle)
  {}

  ModelObject SolarCollectorFlatPlatePhotovoltaicThermal_Impl::clone(Model model) const
  {

    SolarCollectorFlatPlatePhotovoltaicThermal result = StraightComponent_Impl::clone(model).cast<SolarCollectorFlatPlatePhotovoltaicThermal>();
    SolarCollectorPerformancePhotovoltaicThermalSimple newPerformance = this->solarCollectorPerformance().clone(model).cast<SolarCollectorPerformancePhotovoltaicThermalSimple>();
    result.setPointer(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::PhotovoltaicThermalModelPerformanceName, newPerformance.handle());

    boost::optional<GeneratorPhotovoltaic> pv = this->generatorPhotovoltaic();
    if (pv){
      GeneratorPhotovoltaic newPV = pv->clone(model).cast<GeneratorPhotovoltaic>();
      result.setGeneratorPhotovoltaic(newPV);
    }

    // do not want to point to any surface after cloning
    result.resetSurface();

    return result;
  }

  std::vector<IdfObject> SolarCollectorFlatPlatePhotovoltaicThermal_Impl::remove()
  {
    // DLM: will remove performance object due to parent/child relationship
    return StraightComponent_Impl::remove();
  }

  const std::vector<std::string>& SolarCollectorFlatPlatePhotovoltaicThermal_Impl::outputVariableNames() const
  {
    static std::vector<std::string> result;
    if (result.empty()){
      result.push_back("Generator Produced Thermal Rate");
      result.push_back("Generator Produced Thermal Energy");
      result.push_back("Generator PVT Fluid Bypass Status");
      result.push_back("Generator PVT Fluid Inlet Temperature");
      result.push_back("Generator PVT Fluid Outlet Temperature");
      result.push_back("Generator PVT Fluid Mass Flow Rate");
    }
    return result;
  }

  IddObjectType SolarCollectorFlatPlatePhotovoltaicThermal_Impl::iddObjectType() const {
    return SolarCollectorFlatPlatePhotovoltaicThermal::iddObjectType();
  }

  std::vector<ModelObject> SolarCollectorFlatPlatePhotovoltaicThermal_Impl::children() const
  {
    std::vector<ModelObject> result;

    SolarCollectorPerformancePhotovoltaicThermalSimple solarCollectorPerformance = this->solarCollectorPerformance();
    result.push_back(solarCollectorPerformance);

    return result;
  }

  unsigned SolarCollectorFlatPlatePhotovoltaicThermal_Impl::inletPort()
  {
    return OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::InletNodeName;
  }

  unsigned SolarCollectorFlatPlatePhotovoltaicThermal_Impl::outletPort()
  {
    return OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::OutletNodeName;
  }

  bool SolarCollectorFlatPlatePhotovoltaicThermal_Impl::addToNode(Node & node)
  {
    if (boost::optional<PlantLoop> plantLoop = node.plantLoop())
    {
      if (plantLoop->supplyComponent(node.handle()))
      {
        if (StraightComponent_Impl::addToNode(node))
        {
          return true;
        }
      }
    } else if (boost::optional<AirLoopHVAC> airLoopHVAC = node.airLoopHVAC())
    {
      boost::optional<AirLoopHVACOutdoorAirSystem> oa = airLoopHVAC->airLoopHVACOutdoorAirSystem();
      if (oa)
      {
        if (oa->oaComponent(node.handle()))
        {
          if (StraightComponent_Impl::addToNode(node))
          {
            return true;
          }
        }
      }
    }


    return false;
  }

  SolarCollectorPerformancePhotovoltaicThermalSimple SolarCollectorFlatPlatePhotovoltaicThermal_Impl::solarCollectorPerformance() const {
    boost::optional<SolarCollectorPerformancePhotovoltaicThermalSimple> value = getObject<ModelObject>().getModelObjectTarget<SolarCollectorPerformancePhotovoltaicThermalSimple>(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::PhotovoltaicThermalModelPerformanceName);
    if (!value) {
      // DLM: could default construct one here?
      LOG_AND_THROW(briefDescription() << " does not have an Solar Collector Performance attached.");
    }
    return value.get();
  
  }

  boost::optional<PlanarSurface> SolarCollectorFlatPlatePhotovoltaicThermal_Impl::surface() const {
    return getObject<ModelObject>().getModelObjectTarget<PlanarSurface>(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::SurfaceName);
  }

  boost::optional<GeneratorPhotovoltaic> SolarCollectorFlatPlatePhotovoltaicThermal_Impl::generatorPhotovoltaic() const
  {
    return getObject<ModelObject>().getModelObjectTarget<GeneratorPhotovoltaic>(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::PhotovoltaicName);
  }

  boost::optional<double> SolarCollectorFlatPlatePhotovoltaicThermal_Impl::designFlowRate() const {
    return getDouble(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::DesignFlowRate,true);
  }

  bool SolarCollectorFlatPlatePhotovoltaicThermal_Impl::isDesignFlowRateAutosized() const {
    bool result = false;
    boost::optional<std::string> value = getString(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::DesignFlowRate, true);
    if (value) {
      result = openstudio::istringEqual(value.get(), "autosize");
    }
    return result;
  }

  bool SolarCollectorFlatPlatePhotovoltaicThermal_Impl::setSolarCollectorPerformance(const SolarCollectorPerformancePhotovoltaicThermalSimple& performance) {
    bool result(false);
    SolarCollectorPerformancePhotovoltaicThermalSimple current = this->solarCollectorPerformance();
    if (current.handle() == performance.handle()){
      return true; // no-op
    }
    ModelObject clone = performance.clone(this->model());
    result = setSolarCollectorPerformanceNoClone(clone.cast<SolarCollectorPerformancePhotovoltaicThermalSimple>());
    if (result){
      current.remove();
    } else{
      result = setSolarCollectorPerformanceNoClone(current);
      OS_ASSERT(result);
      return false;
    }
    return result;
  }

  void SolarCollectorFlatPlatePhotovoltaicThermal_Impl::resetSolarCollectorPerformance() {
    boost::optional<SolarCollectorPerformancePhotovoltaicThermalSimple> oldPerformance = getObject<ModelObject>().getModelObjectTarget<SolarCollectorPerformancePhotovoltaicThermalSimple>(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::PhotovoltaicThermalModelPerformanceName);
    if (oldPerformance){
      oldPerformance->remove();
    }

    SolarCollectorPerformancePhotovoltaicThermalSimple performance(this->model());
    bool ok = setSolarCollectorPerformanceNoClone(performance);
    OS_ASSERT(ok);
  }

  bool SolarCollectorFlatPlatePhotovoltaicThermal_Impl::setSurface(const PlanarSurface& surface) {
    bool result(false);

    // DLM: check for existing solar collectors or photovoltaic generators?

    if (surface.optionalCast<Surface>()){
      result = setPointer(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::SurfaceName, surface.handle());
    } else if (surface.optionalCast<ShadingSurface>()){
      result = setPointer(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::SurfaceName, surface.handle());
    }

    return result;
  }

  void SolarCollectorFlatPlatePhotovoltaicThermal_Impl::resetSurface() {
    bool result = setString(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::SurfaceName, "");
    OS_ASSERT(result);
  }

  bool SolarCollectorFlatPlatePhotovoltaicThermal_Impl::setGeneratorPhotovoltaic(const GeneratorPhotovoltaic& generatorPhotovoltaic)
  {
    // DLM: should we check that these reference the same surface?
    return setPointer(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::PhotovoltaicName, generatorPhotovoltaic.handle());
  }

  void SolarCollectorFlatPlatePhotovoltaicThermal_Impl::resetGeneratorPhotovoltaic()
  {
    bool result = setString(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::PhotovoltaicName, "");
    OS_ASSERT(result);
  }

  bool SolarCollectorFlatPlatePhotovoltaicThermal_Impl::setDesignFlowRate(double designFlowRate) {
    return setDouble(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::DesignFlowRate, designFlowRate);
  }

  void SolarCollectorFlatPlatePhotovoltaicThermal_Impl::resetDesignFlowRate() {
    bool result = setString(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::DesignFlowRate, "");
    OS_ASSERT(result);
  }

  void SolarCollectorFlatPlatePhotovoltaicThermal_Impl::autosizeDesignFlowRate() {
    bool result = setString(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::DesignFlowRate, "autosize");
    OS_ASSERT(result);
  }

  bool SolarCollectorFlatPlatePhotovoltaicThermal_Impl::setSolarCollectorPerformanceNoClone(const SolarCollectorPerformancePhotovoltaicThermalSimple& performance) {
    return setPointer(OS_SolarCollector_FlatPlate_PhotovoltaicThermalFields::PhotovoltaicThermalModelPerformanceName, performance.handle());
  }

} // detail

SolarCollectorFlatPlatePhotovoltaicThermal::SolarCollectorFlatPlatePhotovoltaicThermal(const Model& model)
  : StraightComponent(SolarCollectorFlatPlatePhotovoltaicThermal::iddObjectType(),model)
{
  OS_ASSERT(getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>());

  SolarCollectorPerformancePhotovoltaicThermalSimple performance(model);

  bool ok = true;
  ok = getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->setSolarCollectorPerformanceNoClone(performance);
  OS_ASSERT(ok);
}

IddObjectType SolarCollectorFlatPlatePhotovoltaicThermal::iddObjectType() {
  return IddObjectType(IddObjectType::OS_SolarCollector_FlatPlate_PhotovoltaicThermal);
}

SolarCollectorPerformancePhotovoltaicThermalSimple SolarCollectorFlatPlatePhotovoltaicThermal::solarCollectorPerformance() const {
  return getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->solarCollectorPerformance();
}

boost::optional<PlanarSurface> SolarCollectorFlatPlatePhotovoltaicThermal::surface() const {
  return getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->surface();
}

boost::optional<GeneratorPhotovoltaic> SolarCollectorFlatPlatePhotovoltaicThermal::generatorPhotovoltaic() const{
  return getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->generatorPhotovoltaic();
}

boost::optional<double> SolarCollectorFlatPlatePhotovoltaicThermal::designFlowRate() const {
  return getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->designFlowRate();
}

bool SolarCollectorFlatPlatePhotovoltaicThermal::isDesignFlowRateAutosized() const {
  return getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->isDesignFlowRateAutosized();
}

bool SolarCollectorFlatPlatePhotovoltaicThermal::setSurface(const PlanarSurface& surface) {
  return getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->setSurface(surface);
}

void SolarCollectorFlatPlatePhotovoltaicThermal::resetSurface() {
  return getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->resetSurface();
}

bool SolarCollectorFlatPlatePhotovoltaicThermal::setGeneratorPhotovoltaic(const GeneratorPhotovoltaic& generatorPhotovoltaic){
  return getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->setGeneratorPhotovoltaic(generatorPhotovoltaic);
}

void SolarCollectorFlatPlatePhotovoltaicThermal::resetGeneratorPhotovoltaic(){
  getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->resetGeneratorPhotovoltaic();
}

bool SolarCollectorFlatPlatePhotovoltaicThermal::setSolarCollectorPerformance(const SolarCollectorPerformancePhotovoltaicThermalSimple& performance) {
  return getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->setSolarCollectorPerformance(performance);
}

void SolarCollectorFlatPlatePhotovoltaicThermal::resetSolarCollectorPerformance() {
  getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->resetSolarCollectorPerformance();
}

bool SolarCollectorFlatPlatePhotovoltaicThermal::setDesignFlowRate(double designFlowRate) {
  return getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->setDesignFlowRate(designFlowRate);
}

void SolarCollectorFlatPlatePhotovoltaicThermal::resetDesignFlowRate() {
  getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->resetDesignFlowRate();
}

void SolarCollectorFlatPlatePhotovoltaicThermal::autosizeDesignFlowRate() {
  getImpl<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl>()->autosizeDesignFlowRate();
}

/// @cond
SolarCollectorFlatPlatePhotovoltaicThermal::SolarCollectorFlatPlatePhotovoltaicThermal(std::shared_ptr<detail::SolarCollectorFlatPlatePhotovoltaicThermal_Impl> impl)
  : StraightComponent(impl)
{}
/// @endcond

} // model
} // openstudio
