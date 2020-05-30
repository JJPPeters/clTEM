---
title: OpenCL
---

# {{page.title}}

OpenCL is an external library that is required to even open clTEM. How you get OpenCL is platform and vendor specific. Most people may already have OpenCL as part of their graphics drivers, though if you do not have it, then check out your hardware vendor's website (i.e. Intel, NVidia, AMD).

## Selecting device(s)

In clTEM, the device(s) you want to use need to be selected the first time you run the program (the device(s) will be save for next time). To do this, go to the <code>Settings &rarr; OpenCL</code> menu to open the OpenCL dialog (shown below). On the left is a list of the currently active devices, and the right allows you to add/remove devices. Each device has a platform and a device name, so selecting different platforms will show different devices. Click <code>Add</code> to add the device. To remove a device, select it in the table on the left, and click <code>Delete</code>.

If multiple devices are selected, they will be used in the order that they are listed in the table.

   <div class="image-figure">
    <img style="width:80%;" src="{{'guide/assets/images/opencl-dialog.png' | relative_url}}" alt="OpenCL dialog">
  <p>
      <span class="figure-title">Figure</span> Dialog used to configure OpenCL.
  </p>
  </div> 