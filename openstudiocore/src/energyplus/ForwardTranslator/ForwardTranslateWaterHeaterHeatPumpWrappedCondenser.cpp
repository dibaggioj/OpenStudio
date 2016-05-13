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

#include "../ForwardTranslator.hpp"
#include "../../model/WaterHeaterHeatPumpWrappedCondenser.hpp"
#include "../../model/WaterHeaterHeatPumpWrappedCondenser_Impl.hpp"
#include "../../model/WaterHeaterMixed.hpp"
#include "../../model/WaterHeaterMixed_Impl.hpp"
#include "../../model/WaterHeaterStratified.hpp"
#include "../../model/WaterHeaterStratified_Impl.hpp"
#include "../../model/CoilWaterHeatingAirToWaterHeatPump.hpp"
#include "../../model/CoilWaterHeatingAirToWaterHeatPump_Impl.hpp"
#include "../../model/Model.hpp"
#include "../../model/Schedule.hpp"
#include "../../model/Schedule_Impl.hpp"
#include "../../model/Curve.hpp"
#include "../../model/Curve_Impl.hpp"
#include "../../model/ThermalZone.hpp"
#include "../../model/ThermalZone_Impl.hpp"
#include "../../model/Node.hpp"
#include "../../model/Node_Impl.hpp"
#include "../../model/FanOnOff.hpp"
#include "../../model/FanOnOff_Impl.hpp"
#include "../../utilities/core/Assert.hpp"
#include <utilities/idd/WaterHeater_HeatPump_WrappedCondenser_FieldEnums.hxx>
#include <utilities/idd/Coil_WaterHeating_AirToWaterHeatPump_Pumped_FieldEnums.hxx>
#include <utilities/idd/WaterHeater_Mixed_FieldEnums.hxx>
#include <utilities/idd/WaterHeater_Stratified_FieldEnums.hxx>
#include <utilities/idd/Fan_OnOff_FieldEnums.hxx>
#include <utilities/idd/IddEnums.hxx>

using namespace openstudio::model;
using namespace std;

namespace openstudio {

namespace energyplus {

boost::optional<IdfObject> ForwardTranslator::translateWaterHeaterHeatPumpWrappedCondenser( 
    WaterHeaterHeatPumpWrappedCondenser & modelObject)
{
  IdfObject idfObject(IddObjectType::WaterHeater_HeatPump_WrappedCondenser);
  m_idfObjects.push_back(idfObject);

  // Name
  if( auto s = modelObject.name() ) {
    idfObject.setName(*s);
  }

  auto tank = modelObject.tank();
  auto stratifiedTank = tank.optionalCast<model::WaterHeaterStratified>();

  if( auto height = modelObject.controlSensor1HeightInStratifiedTank() ) {
    idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::ControlSensor1HeightInStratifiedTank,height.get());
  } else {
    if( stratifiedTank ) {
      auto heater1Height = stratifiedTank->heater1Height();
      idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::ControlSensor1HeightInStratifiedTank,heater1Height);
    }
  }

