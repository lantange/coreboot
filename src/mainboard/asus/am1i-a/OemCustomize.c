/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2012 Advanced Micro Devices, Inc.
 * Copyright (C) 2018 Gergely Kiss <mail.gery@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "AGESA.h"
#include <PlatformMemoryConfiguration.h>

#include <northbridge/amd/agesa/state_machine.h>

static const PCIe_PORT_DESCRIPTOR PortList[] = {
	{
		0,
		PCIE_ENGINE_DATA_INITIALIZER(PciePortEngine, 3, 3),
		PCIE_PORT_DATA_INITIALIZER_V2(PortEnabled, ChannelTypeExt6db, 2, 5,
				HotplugDisabled,
				PcieGenMaxSupported,
				PcieGenMaxSupported,
				AspmDisabled, 0x01, 0)
	},
	/* Initialize Port descriptor (PCIe port, Lanes 1, PCI Device Number 2, ...) */
	{
		0,
		PCIE_ENGINE_DATA_INITIALIZER(PciePortEngine, 2, 2),
		PCIE_PORT_DATA_INITIALIZER_V2(PortEnabled, ChannelTypeExt6db, 2, 4,
				HotplugDisabled,
				PcieGenMaxSupported,
				PcieGenMaxSupported,
				AspmDisabled, 0x02, 0)
	},
	/* Initialize Port descriptor (PCIe port, Lanes 2, PCI Device Number 2, ...) */
	{
		0,
		PCIE_ENGINE_DATA_INITIALIZER(PciePortEngine, 1, 1),
		PCIE_PORT_DATA_INITIALIZER_V2(PortEnabled, ChannelTypeExt6db, 2, 3,
				HotplugDisabled,
				PcieGenMaxSupported,
				PcieGenMaxSupported,
				AspmDisabled, 0x03, 0)
	},
	/* Initialize Port descriptor (PCIe port, Lanes 3, PCI Device Number 2, ...) */
	{
		0,
		PCIE_ENGINE_DATA_INITIALIZER(PciePortEngine, 0, 0),
		PCIE_PORT_DATA_INITIALIZER_V2(PortEnabled, ChannelTypeExt6db, 2, 2,
				HotplugDisabled,
				PcieGenMaxSupported,
				PcieGenMaxSupported,
				AspmDisabled, 0x04, 0)
	},
	/* Initialize Port descriptor (PCIe port, Lanes 4-7, PCI Device Number 4, ...) */
	{
		DESCRIPTOR_TERMINATE_LIST,
		PCIE_ENGINE_DATA_INITIALIZER(PciePortEngine, 4, 7),
		PCIE_PORT_DATA_INITIALIZER_V2(PortEnabled, ChannelTypeExt6db, 2, 1,
				HotplugDisabled,
				PcieGenMaxSupported,
				PcieGenMaxSupported,
				AspmDisabled, 0x05, 0)
	}
};

static const PCIe_DDI_DESCRIPTOR DdiList[] = {
	/* DP0 to HDMI */
	{
		0,
		PCIE_ENGINE_DATA_INITIALIZER(PcieDdiEngine, 8, 11),
		PCIE_DDI_DATA_INITIALIZER(ConnectorTypeHDMI, Aux1, Hdp1)
	},
	/* DP1 to DVI-D */
	{
		0,
		PCIE_ENGINE_DATA_INITIALIZER(PcieDdiEngine, 12, 15),
		PCIE_DDI_DATA_INITIALIZER(ConnectorTypeSingleLinkDVI, Aux2, Hdp2)
	},
	/* DP2 to VGA */
	{
		DESCRIPTOR_TERMINATE_LIST,
		PCIE_ENGINE_DATA_INITIALIZER(PcieDdiEngine, 16, 19),
		PCIE_DDI_DATA_INITIALIZER(ConnectorTypeCrt, Aux3, Hdp3)
	},
};

static const PCIe_COMPLEX_DESCRIPTOR PcieComplex = {
	.Flags        = DESCRIPTOR_TERMINATE_LIST,
	.SocketId     = 0,
	.PciePortList = PortList,
	.DdiLinkList  = DdiList
};


void board_BeforeInitReset(struct sysinfo *cb, AMD_RESET_PARAMS *Reset)
{
	FCH_RESET_INTERFACE *FchReset = &Reset->FchInterface;

	FchReset->Xhci0Enable = IS_ENABLED(CONFIG_HUDSON_XHCI_ENABLE);
	FchReset->Xhci1Enable = IS_ENABLED(CONFIG_HUDSON_XHCI_ENABLE);

	FchReset->SataEnable = 1;
	FchReset->IdeEnable = 0;
}

void board_BeforeInitEarly(struct sysinfo *cb, AMD_EARLY_PARAMS *InitEarly)
{
	InitEarly->GnbConfig.PcieComplexList = &PcieComplex;
}

/*----------------------------------------------------------------------------------------
 *                        CUSTOMER OVERIDES MEMORY TABLE
 *----------------------------------------------------------------------------------------
 */

/*
 *  Platform Specific Overriding Table allows IBV/OEM to pass in platform information to AGESA
 *  (e.g. MemClk routing, the number of DIMM slots per channel,...). If PlatformSpecificTable
 *  is populated, AGESA will base its settings on the data from the table. Otherwise, it will
 *  use its default conservative settings.
 */
static CONST PSO_ENTRY ROMDATA PlatformMemoryTable[] = {
	#define SEED_A 0x12
	HW_RXEN_SEED(
		ANY_SOCKET, CHANNEL_A, ALL_DIMMS,
		SEED_A, SEED_A, SEED_A, SEED_A, SEED_A, SEED_A, SEED_A, SEED_A,
		SEED_A),

  NUMBER_OF_DIMMS_SUPPORTED(ANY_SOCKET, ANY_CHANNEL, 2),
  NUMBER_OF_CHANNELS_SUPPORTED(ANY_SOCKET, 1),
  MOTHER_BOARD_LAYERS(LAYERS_4),

  MEMCLK_DIS_MAP(ANY_SOCKET, ANY_CHANNEL, 0x01, 0x02, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00),
  CKE_TRI_MAP(ANY_SOCKET, ANY_CHANNEL, 0x01, 0x02, 0x04, 0x08), /* TODO: bit2map, bit3map */
  ODT_TRI_MAP(ANY_SOCKET, ANY_CHANNEL, 0x01, 0x02, 0x04, 0x08),
  CS_TRI_MAP(ANY_SOCKET, ANY_CHANNEL, 0x01, 0x02, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00),

  PSO_END
};

void board_BeforeInitPost(struct sysinfo *cb, AMD_POST_PARAMS *InitPost)
{
	InitPost->MemConfig.PlatformMemoryConfiguration = (PSO_ENTRY *)PlatformMemoryTable;
}

void board_BeforeInitMid(struct sysinfo *cb, AMD_MID_PARAMS *InitMid)
{
	/* 0 iGpuVgaAdapter, 1 iGpuVgaNonAdapter; */
	InitMid->GnbMidConfiguration.iGpuVgaMode = 0;
}
