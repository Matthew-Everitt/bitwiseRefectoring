#pragma once
#define reportRawBits
#define reportNewBytes
#define rawBytes

//#define verboseCUTS
#ifdef verboseCUTS //All options for the CUTS interface, hopefully reasonably self explanatory
#define detailedChecksum
#define rawBytes
#define twoByteNumbers
#define reportFreqChanges
#define reportFunctionCalls
#define reportBlockDetails
#define reportCarrierState
#define verboseFile
#define reportNewBytes
#define reportRawBits
#define reportInputStateChange
#endif



#ifdef verboseTimings //Timing benchmarks
#define verboseDisplayTime
#endif

#ifdef verboseFileDisplay //Debug levels for the SD card exploration menus
#define verbosePathManagement
#endif
