---
title: Overview
---

# {{page.title}}

   <div class="image-figure">
    <img style="width:100%;" src="{{'guide/assets/images/ui_overview.jpg' | relative_url}}" alt="Main UI">
  <p>
      <span class="figure-title">Figure</span> The main clTEM user interface.
  </p>
  </div> 

When you open clTEM, you will be greeted by the main window shown below. The user interface is split into several 'panels' that contain various settings or show information. Some of these have multiple tabs for the different settings. From these panels, multiple dialogs may be opened that may also accessible from the menu bar. This page will give a brief overview of the interface elements. Each panel has it's own page with an in-depth explanation.

## Menus

   <div class="image-figure">
    <img style="width:70%;" src="{{'guide/assets/images/menus.png' | relative_url}}" alt="Main UI">
  <p>
      <span class="figure-title">Figure</span> The the menus in clTEM
  </p>
  </div> 

- **File menu** is used to open the structure and import/export configuration files

- **Simulation menu** is used to set various parameters for the simulation. Many of these functions can be accessed through the main interface. It is also possible to run the simulation from this menu.

- **Settings menu** contains computer specific settings that will rarely be modified, and may vary between computers/users.

## Panels

- **Output** is where the results of the simulations are displayed and stored. This also also where the data can be exported for use in other applications.

- **Overview** gives a quick overview of the size of the simulation, in particular the real and reciprocal scale. Here you can toggle the use of the full 3D approximation, set the simulation resolution and open the dialog to set the simulation area.

- **Settings** contains the majority of the parameters used for most simulation. This includes setting the microscope parameters, aberrations and setting inelastic/incoherent effects. Many of the panels have further settings available from a dialog when pressing <code>More</code>

- **Mode** The primary functions of this panel is to select the simulation mode. The active tab will be the type of simulation performed. Further to this are some settings the apply only to that simulation type (e.g. STEM detectors).

- **Run** This panel is where simulations are started and canceled. The iterations used for incoherent effects is also set there. If no incoherent effects are enabled, <code>(N/A)</code> is displayed in the text box.

## UI features

There are several helpful features in the interface to take note of. These are designed to guide you in using the application and avoid confusion.

- Text entry boxes will display units where applicable.
- Text entry boxes will highlight invalid settings by highlighting the text orange.
- Text entry boxes will grey out when the parameter isn't relevant to the current simulation. You may still edit the value but it will have no effect.
- Before you run a simulation a check will be performed to validate all the settings. An error dialog will be displayed explaining the problem. 