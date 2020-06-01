---
title: Advanced settings
---

# {{page.title}}

The advanced settings are split into two parts. One group affects the simulation and the other affects the use of the program.

## Advanced simulation settings

<div class="image-figure">
	<img style="width:40%;" src="{{'guide/assets/images/advanced_sim.png' | relative_url}}" alt="Advanced simulation settings">
  	<p>
		<span class="figure-title">Figure</span> The advanced simulation settings dialog.
  	</p>
</div>

The advanced simulation settings are found in the <code>Simulation &rarr; General</code> menu and show the general simulation settings dialog. The options are as follows:

- **Double precision** will enable double precision calculations. On GPU hardware that supports it, this is significantly slower but more precise. Many GPUs do not support double precision calculation at any reasonable speeds.
- **Potentials** is the form of the scattering factors/potentials used. The current options are `kirkland`, `lobato` and `peng`. It is recommended to use the Kirkland or Lobato form of the parameterisation. For more information see references 1, 5 and 6.
- **Full 3D integrals** is the number of sub-slices to split each slice into when calculating the potentials for a full 3D simulation. 
- **Min padding x/y** is the default padding added to a simulation to avoid wrap around effects. 
- **Min padding z** is the minimum padding added to the top and bottom of each slice. Only useful for full 3D simulations where potentials can influence beyond their own slice. 
- **Parallel pixels** will optimise STEM simulations by calculating the potentials once for each group of parallel pixels. This has the disadvantage that incoherent effects will be the same for pixels simulated in parallel. To mitigate this random pixels are chosen to be parallel, though you should choose as small a value as possible if you want to simulate incoherent effects. This also has the effect of decreasing the maximum reciprocal space vectors as the full crystal potential must be calculated at the same time (and hence a larger real space area)

## Advanced program settings

<div class="image-figure">
	<img style="width:40%;" src="{{'guide/assets/images/advanced_general.png' | relative_url}}" alt="Advanced general settings">
  	<p>
		<span class="figure-title">Figure</span> The advanced genera; settings dialog.
  	</p>
</div>

The advanced program settings are found in the <code>Settings &rarr; General</code> menu and show the general settings dialog. The options are as follows:

- **Theme** allows you to choose from a number of themes to customise the appearance (available on Windows only). 
- **MSAA** sets the sampling for structure previews. The higher the number, the less 'jagged' the preview will look but at the cost of performance.
- **Live STEM** enabled the live update of the STEM images as individual pixels are simulated. This should have minimal performance loss, but can be enabled/disabled as desired.
- **Debug logging** enabled the saving of log files. Use this only if advised. It will negatively impact simulation performance and may create large files.