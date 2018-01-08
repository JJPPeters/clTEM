//
// Created by jon on 09/10/16.
//

#include "clfourier.h"

#include <math.h>

clFourier::~clFourier(void)
{
    clMedBuffer.reset();

    fftStatus = clfftDestroyPlan(&fftplan);
    clFftError::Throw(fftStatus, "clFourier");
}

void clFourier::Setup(int width, int height)
{
    // Perform setup for FFT's

    fftStatus = clfftInitSetupData(&fftSetupData);

//    fftSetupData.debugFlags = 0x1; # this dumps the kernels that are produced to the current working directory

    clFftError::Throw(fftStatus);
    fftStatus = clfftSetup(&fftSetupData);
    clFftError::Throw(fftStatus);

    //	Local Data
    size_t buffSizeBytesIn = 0;
    size_t buffSizeBytesOut = 0;
    size_t fftVectorSize= 0;
    size_t fftVectorSizePadded = 0;
    size_t fftBatchSize = 0;
    cl_uint nBuffersOut = 0;
    cl_uint profileCount = 0;

    clfftDim fftdim = CLFFT_2D;
    clfftResultLocation	place = CLFFT_OUTOFPLACE;
    clfftLayout inLayout  = CLFFT_COMPLEX_INTERLEAVED;
    clfftLayout outLayout = CLFFT_COMPLEX_INTERLEAVED;

    size_t clLengths[ 3 ];
    size_t clPadding[ 3 ] = {0, 0, 0 };  // *** TODO
    size_t clStrides[ 4 ];
    size_t batchSize = 1;


    clLengths[0]=width;
    clLengths[1]=height;
    clLengths[2]=1;

    clStrides[ 0 ] = 1;
    clStrides[ 1 ] = clStrides[ 0 ] * (clLengths[ 0 ] + clPadding[ 0 ]);
    clStrides[ 2 ] = clStrides[ 1 ] * (clLengths[ 1 ] + clPadding[ 1 ]);
    clStrides[ 3 ] = clStrides[ 2 ] * (clLengths[ 2 ] + clPadding[ 2 ]);

    fftVectorSize	= clLengths[ 0 ] * clLengths[ 1 ] * clLengths[ 2 ];
    fftVectorSizePadded = clStrides[ 3 ];
    fftBatchSize	= fftVectorSizePadded * batchSize;


    fftStatus = clfftCreateDefaultPlan( &fftplan, Context->GetContext(), fftdim, clLengths );
    clFftError::Throw(fftStatus, "clFourier");

    //	Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
    fftStatus = clfftSetResultLocation( fftplan, place );
    clFftError::Throw(fftStatus, "clFourier");
    fftStatus = clfftSetPlanPrecision(fftplan,CLFFT_SINGLE);
    clFftError::Throw(fftStatus, "clFourier");
    fftStatus = clfftSetLayout( fftplan, inLayout, outLayout );
    clFftError::Throw(fftStatus, "clFourier");
    fftStatus = clfftSetPlanBatchSize( fftplan, batchSize );
    clFftError::Throw(fftStatus, "clFourier");
    fftStatus = clfftSetPlanScale (fftplan, CLFFT_FORWARD, 1/sqrtf(width * height));
    clFftError::Throw(fftStatus, "clFourier");
    fftStatus = clfftSetPlanScale (fftplan, CLFFT_BACKWARD, 1/sqrtf(width * height));
    clFftError::Throw(fftStatus, "clFourier");

    // Not using padding here yet
    if ((clPadding[ 0 ] | clPadding[ 1 ] | clPadding[ 2 ]) != 0) {
        clfftSetPlanInStride  ( fftplan, fftdim, clStrides );
        clfftSetPlanOutStride ( fftplan, fftdim, clStrides );
        clfftSetPlanDistance  ( fftplan, clStrides[ fftdim ], clStrides[ fftdim ]);
    }

    fftStatus = clfftBakePlan( fftplan, 1, &Context->GetQueue(), NULL, NULL );
    clFftError::Throw(fftStatus, "clFourier");

    //get the buffersize

    fftStatus = clfftGetTmpBufSize(fftplan, &buffersize );
    clFftError::Throw(fftStatus, "clFourier");

    if (buffersize)
    {
        // because buffersize should be in bytes already...
        clMedBuffer = std::move(Context->CreateBuffer<char,Manual>(buffersize));
        //clCreateBuffer ( *context, CL_MEM_READ_WRITE, buffersize, 0, &medstatus);
    }
}

clFourier::clFourier(clContext &Context, int _width, int _height): Context(&Context), width(_width), height(_height), buffersize(0)
{
    Setup(_width,_height);
    AutoTeardownFFT::GetInstance();
};