  {
    auto value = modelObject.controlSensor1Weight();
    idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::ControlSensor1Weight,value);
  }

  if( auto height = modelObject.controlSensor2HeightInStratifiedTank() ) {
    idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::ControlSensor2HeightInStratifiedTank,height.get());
  } else {
    if( stratifiedTank ) {
      auto heater2Height = stratifiedTank->heater2Height();
      idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::ControlSensor2HeightInStratifiedTank,heater2Height);
    }
  }

  if( auto schedule = modelObject.availabilitySchedule() ) {
    if( auto _schedule = translateAndMapModelObject(schedule.get()) ) {
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::AvailabilityScheduleName,_schedule->nameString());
    }
  }

  {
    auto schedule = modelObject.compressorSetpointTemperatureSchedule();
    if( auto _schedule = translateAndMapModelObject(schedule) ) {
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::CompressorSetpointTemperatureScheduleName,_schedule->nameString());
    }
  }

  {
    auto value = modelObject.deadBandTemperatureDifference();
    idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::DeadBandTemperatureDifference,value);
  }

  {
    auto value = modelObject.condenserBottomLocation();
    idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::CondenserBottomLocation,value);
  }

  {
    auto value = modelObject.condenserTopLocation();
    idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::CondenserTopLocation,value);
  }

  {
    if( modelObject.isEvaporatorAirFlowRateAutocalculated() ) {
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::EvaporatorAirFlowRate,"Autocalculate");
    } else if( auto value = modelObject.evaporatorAirFlowRate() ) {
      idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::EvaporatorAirFlowRate,value.get());
    }
  }

  std::string airInletNodeName;
  std::string airOutletNodeName;
  std::string outdoorAirNodeName;
  std::string exhaustAirNodeName;
  std::string inletAirZoneName;
  std::string inletAirMixerNodeName;
  std::string outletAirSplitterNodeName;
  std::string fanInletNodeName;
  std::string fanOutletNodeName;
  std::string evapInletNodeName;
  std::string evapOutletNodeName;

  auto inletAirConfiguration = modelObject.inletAirConfiguration();
  idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirConfiguration,inletAirConfiguration);

  if( istringEqual(modelObject.fanPlacement(),"DrawThrough") ) {
    idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::FanPlacement,"DrawThrough");

    if( istringEqual(inletAirConfiguration,"ZoneAirOnly") ) {
      if( auto thermalZone = modelObject.thermalZone() ) {
        auto inletNode = modelObject.inletNode();
        OS_ASSERT(inletNode);
        airInletNodeName = inletNode->name().get();

        auto outletNode = modelObject.outletNode();
        OS_ASSERT(outletNode);
        airOutletNodeName = outletNode->name().get();
        
        inletAirZoneName = thermalZone->name().get();
        fanInletNodeName = modelObject.name().get() + " Evap Outlet - Fan Inlet";
        fanOutletNodeName = airOutletNodeName;
        evapInletNodeName = airInletNodeName;
        evapOutletNodeName = fanInletNodeName;
      }
    } else if( istringEqual(inletAirConfiguration,"ZoneAndOutdoorAir") ) {
      if( auto thermalZone = modelObject.thermalZone() ) {
        auto inletNode = modelObject.inletNode();
        OS_ASSERT(inletNode);
        airInletNodeName = inletNode->name().get();

        auto outletNode = modelObject.outletNode();
        OS_ASSERT(outletNode);
        airOutletNodeName = outletNode->name().get();

        outdoorAirNodeName = modelObject.name().get() + " Outdoor Air";
        exhaustAirNodeName = modelObject.name().get() + " Exhaust Air";
        inletAirZoneName = thermalZone->name().get();
        inletAirMixerNodeName = modelObject.name().get() + " Mixer Outlet - Evap Inlet";
        outletAirSplitterNodeName = modelObject.name().get() + " Fan Outlet - Splitter Inlet";
        fanInletNodeName = modelObject.name().get() + " Evap Outlet - Fan Inlet";
        fanOutletNodeName = outletAirSplitterNodeName;
        evapInletNodeName = inletAirMixerNodeName;
        evapOutletNodeName = fanInletNodeName;
      }
    } else if( istringEqual(inletAirConfiguration,"OutdoorAirOnly") ) {
      outdoorAirNodeName = modelObject.name().get() + " Outdoor Air";
      exhaustAirNodeName = modelObject.name().get() + " Exhaust Air";
      fanInletNodeName = modelObject.name().get() + " Evap Outlet - Fan Inlet";
      fanOutletNodeName = exhaustAirNodeName;
      evapInletNodeName = outdoorAirNodeName;
      evapOutletNodeName = fanInletNodeName;
    } else if( istringEqual(inletAirConfiguration,"Schedule") ) {
      airInletNodeName = modelObject.name().get() + " Inlet";
      airOutletNodeName = modelObject.name().get() + " Outlet";
      fanInletNodeName = modelObject.name().get() + " Evap Outlet - Fan Inlet";
      fanOutletNodeName = airOutletNodeName;
      evapInletNodeName = airInletNodeName;
      evapOutletNodeName = fanInletNodeName;
    }

  } else { // BlowThrough
    idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::FanPlacement,"BlowThrough");

    if( istringEqual(inletAirConfiguration,"ZoneAirOnly") ) {
      if( auto thermalZone = modelObject.thermalZone() ) {
        auto inletNode = modelObject.inletNode();
        OS_ASSERT(inletNode);
        airInletNodeName = inletNode->name().get();

        auto outletNode = modelObject.outletNode();
        OS_ASSERT(outletNode);
        airOutletNodeName = outletNode->name().get();
        
        inletAirZoneName = thermalZone->name().get();
        fanInletNodeName = airInletNodeName;
        fanOutletNodeName = modelObject.name().get() + " Fan Outlet - Evap Inlet";
        evapInletNodeName = fanOutletNodeName;
        evapOutletNodeName = airOutletNodeName;
      }
    } else if( istringEqual(inletAirConfiguration,"ZoneAndOutdoorAir") ) {
      if( auto thermalZone = modelObject.thermalZone() ) {
        auto inletNode = modelObject.inletNode();
        OS_ASSERT(inletNode);
        airInletNodeName = inletNode->name().get();

        auto outletNode = modelObject.outletNode();
        OS_ASSERT(outletNode);
        airOutletNodeName = outletNode->name().get();

        outdoorAirNodeName = modelObject.name().get() + " Outdoor Air";
        exhaustAirNodeName = modelObject.name().get() + " Exhaust Air";
        inletAirZoneName = thermalZone->name().get();
        inletAirMixerNodeName = modelObject.name().get() + " Mixer Outlet - Fan Inlet";
        outletAirSplitterNodeName = modelObject.name().get() + " Evap Outlet - Splitter Inlet";
        fanInletNodeName = inletAirMixerNodeName;
        fanOutletNodeName = modelObject.name().get() + " Fan Outlet - Evap Inlet";
        evapInletNodeName = fanOutletNodeName;
        evapOutletNodeName = outletAirSplitterNodeName;
      }
    } else if( istringEqual(inletAirConfiguration,"OutdoorAirOnly") ) {
      outdoorAirNodeName = modelObject.name().get() + " Outdoor Air";
      exhaustAirNodeName = modelObject.name().get() + " Exhaust Air";
      fanInletNodeName = outdoorAirNodeName;
      fanOutletNodeName = modelObject.name().get() + " Fan Outlet - Evap Inlet";
      evapInletNodeName = fanOutletNodeName;
      evapOutletNodeName = exhaustAirNodeName;
    } else if( istringEqual(inletAirConfiguration,"Schedule") ) {
      airInletNodeName = modelObject.name().get() + " Inlet";
      airOutletNodeName = modelObject.name().get() + " Outlet";
      fanInletNodeName = airInletNodeName;
      fanOutletNodeName = modelObject.name().get() + " Fan Outlet - Evap Inlet";
      evapInletNodeName = fanOutletNodeName;
      evapOutletNodeName = airOutletNodeName;
    }

  }

  idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::AirInletNodeName,airInletNodeName);
  idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::AirOutletNodeName,airOutletNodeName);
  idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::OutdoorAirNodeName,outdoorAirNodeName);
  idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::ExhaustAirNodeName,exhaustAirNodeName);
  idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirZoneName,inletAirZoneName);
  idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirMixerNodeName,inletAirMixerNodeName);
  idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::OutletAirSplitterNodeName,outletAirSplitterNodeName);

  std::string condOutletTankInletNodeName =
    modelObject.name().get() + " Condenser Outlet - Tank Inlet";

  std::string tankOutletCondInletNodeName = 
    modelObject.name().get() + " Tank Outlet - Condenser Inlet";

  if( auto schedule = modelObject.inletAirTemperatureSchedule() ) {
    if( auto _schedule = translateAndMapModelObject(schedule.get()) ) {
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirTemperatureScheduleName,_schedule->nameString());
    }
  }

  if( auto schedule = modelObject.inletAirHumiditySchedule() ) {
    if( auto _schedule = translateAndMapModelObject(schedule.get()) ) {
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirHumidityScheduleName,_schedule->nameString());
    }
  }

  {
    auto mo = modelObject.tank();
    if( auto idf = translateAndMapModelObject(mo) ) {
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::TankName,idf->name().get());
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::TankObjectType,idf->iddObject().name());
      if( mo.iddObjectType() == model::WaterHeaterMixed::iddObjectType() ) {
        idf->setString(WaterHeater_MixedFields::SourceSideOutletNodeName,tankOutletCondInletNodeName);
        idf->setString(WaterHeater_MixedFields::SourceSideInletNodeName,condOutletTankInletNodeName);
        auto waterHeaterMixed = mo.cast<model::WaterHeaterMixed>();
        if( auto node = waterHeaterMixed.supplyInletModelObject() ) {
          idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::TankUseSideInletNodeName,node->name().get());
        }
        if( auto node = waterHeaterMixed.supplyOutletModelObject() ) {
          idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::TankUseSideOutletNodeName,node->name().get());
        }
      } else if( mo.iddObjectType() == model::WaterHeaterStratified::iddObjectType() ) {
        idf->setString(WaterHeater_StratifiedFields::SourceSideOutletNodeName,tankOutletCondInletNodeName);
        idf->setString(WaterHeater_StratifiedFields::SourceSideInletNodeName,condOutletTankInletNodeName);
        auto waterHeaterStratified = mo.cast<model::WaterHeaterStratified>();
        if( auto node = waterHeaterStratified.supplyInletModelObject() ) {
          idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::TankUseSideInletNodeName,node->name().get());
        }
        if( auto node = waterHeaterStratified.supplyOutletModelObject() ) {
          idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::TankUseSideOutletNodeName,node->name().get());
        }
      } else {
        LOG(Error, modelObject.briefDescription() << " is attached to an unsupported type of tank: " << mo.briefDescription() );
      }
    }
  }

  {
    auto mo = modelObject.dXCoil();
    if( auto idf = translateAndMapModelObject(mo) ) {
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::DXCoilName,idf->name().get());
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::DXCoilObjectType,idf->iddObject().name());
      if( mo.iddObjectType() == model::CoilWaterHeatingAirToWaterHeatPump::iddObjectType() ) {
        idf->setString(Coil_WaterHeating_AirToWaterHeatPump_PumpedFields::CondenserWaterInletNodeName,tankOutletCondInletNodeName);
        idf->setString(Coil_WaterHeating_AirToWaterHeatPump_PumpedFields::CondenserWaterOutletNodeName,condOutletTankInletNodeName);
        idf->setString(Coil_WaterHeating_AirToWaterHeatPump_PumpedFields::EvaporatorAirInletNodeName,evapInletNodeName);
        idf->setString(Coil_WaterHeating_AirToWaterHeatPump_PumpedFields::EvaporatorAirOutletNodeName,evapOutletNodeName);
      } else {
        LOG(Error, modelObject.briefDescription() << " is attached to an unsupported type of coil: " << mo.briefDescription() );
      }
    }
  }

  {
    auto value = modelObject.minimumInletAirTemperatureforCompressorOperation();
    idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::MinimumInletAirTemperatureforCompressorOperation,value);
  }

  {
    auto value = modelObject.maximumInletAirTemperatureforCompressorOperation();
    idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::MaximumInletAirTemperatureforCompressorOperation,value);
  }

  {
    auto value = modelObject.compressorLocation();
    idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::CompressorLocation,value);
  }

  if( auto schedule = modelObject.compressorAmbientTemperatureSchedule() ) {
    if( auto _schedule = translateAndMapModelObject(schedule.get()) ) {
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::CompressorAmbientTemperatureScheduleName,_schedule->nameString());
    }
  }

  {
    auto mo = modelObject.fan();
    if( auto idf = translateAndMapModelObject(mo) ) {
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::FanName,idf->name().get());
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::FanObjectType,idf->iddObject().name());
      if( mo.iddObjectType() == model::FanOnOff::iddObjectType() ) {
        idf->setString(Fan_OnOffFields::AirInletNodeName,fanInletNodeName);
        idf->setString(Fan_OnOffFields::AirOutletNodeName,fanOutletNodeName);
      } else {
        LOG(Error, modelObject.briefDescription() << " is attached to an unsupported type of fan: " << mo.briefDescription() );
      }
    }
  }

  {
    auto value = modelObject.onCycleParasiticElectricLoad();
    idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::OnCycleParasiticElectricLoad,value);
  }

  {
    auto value = modelObject.offCycleParasiticElectricLoad();
    idfObject.setDouble(WaterHeater_HeatPump_WrappedCondenserFields::OffCycleParasiticElectricLoad,value);
  }

  {
    auto value = modelObject.parasiticHeatRejectionLocation();
    idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::ParasiticHeatRejectionLocation,value);
  }

  if( auto schedule = modelObject.inletAirMixerSchedule() ) {
    if( auto _schedule = translateAndMapModelObject(schedule.get()) ) {
      idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirMixerScheduleName,_schedule->nameString());
    }
  }

  {
    auto value = modelObject.tankElementControlLogic();
    idfObject.setString(WaterHeater_HeatPump_WrappedCondenserFields::TankElementControlLogic,value);
  }

  return idfObject;
}

} // energyplus
} // openstudio