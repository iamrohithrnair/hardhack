export interface MachineProfile {
  id: string;
  name: string;
  category: string;
  motorType: string;
  nominalRPM: number;
  fundamentalHz: number;
  isoClass: "Class I" | "Class II" | "Class III" | "Class IV";
  warningRms: number;
  criticalRms: number;
  image: string;
  bearingType: string;
  mountTorque: string;
}

export const DEFAULT_MACHINES: MachineProfile[] = [
  {
    id: "desk-fan",
    name: "Industrial Desk Fan 01",
    category: "Cooling Fan & Blower",
    motorType: "4-Pole AC Induction Motor",
    nominalRPM: 2910,
    fundamentalHz: 48.5,
    isoClass: "Class I",
    warningRms: 0.25,
    criticalRms: 0.80,
    image: "https://images.unsplash.com/photo-1581092160607-ee22621dd758?w=600&auto=format&fit=crop&q=80",
    bearingType: "Dual Deep-Groove 608RS",
    mountTorque: "12.5 Nm"
  },
  {
    id: "3d-printer",
    name: "Voron 2.4 Extruder & Stepper",
    category: "3D Printer CoreXY",
    motorType: "NEMA 17 Stepper (1.8°)",
    nominalRPM: 4800,
    fundamentalHz: 80.0,
    isoClass: "Class I",
    warningRms: 0.35,
    criticalRms: 1.10,
    image: "https://images.unsplash.com/photo-1631553127988-3482a991f24d?w=600&auto=format&fit=crop&q=80",
    bearingType: "MR85ZZ Precision Linear",
    mountTorque: "3.2 Nm"
  },
  {
    id: "washing-machine",
    name: "Whirlpool Drum Inverter Pump",
    category: "Home Appliance",
    motorType: "Direct-Drive Inverter BLDC",
    nominalRPM: 1200,
    fundamentalHz: 20.0,
    isoClass: "Class II",
    warningRms: 0.40,
    criticalRms: 1.50,
    image: "https://images.unsplash.com/photo-1626806787461-102c1bfaaea1?w=600&auto=format&fit=crop&q=80",
    bearingType: "Heavy-Duty 6205-2RS",
    mountTorque: "45.0 Nm"
  },
  {
    id: "cnc-spindle",
    name: "CNC 2.2kW Water-Cooled Spindle",
    category: "CNC Milling Machine",
    motorType: "High-Speed 3-Phase Brushless",
    nominalRPM: 24000,
    fundamentalHz: 400.0,
    isoClass: "Class I",
    warningRms: 0.20,
    criticalRms: 0.65,
    image: "https://images.unsplash.com/photo-1504917599217-d4dc5ebe6122?w=600&auto=format&fit=crop&q=80",
    bearingType: "Angular Contact 7005C P4",
    mountTorque: "28.0 Nm"
  },
  {
    id: "auto-engine",
    name: "Automotive Alternator & Serpentine",
    category: "Automotive Powertrain",
    motorType: "Claw-Pole Rotor w/ Slip Rings",
    nominalRPM: 3600,
    fundamentalHz: 60.0,
    isoClass: "Class II",
    warningRms: 0.30,
    criticalRms: 0.95,
    image: "https://images.unsplash.com/photo-1486006920555-c77dce18193b?w=600&auto=format&fit=crop&q=80",
    bearingType: "Sealed Alternator Ball Bearing",
    mountTorque: "35.0 Nm"
  }
];
