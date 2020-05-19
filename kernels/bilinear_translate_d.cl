////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Translates (shifts) an image by a pixel and subpixel amount
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Takes an image and shifts and applies them using bilinear interpolation for sub-pixel movements
/// wikipedia has a nice visualisation of this calculation (basically splitting the pixel into quandrants, created from
/// the sub pixel shift, and multiplying the pixel value by the opposing quadrant
/// Image is wrapped around, but doesnt account for translation by more than one image size
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input - image to be translated
/// output - output translated image
/// pixel_shift_x - integer shift amount in x
/// pixel_shift_y - integer shift amount in y
/// subpixel_shift_x - sub-pixel shift in x
/// subpixel_shift_y - sub-pixel shift in y
/// width - width of the input/output
/// height - height of the output/output
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void bilinear_translate_d( __global double* input,
								 __global double* output,
								 int pixel_shift_x,
								 int pixel_shift_y,
								 double subpixel_shift_x,
								 double subpixel_shift_y,
								 unsigned int width,
								 unsigned int height)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
    int id = xid + width * yid;

    // first account for pixel shifts
    int new_xid = xid - pixel_shift_x;
    int new_yid = yid - pixel_shift_y;

    if (new_xid < 0)
        new_xid = width + new_xid;
    else if (new_xid >= width)
        new_xid = new_xid - width;

    if (new_yid < 0)
        new_yid = height + new_yid;
    else if (new_yid >= height)
        new_yid = new_yid - height;

    int new_id = new_xid + width * new_yid;

    // index for pixel to top, left and top left
    int br_px = new_id;
    int bl_px = new_id - 1;
    int tr_px = new_id - width;
    int tl_px = new_id - width - 1;

    if (new_xid == 0) {
        bl_px += width;
        tl_px += width;
    }
    if (new_yid == 0) {
        tr_px += width*height;
        tl_px += width*height;
    }


    if (new_xid >= 0 && new_xid < width && new_yid >= 0 && new_yid < height) {

        // the are the factors for 'how much to take' from the nearest neighbours
        // they are applied to the 'opposite' corners
        double f_br = subpixel_shift_x * subpixel_shift_y;
        double f_bl = (1.0 - subpixel_shift_x) * subpixel_shift_y;
        double f_tr = subpixel_shift_x * (1.0 - subpixel_shift_y);
        double f_tl = (1.0 - subpixel_shift_x) * (1.0 - subpixel_shift_y);

        // I think this works with complex numbers?
        output[id] = f_tl * input[br_px] + f_tr * input[bl_px] + f_bl * input[tr_px] + f_br * input[tl_px];
    } else {
        output[id] = 0.0;
    }
}