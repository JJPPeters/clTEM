import os
import json
import subprocess
import shutil
from distutils.dir_util import copy_tree

#
# Set our directories first
#

# executable of clTEM_cmd.exe, wherever you have that installed
# sim_exe = r"D:\Users\Jon\Git\clTEM-dev\clTEM-dist\clTEM_0.2.4a\dist_files\clTEM_cmd.exe")
sim_exe = r"D:\OneDrive\Documents\Programming\clTEM\clTEM-dist\clTEM_0.3.3a\dist_files\clTEM_cmd.exe"

# working folder (i.e. where the input and output data will be)
# os.path.dirname(os.path.abspath(__file__)) is the directory of the script
work_folder = os.path.dirname(os.path.abspath(__file__))

# input structure file (.cif or .xyz)
inp_structure = os.path.join(work_folder, "sto.cif")
# inp_structure = os.path.join(work_folder, "BTO_cubic.xyz")

# base configuration file
conf_file = os.path.join(work_folder, "config.json")

# folder for the final output
out_folder = os.path.join(work_folder, "out")

# folder for the direct output of the simulation (before copying to the final output folder)
temp_folder = os.path.join(work_folder, "out_temp")

# the (possibly altered) configuration file
temp_conf_file = os.path.join(temp_folder, "config.json")

# create these directories

try:
    os.makedirs(temp_folder)
except FileExistsError:
    pass

try:
    os.makedirs(out_folder)
except FileExistsError:
    pass

#
# Set some cif options (not needed for xyz)
#

# cell x, y, z dimensions in Angstrom
cif_cell_size = "{},{},{}".format(100, 100, 100)

# zone axis
cif_zone_axis = "{},{},{}".format(0, 0, 1)

# x axis
cif_normal_axis = "{},{},{}".format(0, 1, 0)

# tilt perturbations (about x, y, z axis)
cif_tilt = "{},{},{}".format(0, 0, 0)

#
# Set our OpenCL device
#

device_string = "gpus"

#
# Start of the actual scripting bit
#

# load the config file
with open(conf_file, "r") as f:
    default_config = json.load(f)

# loop over our defocus (or any variable)
for df in range (-100, 100, 10):

    # generate our output sub-folder
    out_subfolder = os.path.join(out_folder, f"df={df}")
    # and create it
    try:
        os.makedirs(out_subfolder)
    except FileExistsError:
        pass

    # simple test to see if we have simulated data already (and then skip this iteration)
    # useful for if there is a crash and we don't want to simulate everything from the start
    if os.listdir(out_subfolder):
        continue

    # modify our configuration
    config = default_config.copy()

    config["microscope"]["aberrations"]["C10"]["val"] = df

    # save it to it's new location (we don't want to overwrite the original)
    with open(temp_conf_file, "w") as f:
            json.dump(config, f)

    # here we actually run the simulation (inputs depend on the structure file type)
    if os.path.splitext(inp_structure)[1] == ".cif":
        print("cif")
        subprocess.call([sim_exe, inp_structure, "-s", cif_cell_size,
                                                 "-z", cif_zone_axis,
                                                 "-n", cif_normal_axis,
                                                 "-t", cif_tilt,
                                                 "-o", temp_folder,
                                                 "-d", device_string,
                                                 "-c", temp_conf_file])
    else:
        subprocess.call([sim_exe, inp_structure, "-o", out_folder,
                                                 "-d", device_string,
                                                 "-c", temp_conf_file])

    # copy the outputs to their new folder
    copy_tree(temp_folder, out_subfolder)
