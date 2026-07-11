/**
 * Nissan Z31 300ZX (VG30E/ET) plug-n-play unit
 * mega-uaefi module + mcu100-f7, see docs/schematic-notes.md for the pin mapping extraction
 */

#include "pch.h"
#include "defaults.h"
#include "mega-uaefi.h"
#include "hellen_meta.h"
#include "board_overrides.h"
#include "hellen_mm100_meta.h"

#include "hellen_leds_100.cpp"

static void customBoardConfigOverrides() {
	// F7 module stack, same as super-uaefi: not setMegaUaefiBoardConfigOverrides which
	// carries F4-specific SPI1/CAN2 pin workarounds
	setHellenMegaEnPin();
	setHellenVbatt();

	hellenMegaSdWithAccelerometer();

	engineConfiguration->vrThreshold[0].pin = Gpio::MM100_OUT_PWM6;

	setHellenCan();

	setDefaultHellenAtPullUps();
}

// board-specific configuration setup
static void customBoardDefaultConfiguration() {
	setUaefiBoardDefaultConfiguration();

	// OEM pins 104/105 are cross-wired to module channels INJ5/INJ4, see schematic-notes.md
	engineConfiguration->injectionPins[3] = Gpio::MM100_INJ5; // 104 cylinder 4
	engineConfiguration->injectionPins[4] = Gpio::MM100_INJ4; // 105 cylinder 5

	// single channel into the OEM external power transistor, distributor engine
	engineConfiguration->ignitionPins[0] = Gpio::MM100_IGN1; // 5 OUT_IGN

	engineConfiguration->mainRelayPin = Gpio::MM100_IGN7; // 6 LS5_HOT Main Relay
	engineConfiguration->fuelPumpPin = Gpio::MM100_OUT_PWM2; // 20 LS4 Fuel Pump Relay
	engineConfiguration->idle.solenoidPin = Gpio::MM100_INJ8; // 2 LS2 Idle Air Valve

	// optical CAS in distributor: crank on OEM pin 17, cam on OEM pin 8
	engineConfiguration->triggerInputPins[0] = Gpio::MM100_IN_D2;
	engineConfiguration->camInputs[0] = Gpio::MM100_IN_D1;

	engineConfiguration->vehicleSpeedSensorInputPin = Gpio::MM100_IN_D3; // 29 IN_VSS
	engineConfiguration->clutchDownPin = Gpio::MM100_IN_CRANK; // 10 IN_CLUTCH

	engineConfiguration->tps1_1AdcChannel = MM100_IN_TPS_ANALOG; // J1-32
	engineConfiguration->mafAdcChannel = MM100_IN_O2S2_ANALOG; // 31 IN_MAF hot-wire
	engineConfiguration->map.sensor.hwChannel = MM100_IN_MAP1_ANALOG; // J1-31
	engineConfiguration->clt.adcChannel = MM100_IN_CLT_ANALOG; // 23
	engineConfiguration->iat.adcChannel = MM100_IN_IAT_ANALOG; // J1-24

	engineConfiguration->enableSoftwareKnock = true; // 21 IN_KNOCK

	engineConfiguration->displayLogicLevelsInEngineSniffer = true;
	engineConfiguration->isSdCardEnabled = true;

	hellenWbo();
}

static Gpio OUTPUTS[] = {
	Gpio::MM100_INJ1, // 101 INJ_1
	Gpio::MM100_INJ2, // 102 INJ_2
	Gpio::MM100_INJ3, // 103 INJ_3
	Gpio::MM100_INJ5, // 104 INJ_4
	Gpio::MM100_INJ4, // 105 INJ_5
	Gpio::MM100_INJ6, // 106 INJ_6
	Gpio::MM100_INJ8, // 2 LS2 Idle Air Valve
	Gpio::MM100_OUT_PWM2, // 20 LS4 Fuel Pump Relay
	Gpio::MM100_IGN7, // 6 LS5_HOT Main Relay
	Gpio::MM100_INJ7, // J1-8 LS1
	Gpio::MM100_OUT_PWM1, // J1-7 LS3
	Gpio::MM100_IGN1, // 5 OUT_IGN power transistor, logic level
	Gpio::MM100_IGN3, // J1-9 IGN_AUX_3
	Gpio::MM100_IGN4, // J1-17 IGN_AUX_4
	Gpio::MM100_IGN5, // J1-25 IGN_AUX_5
};

int getBoardMetaOutputsCount() {
    return efi::size(OUTPUTS);
}

int getBoardMetaLowSideOutputsCount() {
    // the last 4 outputs are logic-level ignition channels
    return getBoardMetaOutputsCount() - 4;
}

Gpio* getBoardMetaOutputs() {
    return OUTPUTS;
}

static void customBoardInitHardware() {
	setupHellenSharedInputs();
}

void setup_custom_board_overrides() {
	custom_board_InitHardware = customBoardInitHardware;
	custom_board_DefaultConfiguration = customBoardDefaultConfiguration;
	custom_board_ConfigOverrides = customBoardConfigOverrides;
}
