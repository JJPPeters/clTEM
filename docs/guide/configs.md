---
title: Configuration files
---

# {{page.title}}

clTEM has support for configuration files that are used to save simulation parameters alongside exported simulations as well as provide a way to save parameters, either as a default setting or to load later. This is useful for setting parameters for different microscopes to quickly switch between.

The format of these files is `.json`, that provide a human readable and editable text file. To see the parameters that are stored, you can export a configuration file from the File menu and open it in any text editor. You can then make any changes and load this file back in using the import option in the file menu.

The option to export the default configuration takes the current settings from the interface and saves them to a default file. This is then loaded when clTEM is run, or when you select the option to import the default configuration.