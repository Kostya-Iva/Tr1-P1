# Tr1-P1
Cartesian parallel manipulator, featuring "Tripteron" with 3 DoF, made as a student project for particular universitu course.
* Image of a robot:

  <p align="center">
    <img src="Tr1-P1.jpg" alt="Trippi" width="400">
  </p>
# Motivation
The Tripteron project was born out of a practical need within the **EPIC** student research group: automating the preparation of metallographic specimens without breaking the bank.

Since professional, dedicated lab equipment is financially out of reach for most student budgets, this project delivers a cost-effective, simple to program, versatile alternative.

# Mechanical design

# Electronical design

# Programming
The system interfaces with three stepper motor drivers (X, Y, Z axes), dedicated limit switches (endstops) for homing, and control hardware.
The code architecture takes form of a state machine and it is described with this diagram:
  <p align="center">
    <img src="State_machine_diagram.PNG" alt="Stany" width="700">
  </p>
  
Such architecture is described with one file, named: *stany.ino* written in the Arduino IDE for ESP32 DevBoard 

### Pin Mapping
| Component / Function | Arduino Pin | Input / Output | Configuration / Description |
| :--- | :---: | :---: | :--- |
| **X-Axis STEP** | `2` | **OUTPUT** | Generates movement pulses for the X axis |
| **X-Axis DIR** | `5` | **OUTPUT** | Controls the direction of rotation for the X axis |
| **Y-Axis STEP** | `3` | **OUTPUT** | Generates movement pulses for the Y axis |
| **Y-Axis DIR** | `6` | **OUTPUT** | Controls the direction of rotation for the Y axis |
| **Z-Axis STEP** | `4` | **OUTPUT** | Generates movement pulses for the Z axis |
| **Z-Axis DIR** | `7` | **OUTPUT** | Controls the direction of rotation for the Z axis |
| **X-Axis Endstop** | `9` | **INPUT_PULLUP** | Limit switch for X home position |
| **Y-Axis Endstop** | `10` | **INPUT_PULLUP** | Limit switch for Y home position |
| **Z-Axis Endstop** | `11` | **INPUT_PULLUP** | Limit switch for Z home position |
| **START Button** | `12` | **INPUT_PULLUP** | Initiates the calibration and automation cycle |
| **STOP Button** | `13` | **INPUT_PULLUP** | Hardwired safety trigger for Emergency Stop |
---

#### State Descriptions:

* **`STATE_IDLE`** Remains stationary with all motors locked or holding position, actively polling the `START` button pin.
  
* **`STATE_HOMING`** Triggered immediately after pressing START. The system drives the X, Y, and Z axes sequentially towards their hardware limit switches to accurately establish the machine's absolute reference point.
  
* **`STATE_READY`** The homing sequence has concluded successfully. The parallel effector is calibrated at its home base, actively polling the `START` button pin.
  
* **`STATE_PROCESSING`** The system dynamically generates step pulses for all three axes simultaneously, executing predefined movement trajectory
  
* **`STATE_FINISHED`** The polishing cycle is complete. Retracts the effector away from the workpiece area, and transitions back to `STATE_IDLE` for the next run.
